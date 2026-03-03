#include "Utils.h"

#include <fstream>
#include <sstream>
#include <iostream>

std::string read_file_contents(const std::string& filename) {
    std::ifstream file(filename);
    if(!file.is_open()){
        std::cerr << "Error reading file: " << filename << std::endl;
        std::exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

std::string formatnum(std::string s){
    if(s.find('.')!=std::string::npos){
        // Remove trailing zeros
        while(!s.empty() && s.back()=='0'){
            s.pop_back();
        }
        // If the last character is a dot, add a zero
        if(!s.empty() && s.back()=='.'){
            s+='0';
        }
    }
    else{
        s+=".0";
    }
    return s;
}
