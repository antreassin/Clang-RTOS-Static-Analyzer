#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class RuleParser {
private:
    std::vector<std::string> mallocFunctions;

public:
    // Construct, taking a path to json file and parses it
    RuleParser(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open " << filepath << std::endl;
            return;
        }

        json configData;
        file >> configData;

        // iter json arr and push the vector
        for (const auto& funcName : configData["malloc_functions"]) {
            mallocFunctions.push_back(funcName);
        }
    }
    
    // Getter func for llvm
    const std::vector<std::string>& getMallocFunctions() const {
        return mallocFunctions;
    }
};