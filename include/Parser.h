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
    int parseFromString(const std::string& source);

private:
    // helper to parse a single expression from a string, advancing index
    std::string parseExpression(const std::string& source, int &i, int &line);
    // precedence levels for operators
    std::string parseEquality(const std::string& source, int &i, int &line);
    std::string parseComparison(const std::string& source, int &i, int &line);
    std::string parseAdditive(const std::string& source, int &i, int &line);
    std::string parseMultiplicative(const std::string& source, int &i, int &line);
    std::string parseUnary(const std::string& source, int &i, int &line);
    std::string parsePrimary(const std::string& source, int &i, int &line);

    const std::vector<Token>& tokens_;
    size_t current_ = 0;
};

#endif // CODECRAFTERS_PARSER_H
