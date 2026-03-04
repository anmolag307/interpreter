#include "../include/Parser.h"
#include "Utils.h"
#include <iostream>
#include <cctype>
#include <functional>

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

// A convenience constructor that accepts raw source (for compatibility)
Parser::Parser(const std::string& source) : tokens_(*(new std::vector<Token>())) {
    (void)source; // we don't produce tokens here; this parser example prints parsed values
}

void Parser::parseFromString(const std::string& source) {
    if(source.empty()){
        std::cerr << "Error: Empty file." << std::endl;
        return;
    }
    int i = 0;
    while(i < (int)source.size()){
        // skip whitespace including newlines
        if(isspace(source[i])){
            i++;
            continue;
        }
        std::string result = parseExpression(source, i);
        if(!result.empty()){
            std::cout << result << std::endl;
        }
    }
}

// recursive descent for a single expression; advances index
std::string Parser::parseExpression(const std::string& source, int &i) {
    return parseAdditive(source, i);
}

// handle + and - (lowest precedence)
std::string Parser::parseAdditive(const std::string& source, int &i) {
    auto skipWhitespace = [&](void){ while(i < (int)source.size() && isspace((unsigned char)source[i])) ++i; };
    
    std::string left = parseMultiplicative(source, i);
    
    while(true) {
        skipWhitespace();
        if(i >= (int)source.size()) break;
        char op = source[i];
        if(op != '+' && op != '-') break;
        ++i; // consume operator
        std::string right = parseMultiplicative(source, i);
        std::string opStr(1, op);
        left = "(" + opStr + " " + left + " " + right + ")";
    }
    return left;
}

// handle * and / (higher precedence than +/-)
std::string Parser::parseMultiplicative(const std::string& source, int &i) {
    auto skipWhitespace = [&](void){ while(i < (int)source.size() && isspace((unsigned char)source[i])) ++i; };
    
    std::string left = parseUnary(source, i);
    
    while(true) {
        skipWhitespace();
        if(i >= (int)source.size()) break;
        char op = source[i];
        if(op != '*' && op != '/') break;
        ++i; // consume operator
        std::string right = parseUnary(source, i);
        std::string opStr(1, op);
        left = "(" + opStr + " " + left + " " + right + ")";
    }
    return left;
}

// handle unary operators: !, -, + (higher precedence than binary operators)
std::string Parser::parseUnary(const std::string& source, int &i) {
    auto skipWhitespace = [&](void){ while(i < (int)source.size() && isspace((unsigned char)source[i])) ++i; };
    
    skipWhitespace();
    if(i < (int)source.size() && (source[i] == '!' || source[i] == '-' || source[i] == '+')) {
        char op = source[i++];
        std::string operand = parseUnary(source, i);
        std::string opStr(1, op);
        return "(" + opStr + " " + operand + ")";
    }
    return parsePrimary(source, i);
}

// handle primary expressions: number, string, identifier, or parenthesized expression
std::string Parser::parsePrimary(const std::string& source, int &i) {
    auto skipWhitespace = [&](void){ while(i < (int)source.size() && isspace((unsigned char)source[i])) ++i; };
    
    skipWhitespace();
    if(i >= (int)source.size()) return "";
    char cur = source[i];
    
    if(cur == '(') {
        ++i; // consume '('
        std::string inner = parseExpression(source, i);
        skipWhitespace();
        if(i < (int)source.size() && source[i] == ')') {
            ++i;
        } else {
            std::cerr << "Error: unmatched '('" << std::endl;
        }
        return "(group " + inner + ")";
    }
    if(i+3 <= (int)source.size() && source.substr(i,4) == "true"){
        i += 4; return "true";
    }
    if(i+4 <= (int)source.size() && source.substr(i,5) == "false"){
        i += 5; return "false";
    }
    if(i+2 <= (int)source.size() && source.substr(i,3) == "nil"){
        i += 3; return "nil";
    }
    if(isdigit((unsigned char)cur)){
        std::string number(1, cur); ++i;
        while(i < (int)source.size() && (isdigit((unsigned char)source[i]) || source[i]=='.')){ number += source[i]; ++i; }
        return formatnum(number);
    }
    if(cur == '"'){
        std::string str; ++i;
        while(i < (int)source.size() && source[i] != '"'){
            if(source[i] == '\n'){ std::cerr<<"Error: Unterminated string."<<std::endl; break; }
            str += source[i]; ++i;
        }
        if(i < (int)source.size() && source[i] == '"') ++i;
        return str;
    }
    if(isalpha((unsigned char)cur) || cur=='_'){
        std::string id(1, cur); ++i;
        while(i < (int)source.size() && (isalnum((unsigned char)source[i]) || source[i]=='_')){ id += source[i]; ++i; }
        return id;
    }
    // unknown
    ++i; return std::string();
}
