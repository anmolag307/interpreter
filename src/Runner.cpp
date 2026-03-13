#include "../include/Runner.h"
#include "../include/Evaluator.h"
#include <cctype>
#include <functional>
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

struct ElseStatement {
    bool matches = false;
    bool isElseIf = false;
    bool malformedElseIf = false;
    std::string condition;
    std::string trailingStatement;
};

ElseStatement parseElseStatement(const std::string& statement) {
    ElseStatement result;
    std::string s = trim(statement);
    if (!startsWithKeyword(s, "else")) {
        return result;
    }

    result.matches = true;
    int i = 4;
    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
        ++i;
    }

    if (i >= (int)s.size()) {
        return result;
    }

    std::string rest = trim(s.substr(i));
    if (startsWithKeyword(rest, "if")) {
        result.isElseIf = true;
        if (!parseIfStatement(rest, result.condition, result.trailingStatement)) {
            result.malformedElseIf = true;
        }
        return result;
    }

    result.trailingStatement = rest;
    return result;
}

bool allowsOpeningBraceAfterHeader(const std::string& statement) {
    std::string ifCondition;
    if (parseIfCondition(statement, ifCondition)) {
        return true;
    }

    ElseStatement elseInfo = parseElseStatement(statement);
    return elseInfo.matches && !elseInfo.malformedElseIf && elseInfo.trailingStatement.empty();
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
                if (!allowsOpeningBraceAfterHeader(beforeBrace)) {
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

    int errorCode = 0;
    std::function<int(int, bool)> consumeStatement;

    auto runMaybe = [&](const std::string& text, int stmtLine, bool shouldExecute) -> int {
        if (!shouldExecute) {
            return 0;
        }
        return runRegularStatement(text, stmtLine);
    };

    std::function<int(int, bool)> consumeBlock = [&](int index, bool shouldExecute) -> int {
        if (index >= (int)statements.size() || statements[index].text != "{") {
            return -1;
        }

        if (shouldExecute) {
            evaluator.beginScope();
        }

        int i = index + 1;
        while (i < (int)statements.size()) {
            if (statements[i].text == "}") {
                if (shouldExecute) {
                    int code = evaluator.endScope();
                    if (code != 0) {
                        std::cerr << "[line " << statements[i].line << "] Error at '}': Unexpected '}'" << std::endl;
                        errorCode = code;
                        return -1;
                    }
                }
                return i + 1;
            }

            i = consumeStatement(i, shouldExecute);
            if (i < 0) {
                return -1;
            }
        }

        int errLine = statements[index].line;
        std::cerr << "[line " << errLine << "] Error at 'end': Expect '}' after block." << std::endl;
        errorCode = 65;
        return -1;
    };

    auto consumeBody = [&](int headerIndex,
                           const std::string& inlineBody,
                           bool shouldExecute,
                           int& outNextIndex,
                           const std::string& missingBodyMessage) -> bool {
        if (!inlineBody.empty()) {
            int code = runMaybe(inlineBody, statements[headerIndex].line, shouldExecute);
            if (code != 0) {
                errorCode = code;
                return false;
            }
            outNextIndex = headerIndex + 1;
            return true;
        }

        int bodyIndex = headerIndex + 1;
        if (bodyIndex >= (int)statements.size()) {
            std::cerr << "[line " << statements[headerIndex].line << "] Error at 'end': " << missingBodyMessage << std::endl;
            errorCode = 65;
            return false;
        }

        outNextIndex = consumeStatement(bodyIndex, shouldExecute);
        return outNextIndex >= 0;
    };

    consumeStatement = [&](int index, bool shouldExecute) -> int {
        if (index < 0 || index >= (int)statements.size()) {
            return -1;
        }

        const std::string statement = trim(statements[index].text);
        const int stmtLine = statements[index].line;

        if (statement == "}") {
            std::cerr << "[line " << stmtLine << "] Error at '}': Unexpected '}'" << std::endl;
            errorCode = 65;
            return -1;
        }

        if (statement == "{") {
            return consumeBlock(index, shouldExecute);
        }

        std::string firstCond;
        std::string firstTrailing;
        if (parseIfStatement(statement, firstCond, firstTrailing)) {
            if (firstCond.empty()) {
                std::cerr << "[line " << stmtLine << "] Error at ')': Expect expression." << std::endl;
                errorCode = 65;
                return -1;
            }

            int clauseIndex = index;
            bool branchTaken = false;
            bool clauseIsConditional = true;
            std::string clauseCondition = firstCond;
            std::string clauseTrailing = firstTrailing;

            while (true) {
                bool clauseMatches = false;
                if (clauseIsConditional) {
                    bool conditionValue = false;
                    if (shouldExecute) {
                        int code = evaluator.evaluateConditionFromString(clauseCondition, conditionValue);
                        if (code != 0) {
                            errorCode = code;
                            return -1;
                        }
                    }
                    clauseMatches = shouldExecute && conditionValue;
                } else {
                    clauseMatches = shouldExecute;
                }

                bool executeThisBody = clauseMatches && !branchTaken;
                int nextIndex = -1;
                if (!consumeBody(
                        clauseIndex,
                        clauseTrailing,
                        executeThisBody,
                        nextIndex,
                        clauseIsConditional ? "Expect statement after if condition." : "Expect statement after else.")) {
                    return -1;
                }

                if (executeThisBody) {
                    branchTaken = true;
                }

                if (nextIndex >= (int)statements.size()) {
                    return nextIndex;
                }

                ElseStatement elseInfo = parseElseStatement(statements[nextIndex].text);
                if (!elseInfo.matches) {
                    return nextIndex;
                }

                if (elseInfo.malformedElseIf) {
                    std::cerr << "[line " << statements[nextIndex].line << "] Error at 'if': Expect '(' after 'if'." << std::endl;
                    errorCode = 65;
                    return -1;
                }

                clauseIndex = nextIndex;
                clauseIsConditional = elseInfo.isElseIf;
                clauseCondition = elseInfo.condition;
                clauseTrailing = elseInfo.trailingStatement;

                if (clauseIsConditional && clauseCondition.empty()) {
                    std::cerr << "[line " << statements[clauseIndex].line << "] Error at ')': Expect expression." << std::endl;
                    errorCode = 65;
                    return -1;
                }
            }
        }

        ElseStatement danglingElse = parseElseStatement(statement);
        if (danglingElse.matches) {
            std::cerr << "[line " << stmtLine << "] Error at 'else': Unexpected 'else'." << std::endl;
            errorCode = 65;
            return -1;
        }

        int code = runMaybe(statement, stmtLine, shouldExecute);
        if (code != 0) {
            errorCode = code;
            return -1;
        }
        return index + 1;
    };

    int index = 0;
    while (index < (int)statements.size()) {
        index = consumeStatement(index, true);
        if (index < 0) {
            return errorCode == 0 ? 65 : errorCode;
        }
    }

    return 0;
}
