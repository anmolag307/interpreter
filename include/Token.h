#ifndef CODECRAFTERS_TOKEN_H
#define CODECRAFTERS_TOKEN_H

#include <string>

enum class TokenType {
    // single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    // literals
    IDENTIFIER, STRING, NUMBER,
    // keywords
    VAR, FUN, IF, ELSE, FOR, WHILE, RETURN,
    // special
    END_OF_FILE
};

class Token {
public:
    Token(TokenType type, const std::string& lexeme, int line);
    std::string toString() const;
    TokenType type() const;
    const std::string& lexeme() const;
    int line() const;

private:
    TokenType type_;
    std::string lexeme_;
    int line_;
};

#endif // CODECRAFTERS_TOKEN_H
