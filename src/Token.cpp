#include "Token.h"

Token::Token(TokenType type, const std::string& lexeme, int line)
    : type_(type), lexeme_(lexeme), line_(line) {}

TokenType Token::type() const { return type_; }
const std::string& Token::lexeme() const { return lexeme_; }
int Token::line() const { return line_; }

std::string Token::toString() const {
    return lexeme_; // simple placeholder
}
