#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "RuleParser.h"
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <string>
#include <utility> // Required for std::pair

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

DeclarationMatcher IsrCallMatcher = 
  functionDecl(
      hasName("hardware_interrupt_handler"), 
      forEachDescendant(
          callExpr(callee(functionDecl().bind("called_func"))).bind("illegal_call")
      )
  );

class IsrCheckerCallback : public MatchFinder::MatchCallback {
private:
    std::vector<std::string> ForbiddenList;

    // CHANGE 1: CallStack now holds a pair of <Name, Location>
    bool checkCallGraphDFS(const FunctionDecl *Func, ASTContext *Context, 
                           std::unordered_set<std::string>& Visited, 
                           std::vector<std::pair<std::string, SourceLocation>>& CallStack) {
        
        std::string FuncName = Func->getNameAsString();
        
        if (Visited.count(FuncName) > 0) {
            return false;
        }
        Visited.insert(FuncName);

        // --- STEP A: PUSH TO STACK ---
        // CHANGE 2: Push both the name and the exact SourceLocation of the function
        CallStack.push_back({FuncName, Func->getLocation()});

        // --- STEP B: CHECK FOR VIOLATION ---
        if (std::find(ForbiddenList.begin(), ForbiddenList.end(), FuncName) != ForbiddenList.end()) {
            return true;
        }

        if (Func->hasBody()) {
            auto NestedCalls = match(findAll(callExpr(callee(functionDecl().bind("nested")))), *Func->getBody(), *Context);
            for (const auto &CallNode : NestedCalls) {
                const FunctionDecl *NestedFunc = CallNode.getNodeAs<FunctionDecl>("nested");
                if (NestedFunc != nullptr) {
                    
                    bool hasViolationDeepDown = checkCallGraphDFS(NestedFunc, Context, Visited, CallStack);
                    
                    if (hasViolationDeepDown) {
                        return true; 
                    }
                }
            }
        }

        // --- STEP C: POP FROM STACK ---
        CallStack.pop_back();
        return false;
    }

public:
    IsrCheckerCallback(const std::vector<std::string>& Forbidden) 
        : ForbiddenList(Forbidden) {}

    virtual void run(const MatchFinder::MatchResult &Result) {
        ASTContext *Context = Result.Context;
        const CallExpr *Call = Result.Nodes.getNodeAs<CallExpr>("illegal_call");
        const FunctionDecl *Func = Result.Nodes.getNodeAs<FunctionDecl>("called_func");

        if (Call && Func) {
            std::unordered_set<std::string> Visited;
            
            // CHANGE 3: Update the vector declaration in run()
            std::vector<std::pair<std::string, SourceLocation>> CallStack; 
            
            if (checkCallGraphDFS(Func, Context, Visited, CallStack)) {
                DiagnosticsEngine &Diag = Context->getDiagnostics();
                
                // 1. Print the Red Error and the Green Hint
                unsigned ID = Diag.getCustomDiagID(DiagnosticsEngine::Error, 
                              "Violation: This ISR eventually calls a prohibited function.");
                FixItHint Hint = FixItHint::CreateInsertion(Call->getBeginLoc(), "/* FIXME: PROHIBITED RTOS CALL */ ");
                Diag.Report(Call->getBeginLoc(), ID) << Hint;

                // 2. Loop through our preserved CallStack and print the trace as Gray Notes!
                unsigned NoteID = Diag.getCustomDiagID(DiagnosticsEngine::Note, "Call graph trace -> '%0'");
                
                // CHANGE 4: Loop through the pairs and use step.second for the line location!
                for (const auto& step : CallStack) {
                    Diag.Report(step.second, NoteID) << step.first;
                }
            }
        }
    }
};

static cl::OptionCategory MyToolCategory("rtos-analyzer options");

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        errs() << ExpectedParser.takeError();
        return 1;
    }
    
    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    
    RuleParser Parser("rtos_rules.json");
    
    IsrCheckerCallback Callback(Parser.getForbiddenFunctions());
    
    MatchFinder Finder;
    Finder.addMatcher(IsrCallMatcher, &Callback);

    return Tool.run(newFrontendActionFactory(&Finder).get());
}