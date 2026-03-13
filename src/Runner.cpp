#include "../include/Runner.h"
#include "../include/Evaluator.h"
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

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

bool startsWithKeyword(const std::string& text, const std::string& keyword) {
    if (text.size() < keyword.size()) {
        return false;
    }

    if (text.compare(0, keyword.size(), keyword) != 0) {
        return false;
    }

    if (text.size() == keyword.size()) {
        return true;
    }

    return std::isspace((unsigned char)text[keyword.size()]);
}

bool parseIfStatement(const std::string& statement, std::string& condition, std::string& trailingStatement) {
    std::string s = trim(statement);
    if (!startsWithKeyword(s, "if")) {
        return false;
    }

    int i = 2;
    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
        ++i;
    }

    if (i >= (int)s.size() || s[i] != '(') {
        return false;
    }

    int start = ++i;
    int depth = 1;
    while (i < (int)s.size() && depth > 0) {
        if (s[i] == '(') {
            ++depth;
        } else if (s[i] == ')') {
            --depth;
        }
        ++i;
    }

    if (depth != 0) {
        return false;
    }

    int end = i - 1;
    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
        ++i;
    }

    condition = s.substr(start, end - start);
    trailingStatement = trim(s.substr(i));
    return true;
}

bool parseIfCondition(const std::string& statement, std::string& condition) {
    std::string trailing;
    if (!parseIfStatement(statement, condition, trailing)) {
        return false;
    }
    return trailing.empty();
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
    int blockDepth = 0;
    std::vector<int> blockStartLines;
    struct PendingStatement {
        std::string text;
        int line;
    };
    std::vector<PendingStatement> statements;
    std::string current;

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

        if (!inString && c == '{') {
            std::string beforeBrace = trim(current);
            if (!beforeBrace.empty()) {
                std::string ifCondition;
                if (!parseIfCondition(beforeBrace, ifCondition)) {
                    std::cerr << "[line " << line << "] Error at '{': Expect ';' after statement." << std::endl;
                    return 65;
                }
                statements.push_back({beforeBrace, statementLine});
                current.clear();
            }

            ++blockDepth;
            blockStartLines.push_back(line);
            statements.push_back({"{", line});
            statementLine = line;
            continue;
        }

        if (!inString && c == '}') {
            if (blockDepth == 0) {
                std::cerr << "[line " << line << "] Error at '}': Unexpected '}'" << std::endl;
                return 65;
            }

            if (!trim(current).empty()) {
                std::cerr << "[line " << line << "] Error at '}': Expect ';' after statement." << std::endl;
                return 65; 
            }

            --blockDepth;
            blockStartLines.pop_back();
            statements.push_back({"}", line});
            statementLine = line;
            continue;
        }

        if (!inString && c == ';') {
            std::string statement = trim(current);
            if (!statement.empty()) {
                statements.push_back({statement, statementLine});
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

    if (blockDepth > 0) {
        int startLine = blockStartLines.empty() ? line : blockStartLines.back();
        std::cerr << "[line " << startLine << "] Error at 'end': Expect '}' after block." << std::endl;
        return 65;
    }

    std::string trailing = trim(current);
    if (!trailing.empty()) {
        std::cerr << "[line " << statementLine << "] Error at 'end': Expect ';' after statement." << std::endl;
        return 65;
    }

    Evaluator evaluator;
    struct BlockFrame {
        bool executes;
        bool hasScope;
    };
    std::vector<BlockFrame> blockFrames;
    bool pendingIf = false;
    bool pendingIfResult = false;

    auto isCurrentBranchActive = [&]() {
        for (const auto& frame : blockFrames) {
            if (!frame.executes) {
                return false;
            }
        }
        return true;
    };

    auto runRegularStatement = [&](const std::string& statement, int stmtLine) -> int {
        bool isPrintStatement =
            statement.rfind("print", 0) == 0 &&
            (statement.size() == 5 || std::isspace((unsigned char)statement[5]));

        bool isVarDeclaration =
            statement.rfind("var", 0) == 0 &&
            (statement.size() == 3 || std::isspace((unsigned char)statement[3]));

        if (isVarDeclaration) {
            std::string declaration = trim(statement.substr(3));
            if (declaration.empty()) {
                std::cerr << "[line " << stmtLine << "] Error at ';': Expect variable name." << std::endl;
                return 65;
            }

            int code = evaluator.declareVariableFromString(declaration);
            if (code != 0) {
                return code;
            }
            return 0;
        }

        std::string expression = isPrintStatement
            ? trim(statement.substr(5))
            : statement;

        if (expression.empty()) {
            std::cerr << "[line " << stmtLine << "] Error at ';': Expect expression." << std::endl;
            return 65;
        }

        return evaluator.evaluateFromString(expression, isPrintStatement);
    };

    for (const auto& pending : statements) {
        const std::string& statement = pending.text;
        int stmtLine = pending.line;

        if (statement == "{") {
            bool parentActive = isCurrentBranchActive();
            bool blockExecutes = parentActive;
            if (pendingIf) {
                blockExecutes = parentActive && pendingIfResult;
                pendingIf = false;
            }

            if (blockExecutes) {
                evaluator.beginScope();
            }
            blockFrames.push_back({blockExecutes, blockExecutes});
            continue;
        }

        if (statement == "}") {
            if (blockFrames.empty()) {
                std::cerr << "[line " << stmtLine << "] Error at '}': Unexpected '}'" << std::endl;
                return 65;
            }

            BlockFrame frame = blockFrames.back();
            blockFrames.pop_back();
            if (frame.hasScope) {
                int code = evaluator.endScope();
                if (code != 0) {
                    std::cerr << "[line " << stmtLine << "] Error at '}': Unexpected '}'" << std::endl;
                    return code;
                }
            }
            continue;
        }

        if (pendingIf) {
            bool canRun = isCurrentBranchActive() && pendingIfResult;
            pendingIf = false;
            if (!canRun) {
                continue;
            }
        }

        std::string ifCondition;
        std::string inlineThen;
        if (parseIfStatement(statement, ifCondition, inlineThen)) {
            if (ifCondition.empty()) {
                std::cerr << "[line " << stmtLine << "] Error at ')': Expect expression." << std::endl;
                return 65;
            }

            bool shouldRun = false;
            if (isCurrentBranchActive()) {
                bool conditionValue = false;
                int code = evaluator.evaluateConditionFromString(ifCondition, conditionValue);
                if (code != 0) {
                    return code;
                }
                shouldRun = conditionValue;
            }

            if (inlineThen.empty()) {
                pendingIf = true;
                pendingIfResult = shouldRun;
                continue;
            }

            if (!shouldRun) {
                continue;
            }

            int code = runRegularStatement(inlineThen, stmtLine);
            if (code != 0) {
                return code;
            }
            continue;
        }

        if (!isCurrentBranchActive()) {
            continue;
        }

        int code = runRegularStatement(statement, stmtLine);
        if (code != 0) {
            return code;
        }
    }

    if (pendingIf) {
        std::cerr << "[line " << line << "] Error at 'end': Expect '{' after if condition." << std::endl;
        return 65;
    }

    return 0;
}
