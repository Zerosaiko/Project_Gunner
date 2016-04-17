#include "scriptcomponent.h"
#include <iostream>

const std::string Script::name{"script"};

template<>
Script buildFromString<Script>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    Script s;
    s.updateRate = buildFromString<float>(str, pos);
    std::string::size_type tokenStart = std::string::npos;
    std::locale loc;
    uint32_t nesting = 0;
    std::string::size_type nestStart = std::string::npos;
    for(std::string::size_type start = 0; start != std::string::npos &&
            start < str[pos].size(); ++start) {
        auto& ch = str[pos][start];
        if (ch == '<') {
            if (nesting == 0)
                nestStart = start + 1;
            ++nesting;
        } else if (ch == '>' && nesting != 0) {
            --nesting;
            if (nesting == 0) {
                s.tokenizedScript.emplace_back(str[pos].substr(nestStart, start - nestStart));
                nestStart = std::string::npos;
                continue;
            }
        }

        if (nesting == 0) {
            if (!std::isspace(ch, loc) && ch != ',' && ch != '\n' && tokenStart == std::string::npos) {
                tokenStart = start;
            } else if ( (std::isspace(ch, loc) || ch == ',' || ch== '\n') && tokenStart != std::string::npos) {
                s.tokenizedScript.emplace_back(str[pos].substr(tokenStart, start - tokenStart));
                tokenStart = std::string::npos;
            }
        }

    }
    if (tokenStart != std::string::npos)
        s.tokenizedScript.emplace_back(str[pos].substr(tokenStart));
    ++pos;


    return s;
}
