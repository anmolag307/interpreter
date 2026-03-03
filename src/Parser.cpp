#include "Parser.h"
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
        std::cerr << "Error: Empty file." << std::endl; return;
    }
    int line = 1;
    for(int i=0; i<(int)source.size(); i++){
        char cur = source[i];
        if(i+3<(int)source.size() && source.substr(i,4)=="true"){
            std::cout << "true" <<std::endl;
            i+=3;
        }
        else if(i+4<(int)source.size() && source.substr(i,5)=="false"){
            std::cout << "false" <<std::endl;
            i+=4;
        }
        else if(i+2<(int)source.size() && source.substr(i,3)=="nil"){
            std::cout << "nil" <<std::endl;
            i+=2;
        }
        else if(isdigit(cur)) {
            std::string number(1, cur);
            while(i+1<(int)source.size() && (isdigit(source[i+1]) || source[i+1]=='.')) {
                number+=source[i+1];
                i++;
            }
            std::cout<<formatnum(number) <<std::endl;
        }
        else if(cur=='"'){
            std::string str;
            i++;
            while(i<(int)source.size() && source[i]!='"') {
                if(source[i]=='\n'){
                    std::cerr << "[line " << line << "] Error: Unterminated string." << std::endl; break;
                }
                str += source[i];
                i++;
            }
            if(i>=(int)source.size()){
                std::cerr << "[line " << line << "] Error: Unterminated string." << std::endl;
            }
            else{
                std::cout<< str <<std::endl;
            }
        }
    }
}
