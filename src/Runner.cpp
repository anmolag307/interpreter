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
    std::string src = source;
    if (src.size() >= 3 &&
        (unsigned char)src[0] == 0xEF &&
        (unsigned char)src[1] == 0xBB &&
        (unsigned char)src[2] == 0xBF) {
        src.erase(0, 3);
    }

    int line = 1;
    int statementLine = 1;
    bool inString = false;
    int stringStartLine = 1;
    bool inComment = false;
    std::string current;

    auto processStatement = [&](const std::string& raw, int stmtLine) -> int {
        std::string statement = trim(raw);
        if (statement.empty()) {
            return 0;
        }

        bool isPrintStatement =
            statement.rfind("print", 0) == 0 &&
            (statement.size() == 5 || std::isspace((unsigned char)statement[5]));

        std::string expression = isPrintStatement
            ? trim(statement.substr(5))
            : statement;

        if (expression.empty()) {
            std::cerr << "[line " << stmtLine << "] Error at ';': Expect expression." << std::endl;
            return 65;
        }

        Evaluator evaluator;
        return evaluator.evaluateFromString(expression, isPrintStatement);
    };

    for (int i = 0; i < (int)src.size(); ++i) {
        char c = src[i];

        if (inComment) {
            if (c == '\n') {
                inComment = false;
                ++line;
            }
            continue;
        }

        if (!inString && c == '/' && i + 1 < (int)src.size() && src[i + 1] == '/') {
            inComment = true;
            ++i;
            continue;
        }

        if (c == '"') {
            if (!inString) {
                stringStartLine = line;
            }
            inString = !inString;
        }

        if (!inString && c == ';') {
            int code = processStatement(current, statementLine);
            if (code != 0) {
                return code;
            }
            current.clear();
            statementLine = line;
            continue;
        }

        current += c;

        if (c == '\n') {
            ++line;
            if (trim(current).empty()) {
                statementLine = line;
            }
        }
    }

    if (inString) {
        std::cerr << "[line " << stringStartLine << "] Error: Unterminated string." << std::endl;
        return 65;
    }

    std::string trailing = trim(current);
    if (!trailing.empty()) {
        std::cerr << "[line " << statementLine << "] Error at 'end': Expect ';' after statement." << std::endl;
        return 65;
    }

    return 0;
}
