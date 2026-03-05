#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include "../include/Scanner.h"
#include "../include/Parser.h"
#include "../include/Evaluator.h"
#include "../include/Utils.h"

int main(int argc, char *argv[]) {
    // Disable output buffering
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cerr << "Logs from your program will appear here!" << std::endl;

    if (argc < 3) {
        std::cerr << "Usage: ./your_program tokenize|parse|evaluate <filename>" << std::endl;
        return 1;
    }

    const std::string command = argv[1];
    int ret_val = 0;

    const std::string file_contents = read_file_contents(argv[2]);

    if(command == "tokenize"){
        Scanner scanner(file_contents);
        scanner.scanTokens();
        ret_val = scanner.getErrorCode();
    }
    else if(command == "parse"){
        Parser parser{std::vector<Token>()};
        ret_val = parser.parseFromString(file_contents);
    }
    else if(command == "evaluate"){
        Evaluator evaluator;
        ret_val = evaluator.evaluateFromString(file_contents);
    }
    else{
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }

    return ret_val;
}
