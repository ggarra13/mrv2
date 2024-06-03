#pragma once

#include <cctype>  // For std::isspace
#include <vector>
#include <string>
#include <sstream>

namespace
{
    std::vector<std::string> splitQuotedString(const std::string& str)
    {
        std::vector<std::string> tokens;
        size_t start = 0;  // Track start of current argument
        
        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];
            
            if (std::isspace(c))
            {
                // Potential end of argument (excluding leading/trailing spaces)
                if (i > start) {
                    const size_t len = i - start;
                    tokens.push_back(str.substr(start, len));
                }
                start = i + 1;  // Update start for next argument
            }
            else if ((c == '"' || c == '\'') &&
                     (i == start || !std::isspace(str[i - 1])))
            {
                // Start of quoted argument (excluding quotes within spaces)
                start = i + 1;
                i = str.find(c, start);  // Find matching closing quote
                if (i != std::string::npos)
                {
                    const size_t len = i - start;
                    const std::string& token = str.substr(start, len);
                    // Extract argument
                    tokens.push_back(token);
                    start += token.size() + 1;
                }
            }
        }
        
        // Handle trailing argument (if any)
        if (start < str.size())
        {
            tokens.push_back(str.substr(start));
        }
        
        return tokens;
    }
    
    //     std::vector<std::string> tokens;
    //     std::istringstream iss(str);

    //     std::string token;
    //     char quote = '\0'; // Track current quote type (single or double)
    //     bool escaped = false; // Flag to track escaped characters

    //     // Skip leading spaces
    //     iss >> std::ws;

    //     // Iterate through characters
    //     while (std::getline(iss, token, ' ')) {
    //         if (token.empty() && !escaped) continue;  // Skip empty tokens except when escaped

    //         for (char& c : token) {
    //             if (escaped) {
    //                 // Handle escaped characters (including quotes)
    //                 escaped = false;
    //             } else if (c == '\\') {
    //                 escaped = true;  // Mark next character as escaped
    //             } else if ((c == '"' && quote != '\'') || (c == '\'' && quote != '"')) {
    //                 // Handle quotes within arguments
    //                 if (quote == '\0') {
    //                     quote = c;
    //                 } else {
    //                     quote = '\0';
    //                     break; // Exit inner loop after closing quote
    //                 }
    //                 token.erase(token.find(c)); // Remove the quote
    //             }
    //         }

    //         // Add token to the vector (even if empty when escaped)
    //         tokens.push_back(token);

    //         // Skip consecutive delimiters
    //         iss >> std::ws;
    //     }
    //     return tokens;
    // }

}

namespace mrv
{

class PythonArgs {
public:
    PythonArgs(const std::vector<std::string>& args) : arguments_(args) {};
    PythonArgs(const std::string quoted_args) :
        PythonArgs(splitQuotedString(quoted_args))
    {
    }

    const std::vector<std::string>& getArguments() const { return arguments_; }
    
private:
    std::vector<std::string> arguments_;
};

}
