#include "Scanner.h"
#include "Utils.h"
#include <iostream>
#include <cctype>

Scanner::Scanner(const std::string& source) : src(source) {}

bool Scanner::isAtEnd() const {
    return current_ >= src.size();
}

std::vector<Token> Scanner::scanTokens() {
    std::vector<Token> tokens;
    int line = 1;
    for (int i = 0; i < src.size(); ++i) {
        char cur = src[i];
        if (cur=='\t' || cur==' ') {
            continue;
        }
        else if(cur=='\n') {
            line++;
            continue;
        }
        else if(isalpha(cur) || cur=='_') {
            std::string identifier(1, cur);
            while(i+1<(int)src.size() && (isalnum(src[i+1]) || src[i+1]=='_')) {
                identifier += src[i+1];
                i++;
            }
            if(identifier=="var") {
                std::cout << "VAR var null" <<std::endl;
            }
            else if(identifier=="print") {
                std::cout << "PRINT print null" <<std::endl;
            }
            else if(identifier=="if") {
                std::cout << "IF if null" <<std::endl;
            }
            else if(identifier=="else") {
                std::cout << "ELSE else null" <<std::endl;
            }
            else if(identifier=="while") {
                std::cout << "WHILE while null" <<std::endl;
            }
            else if(identifier=="for") {
                std::cout << "FOR for null" <<std::endl;
            }
            else if(identifier=="true") {
                std::cout << "TRUE true null" <<std::endl;
            }
            else if(identifier=="false") {
                std::cout << "FALSE false null" <<std::endl;
            }
            else if(identifier=="class") {
                std::cout << "CLASS class null" <<std::endl;
            }
            else if(identifier=="fun") {
                std::cout << "FUN fun null" <<std::endl;
            }
            else if(identifier=="return") {
                std::cout << "RETURN return null" <<std::endl;
            }
            else if(identifier=="and") {
                std::cout << "AND and null" <<std::endl;
            }
            else if(identifier=="or") {
                std::cout << "OR or null" <<std::endl;
            }
            else if(identifier=="super") {
                std::cout << "SUPER super null" <<std::endl;
            }
            else if(identifier=="this") {
                std::cout << "THIS this null" <<std::endl;
            }
            else if(identifier=="nil") {
                std::cout << "NIL nil null" <<std::endl;
            }
            else {
                std::cout << "IDENTIFIER " << identifier << " null" <<std::endl;
            }
        }
        else if(isdigit(cur)) {
            std::string number(1, cur);
            while(i+1<(int)src.size() && (isdigit(src[i+1]) || src[i+1]=='.')) {
                number+=src[i+1];
                i++;
            }
            std::cout << "NUMBER " << number << " " << formatnum(number) <<std::endl;
        }
        else if(cur=='"'){
            std::string str;
            i++;
            while(i<(int)src.size() && src[i]!='"') {
                if(src[i]=='\n'){
                    std::cerr << "[line " << line << "] Error: Unterminated string." << std::endl;
                    error_code_ = 65;
                    break;
                }
                str += src[i];
                i++;
            }
            if(i>=(int)src.size()){
                std::cerr << "[line " << line << "] Error: Unterminated string." << std::endl;
                error_code_ = 65;
            }
            else{
                std::cout << "STRING \"" << str << "\" " << str <<std::endl;
            }
        }
        else {
            if(cur == '(') {
                std::cout << "LEFT_PAREN ( null" << std::endl;
            } else if(cur == ')') {
                std::cout << "RIGHT_PAREN ) null" <<std::endl;
            } 
            else if(cur=='{') {
                std::cout << "LEFT_BRACE { null" <<std::endl;
            } 
            else if(cur=='}') {
                std::cout << "RIGHT_BRACE } null" <<std::endl;
            } 
            else if(cur==';') {
                std::cout << "SEMICOLON ; null" <<std::endl;
            }
            else if(cur==',') {
                std::cout << "COMMA , null" <<std::endl;
            }
            else if(cur=='+') {
                std::cout << "PLUS + null" <<std::endl;
            }
            else if(cur=='-') {
                std::cout << "MINUS - null" <<std::endl;
            }
            else if(cur=='*') {
                std::cout << "STAR * null" <<std::endl;
            }
            else if(cur=='/') {
                if(i+1<(int)src.size() && src[i+1]=='/'){
                    // Comment, skip to end of line
                    while(i<(int)src.size() && src[i]!='\n'){
                        i++;
                    }
                    line++;
                }
                else{
                    std::cout << "SLASH / null" <<std::endl;
                }
            }
            else if(cur=='.') {
                std::cout << "DOT . null" <<std::endl;
            }
            else if(cur=='='){
                if(i+1<(int)src.size() && src[i+1]=='='){
                    std::cout << "EQUAL_EQUAL == null" <<std::endl;
                    i++;
                }
                else{
                    std::cout << "EQUAL = null" <<std::endl;
                }
            }
            else if(cur=='!'){
                if(i+1<(int)src.size() && src[i+1]=='='){
                    std::cout << "BANG_EQUAL != null" <<std::endl;
                    i++;
                }
                else{
                    std::cout << "BANG ! null" <<std::endl;
                }
            }
            else if(cur=='<'){
                if(i+1<(int)src.size() && src[i+1]=='='){
                    std::cout << "LESS_EQUAL <= null" <<std::endl;
                    i++;
                }
                else{
                    std::cout << "LESS < null" <<std::endl;
                }
            }
            else if(cur=='>'){
                if(i+1<(int)src.size() && src[i+1]=='='){
                    std::cout << "GREATER_EQUAL >= null" <<std::endl;
                    i++;
                }
                else{
                    std::cout << "GREATER > null" <<std::endl;
                }
            }
            else {
                std::cerr << "[line " << line << "] Error: Unexpected character: " << cur << std::endl;
                error_code_ = 65;
            }
        }
    }
    std::cout << "EOF  null" << std::endl;
    return tokens;
}
