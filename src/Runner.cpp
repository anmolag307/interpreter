#include "../include/Runner.h"
#include "../include/Evaluator.h"
#include <cctype>
#include <iostream>
#include <string>

namespace {
std::string trim(const std::string& s) {
    int start = 0;
    while (start < (int)s.size() && std::isspace((unsigned char)s[start])) {
        ++start;
    }

    int end = (int)s.size() - 1;
    while (end >= start && std::isspace((unsigned char)s[end])) {
        --end;
    }

    if (start > end) {
        return "";
    }

    return s.substr(start, end - start + 1);
}
}

int Runner::runFromString(const std::string& source) {
    int statementLine = 1;
    int line = 1;
    int statementStart = 0;
    bool inString = false;

    for (int i = 0; i < (int)source.size(); ++i) {
        char c = source[i];
        if (c == '\n') {
            ++line;
        }

        if (c == '"') {
            inString = !inString;
        }

        if (inString) {
            continue;
        }

        if (c != ';') {
            continue;
        }

        std::string statement = trim(source.substr(statementStart, i - statementStart));

        if (!statement.empty()) {
            if (statement.rfind("print", 0) != 0 ||
                (statement.size() > 5 && !std::isspace((unsigned char)statement[5]))) {
                std::cerr << "[line " << statementLine << "] Error: Expect 'print' statement." << std::endl;
                return 65;
            }

            std::string expression = trim(statement.substr(5));
            Evaluator evaluator;
            int code = evaluator.evaluateFromString(expression);
            if (code != 0) {
                return code;
            }
        }

        statementStart = i + 1;
        statementLine = line;
    }

    std::string trailing = trim(source.substr(statementStart));
    if (!trailing.empty()) {
        std::cerr << "[line " << statementLine << "] Error at 'end': Expect ';' after statement." << std::endl;
        return 65;
    }

    return 0;
}
