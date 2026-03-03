#ifndef CODECRAFTERS_SCANNER_H
#define CODECRAFTERS_SCANNER_H

#include <string>
#include <vector>
#include "Token.h"

class Scanner {
public:
    explicit Scanner(const std::string& source);
    std::vector<Token> scanTokens();

private:
    std::string source_;
    size_t start_ = 0;
    size_t current_ = 0;
    int line_ = 1;

    bool isAtEnd() const;
};

#endif // CODECRAFTERS_SCANNER_H
