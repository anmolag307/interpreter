#include "../include/Parser.h"
#include "Utils.h"
#include <iostream>
#include <cctype>

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
    // skip whitespace
    while(i < (int)source.size() && isspace(source[i])) i++;
    if(i >= (int)source.size()) return "";

    char cur = source[i];
    if(cur == '(') {
        i++; // consume '('
        std::string inner = parseExpression(source, i);
        // skip any whitespace before closing
        while(i < (int)source.size() && isspace(source[i])) i++;
        if(i < (int)source.size() && source[i] == ')') {
            i++;
        } else {
            std::cerr << "Error: unmatched '('" << std::endl;
        }
        return "(group " + inner + ")";
    }
    else if(cur=='!'){
        i++;
        std::string operand = parseExpression(source, i);
        return "(! " + operand + ")";
    }
    else if(cur=='-'){
        i++;
        std::string operand = parseExpression(source, i);
        return "(- " + operand + ")";
    }
    else if(i+3 <= (int)source.size() && source.substr(i,4) == "true"){
        i += 4;
        return "true";
    }
    else if(i+4 <= (int)source.size() && source.substr(i,5) == "false"){
        i += 5;
        return "false";
    }
    else if(i+2 <= (int)source.size() && source.substr(i,3) == "nil"){
        i += 3;
        return "nil";
    }
    else if(isdigit(cur)){
        std::string number(1, cur);
        i++;
        while(i < (int)source.size() && (isdigit(source[i]) || source[i]=='.')){
            number += source[i];
            i++;
        }
        return formatnum(number);
    }
    else if(cur == '"'){
        std::string str;
        i++;
        while(i<(int)source.size() && source[i] != '"'){
            if(source[i] == '\n'){
                std::cerr << "Error: Unterminated string." << std::endl;
                break;
            }
            str += source[i];
            i++;
        }
        if(i < (int)source.size() && source[i] == '"') i++;
        return str;
    }
    else if(isalpha(cur) || cur=='_'){
        std::string id(1, cur);
        i++;
        while(i < (int)source.size() && (isalnum(source[i]) || source[i]=='_')){
            id += source[i];
            i++;
        }
        return id;
    }
    // fallback: skip unknown char
    i++;
    return "";
}
