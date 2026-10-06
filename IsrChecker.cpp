#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "RuleParser.h"
#include <algorithm>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;
using namespace std;

DeclarationMatcher IsrCallMatcher = 
    functionDecl(
        hasAttr(attr::ARMInterrupt),
        forEachDescendant(
            callExpr(callee(functionDecl().bind("called_func"))).bind("illegal_call")
        )
    );

class IsrCheckerCallback : public MatchFinder::MatchCallback {
private:
    std::vector<std::string> ForbiddenList;

public:
  IsrCheckerCallback(const std::vector<std::string>& Forbidden) 
      : ForbiddenList(Forbidden) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;
    const CallExpr *Call = Result.Nodes.getNodeAs<CallExpr>("illegal_call");
    const FunctionDecl *Func = Result.Nodes.getNodeAs<FunctionDecl>("called_func");

    if (Call && Func) {
      std::string FuncName = Func->getNameAsString();
      if (std::find(ForbiddenList.begin(), ForbiddenList.end(), FuncName) != ForbiddenList.end()) {
        
        DiagnosticsEngine &Diag = Context->getDiagnostics();
        unsigned ID = Diag.getCustomDiagID(DiagnosticsEngine::Error, 
            "MISRA/RTOS Violation: The function '%0' is strictly prohibited inside an Interrupt Service Routine (ISR).");
        Diag.Report(Call->getBeginLoc(), ID) << FuncName;
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

  // 4. INSTANTIATE YOUR OA-STYLE CODE HERE
  RuleParser Parser("rtos_rules.json");
  IsrCheckerCallback Callback(Parser.getMallocFunctions());
  
  MatchFinder Finder;
  Finder.addMatcher(IsrCallMatcher, &Callback);

  return Tool.run(newFrontendActionFactory(&Finder).get());
}