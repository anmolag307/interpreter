#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

std::string read_file_contents(const std::string& filename);

string formatnum(string s){
    if(s.find('.')!=string::npos){
        // Remove trailing zeros
        while(s.back()=='0'){
            s.pop_back();
        }
        // If the last character is a dot, remove it
        if(s.back()=='.'){
            s+='0';
        }
    }
    else{
        s+=".0";
    }
    return s;
}

int main(int argc, char *argv[]) {
    // Disable output buffering
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here!" << std::endl;

    if (argc < 3) {
        std::cerr << "Usage: ./your_program tokenize <filename>" << std::endl;
        return 1;
    }

    const std::string command = argv[1];
    int ret_val = 0;
    int line=1;
    if(command == "tokenize"){
        std::string file_contents = read_file_contents(argv[2]);
        for (int i = 0; i < file_contents.size(); ++i) {
            char cur = file_contents[i];
            if (cur=='\t' || cur==' ') {
                continue;
            }
            else if(cur=='\n') {
                line++;
                continue;
            }
            else if(isalpha(cur) || cur=='_') {
                std::string identifier(1, cur);
                while(i+1<file_contents.size() && (isalnum(file_contents[i+1]) || file_contents[i+1]=='_')) {
                    identifier += file_contents[i+1];
                    i++;
                }
                if(identifier=="var") {
                    cout << "VAR var null" <<endl;
                }
                else if(identifier=="print") {
                    cout << "PRINT print null" <<endl;
                }
                else if(identifier=="if") {
                    cout << "IF if null" <<endl;
                }
                else if(identifier=="else") {
                    cout << "ELSE else null" <<endl;
                }
                else if(identifier=="while") {
                    cout << "WHILE while null" <<endl;
                }
                else if(identifier=="for") {
                    cout << "FOR for null" <<endl;
                }
                else if(identifier=="true") {
                    cout << "TRUE true null" <<endl;
                }
                else if(identifier=="false") {
                    cout << "FALSE false null" <<endl;
                }
                else if(identifier=="class") {
                    cout << "CLASS class null" <<endl;
                }
                else if(identifier=="fun") {
                    cout << "FUN fun null" <<endl;
                }
                else if(identifier=="return") {
                    cout << "RETURN return null" <<endl;
                }
                else if(identifier=="and") {
                    cout << "AND and null" <<endl;
                }
                else if(identifier=="or") {
                    cout << "OR or null" <<endl;
                }
                else if(identifier=="super") {
                    cout << "SUPER super null" <<endl;
                }
                else if(identifier=="this") {
                    cout << "THIS this null" <<endl;
                }
                else if(identifier=="nil") {
                    cout << "NIL nil null" <<endl;
                }
                else {
                    cout << "IDENTIFIER " << identifier << " null" <<endl;
                }
            }
            else if(isdigit(cur)) {
                std::string number(1, cur);
                while(i+1<file_contents.size() && (isdigit(file_contents[i+1]) || file_contents[i+1]=='.')) {
                    number+=file_contents[i+1];
                    i++;
                }
                cout << "NUMBER " << number << " " << formatnum(number) <<endl;
            }
            else if(cur=='"'){
                std::string str;
                i++;
                while(i<file_contents.size() && file_contents[i]!='"') {
                    if(file_contents[i]=='\n'){
                        cerr << "[line " << line << "] Error: Unterminated string." << std::endl; ret_val = 65; break;
                    }
                    str += file_contents[i];
                    i++;
                }
                if(i>=file_contents.size()){
                    cerr << "[line " << line << "] Error: Unterminated string." << std::endl; ret_val = 65;
                }
                else{
                    cout << "STRING \"" << str << "\" " << str <<endl;
                }
            }
            else {
                if(cur == '(') {
                    std::cout << "LEFT_PAREN ( null" << std::endl;
                } else if(cur == ')') {
                    cout << "RIGHT_PAREN ) null" <<endl;
                } 
                else if(cur=='{') {
                    cout << "LEFT_BRACE { null" <<endl;
                } 
                else if(cur=='}') {
                    cout << "RIGHT_BRACE } null" <<endl;
                } 
                else if(cur==';') {
                    cout << "SEMICOLON ; null" <<endl;
                }
                else if(cur==',') {
                    cout << "COMMA , null" <<endl;
                }
                else if(cur=='+') {
                    cout << "PLUS + null" <<endl;
                }
                else if(cur=='-') {
                    cout << "MINUS - null" <<endl;
                }
                else if(cur=='*') {
                    cout << "STAR * null" <<endl;
                }
                else if(cur=='/') {
                    if(i+1<file_contents.size() && file_contents[i+1]=='/'){
                        // Comment, skip to end of line
                        while(i<file_contents.size() && file_contents[i]!='\n'){
                            i++;
                        }
                        line++;
                    }
                    else{
                        cout << "SLASH / null" <<endl;
                    }
                }
                else if(cur=='.') {
                    cout << "DOT . null" <<endl;
                }
                else if(cur=='='){
                    if(i+1<file_contents.size() && file_contents[i+1]=='='){
                        cout << "EQUAL_EQUAL == null" <<endl;
                        i++;
                    }
                    else{
                        cout << "EQUAL = null" <<endl;
                    }
                }
                else if(cur=='!'){
                    if(i+1<file_contents.size() && file_contents[i+1]=='='){
                        cout << "BANG_EQUAL != null" <<endl;
                        i++;
                    }
                    else{
                        cout << "BANG ! null" <<endl;
                    }
                }
                else if(cur=='<'){
                    if(i+1<file_contents.size() && file_contents[i+1]=='='){
                        cout << "LESS_EQUAL <= null" <<endl;
                        i++;
                    }
                    else{
                        cout << "LESS < null" <<endl;
                    }
                }
                else if(cur=='>'){
                    if(i+1<file_contents.size() && file_contents[i+1]=='='){
                        cout << "GREATER_EQUAL >= null" <<endl;
                        i++;
                    }
                    else{
                        cout << "GREATER > null" <<endl;
                    }
                }
                else {
                    cerr << "[line " << line << "] Error: Unexpected character: " << cur << std::endl; ret_val = 65;
                }
            }
        }
        std::cout << "EOF  null" << std::endl; // Placeholder, replace this line when implementing the scanner 
    }
    else if(command == "parse"){
        std::string file_contents = read_file_contents(argv[2]);
        if(file_contents.empty()){
            cerr << "Error: Empty file." << endl; return 1;
        }
        for(int i=0; i<file_contents.size(); i++){
            char cur = file_contents[i];
            if(i+3<file_contents.size() && file_contents.substr(i,4)=="true"){
                cout << "true" <<endl;
                i+=3;
            }
            else if(i+4<file_contents.size() && file_contents.substr(i,5)=="false"){
                cout << "false" <<endl;
                i+=4;
            }
            else if(i+2<file_contents.size() && file_contents.substr(i,3)=="nil"){
                cout << "nil" <<endl;
                i+=2;
            }
            else if(isdigit(cur)) {
                std::string number(1, cur);
                while(i+1<file_contents.size() && (isdigit(file_contents[i+1]) || file_contents[i+1]=='.')) {
                    number+=file_contents[i+1];
                    i++;
                }
                cout<<formatnum(number) <<endl;
            }
        }
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }

    return ret_val;
}

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
