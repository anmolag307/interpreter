#ifndef CODECRAFTERS_PARSER_H
#define CODECRAFTERS_PARSER_H

#include <vector>
#include <string>
#include "Token.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    // convenience constructor from source
    explicit Parser(const std::string& source);
    void parse();
    void parseFromString(const std::string& source);

private:
    // helper to parse a single expression from a string, advancing index
    std::string parseExpression(const std::string& source, int &i);

    const std::vector<Token>& tokens_;
    size_t current_ = 0;
};

#endif // CODECRAFTERS_PARSER_H
