#include "../include/Runner.h"
#include "../include/Evaluator.h"
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
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

    unsigned char next = (unsigned char)text[keyword.size()];
    return !(std::isalnum(next) || next == '_');
}

bool isIdentifierStartChar(char c) {
    return std::isalpha((unsigned char)c) || c == '_';
}

bool isIdentifierPartChar(char c) {
    return std::isalnum((unsigned char)c) || c == '_';
}

std::string tokenAt(const std::string& s, int i) {
    if (i >= (int)s.size()) {
        return "end";
    }

    if (isIdentifierStartChar(s[i])) {
        int start = i;
        while (i < (int)s.size() && isIdentifierPartChar(s[i])) {
            ++i;
        }
        return s.substr(start, i - start);
    }

    if (std::isdigit((unsigned char)s[i])) {
        int start = i;
        while (i < (int)s.size() && (std::isdigit((unsigned char)s[i]) || s[i] == '.')) {
            ++i;
        }
        return s.substr(start, i - start);
    }

    return std::string(1, s[i]);
}

struct FunctionHeaderParseResult {
    bool isFunctionDeclaration = false;
    bool ok = false;
    std::string functionName;
    std::vector<std::string> parameters;
    std::string errorToken;
    std::string errorMessage;
};

FunctionHeaderParseResult parseFunctionDeclarationHeaderDetailed(const std::string& statement) {
    FunctionHeaderParseResult result;
    std::string s = trim(statement);
    if (!startsWithKeyword(s, "fun")) {
        return result;
    }

    result.isFunctionDeclaration = true;

    int i = 3;
    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
        ++i;
    }

    if (i >= (int)s.size() || !isIdentifierStartChar(s[i])) {
        result.errorToken = tokenAt(s, i);
        result.errorMessage = "Expect function name.";
        return result;
    }

    int nameStart = i;
    while (i < (int)s.size() && isIdentifierPartChar(s[i])) {
        ++i;
    }
    result.functionName = s.substr(nameStart, i - nameStart);

    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
        ++i;
    }

    if (i >= (int)s.size() || s[i] != '(') {
        result.errorToken = tokenAt(s, i);
        result.errorMessage = "Expect '(' after function name.";
        return result;
    }
    ++i;

    result.parameters.clear();
    while (true) {
        while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
            ++i;
        }

        if (i >= (int)s.size()) {
            result.errorToken = "end";
            result.errorMessage = "Expect ')' after parameters.";
            return result;
        }

        if (s[i] == ')') {
            ++i;
            break;
        }

        if (!isIdentifierStartChar(s[i])) {
            result.errorToken = tokenAt(s, i);
            result.errorMessage = "Expect parameter name.";
            return result;
        }

        int paramStart = i;
        while (i < (int)s.size() && isIdentifierPartChar(s[i])) {
            ++i;
        }
        result.parameters.push_back(s.substr(paramStart, i - paramStart));

        while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
            ++i;
        }

        if (i >= (int)s.size()) {
            result.errorToken = "end";
            result.errorMessage = "Expect ')' after parameters.";
            return result;
        }

        if (s[i] == ',') {
            ++i;
            continue;
        }

        if (s[i] == ')') {
            ++i;
            break;
        }

        result.errorToken = tokenAt(s, i);
        result.errorMessage = "Expect ')' after parameters.";
        return result;
    }

    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
        ++i;
    }

    if (i != (int)s.size()) {
        result.errorToken = tokenAt(s, i);
        result.errorMessage = "Expect '{' before function body.";
        return result;
    }

    result.ok = true;
    return result;
}

bool parseFunctionDeclarationHeader(
    const std::string& statement,
    std::string& functionName,
    std::vector<std::string>& parameters) {
    FunctionHeaderParseResult detailed = parseFunctionDeclarationHeaderDetailed(statement);
    if (!detailed.isFunctionDeclaration || !detailed.ok) {
        return false;
    }

    functionName = detailed.functionName;
    parameters = detailed.parameters;
    return true;
}

bool parseFunctionCallExpression(
    const std::string& expression,
    std::string& functionName,
    std::vector<std::string>& arguments) {
    std::string s = trim(expression);
    if (s.empty()) {
        return false;
    }

    int i = 0;
    if (!isIdentifierStartChar(s[i])) {
        return false;
    }

    int nameStart = i;
    while (i < (int)s.size() && isIdentifierPartChar(s[i])) {
        ++i;
    }
    functionName = s.substr(nameStart, i - nameStart);

    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
        ++i;
    }

    if (i >= (int)s.size() || s[i] != '(') {
        return false;
    }
    ++i;

    arguments.clear();
    std::string current;
    int depth = 0;
    bool inString = false;

    while (i < (int)s.size()) {
        char c = s[i];

        if (c == '"') {
            inString = !inString;
            current += c;
            ++i;
            continue;
        }

        if (!inString) {
            if (c == '(') {
                ++depth;
                current += c;
                ++i;
                continue;
            }

            if (c == ')') {
                if (depth == 0) {
                    std::string last = trim(current);
                    if (!last.empty()) {
                        arguments.push_back(last);
                    }
                    ++i;
                    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
                        ++i;
                    }
                    return i == (int)s.size();
                }
                --depth;
                current += c;
                ++i;
                continue;
            }

            if (c == ',' && depth == 0) {
                std::string arg = trim(current);
                if (arg.empty()) {
                    return false;
                }
                arguments.push_back(arg);
                current.clear();
                ++i;
                continue;
            }
        }

        current += c;
        ++i;
    }

    return false;
}

struct DeclarationStatement {
    bool matches = false;
    std::string keyword;
    std::string declaration;
    Evaluator::VariableType type = Evaluator::VariableType::ANY;
};

DeclarationStatement parseDeclarationStatement(const std::string& statement) {
    DeclarationStatement result;
    std::string s = trim(statement);

    if (startsWithKeyword(s, "auto")) {
        result.matches = true;
        result.keyword = "auto";
        result.declaration = trim(s.substr(4));
        result.type = Evaluator::VariableType::ANY;
        return result;
    }
    if (startsWithKeyword(s, "num")) {
        result.matches = true;
        result.keyword = "num";
        result.declaration = trim(s.substr(3));
        result.type = Evaluator::VariableType::NUM;
        return result;
    }
    if (startsWithKeyword(s, "string")) {
        result.matches = true;
        result.keyword = "string";
        result.declaration = trim(s.substr(6));
        result.type = Evaluator::VariableType::STRING;
        return result;
    }
    if (startsWithKeyword(s, "bool")) {
        result.matches = true;
        result.keyword = "bool";
        result.declaration = trim(s.substr(4));
        result.type = Evaluator::VariableType::BOOL;
        return result;
    }

    return result;
}

bool isDeclarationStatement(const std::string& statement) {
    return parseDeclarationStatement(statement).matches;
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

bool parseWhileStatement(const std::string& statement, std::string& condition, std::string& trailingStatement) {
    std::string s = trim(statement);
    if (!startsWithKeyword(s, "while")) {
        return false;
    }

    int i = 5;
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

bool parseForStatement(
    const std::string& statement,
    std::string& initializer,
    std::string& condition,
    std::string& increment,
    std::string& trailingStatement) {
    std::string s = trim(statement);
    if (!startsWithKeyword(s, "for")) {
        return false;
    }

    int i = 3;
    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
        ++i;
    }

    if (i >= (int)s.size() || s[i] != '(') {
        return false;
    }

    int headerStart = ++i;
    int depth = 1;
    bool inString = false;
    while (i < (int)s.size() && depth > 0) {
        char c = s[i];
        if (c == '"') {
            inString = !inString;
        } else if (!inString) {
            if (c == '(') {
                ++depth;
            } else if (c == ')') {
                --depth;
            }
        }
        ++i;
    }

    if (depth != 0) {
        return false;
    }

    int headerEnd = i - 1;
    while (i < (int)s.size() && std::isspace((unsigned char)s[i])) {
        ++i;
    }
    trailingStatement = trim(s.substr(i));

    std::string header = s.substr(headerStart, headerEnd - headerStart);
    int semi1 = -1;
    int semi2 = -1;
    int nestedParens = 0;
    inString = false;
    for (int j = 0; j < (int)header.size(); ++j) {
        char c = header[j];
        if (c == '"') {
            inString = !inString;
            continue;
        }
        if (inString) {
            continue;
        }
        if (c == '(') {
            ++nestedParens;
            continue;
        }
        if (c == ')') {
            if (nestedParens > 0) {
                --nestedParens;
            }
            continue;
        }
        if (c == ';' && nestedParens == 0) {
            if (semi1 < 0) {
                semi1 = j;
            } else if (semi2 < 0) {
                semi2 = j;
            } else {
                return false;
            }
        }
    }

    if (semi1 < 0 || semi2 < 0) {
        return false;
    }

    initializer = trim(header.substr(0, semi1));
    condition = trim(header.substr(semi1 + 1, semi2 - semi1 - 1));
    increment = trim(header.substr(semi2 + 1));
    return true;
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

    std::string whileCondition;
    std::string whileTrailing;
    if (parseWhileStatement(statement, whileCondition, whileTrailing) && whileTrailing.empty()) {
        return true;
    }

    std::string forInit;
    std::string forCond;
    std::string forInc;
    std::string forTrailing;
    if (parseForStatement(statement, forInit, forCond, forInc, forTrailing) && forTrailing.empty()) {
        return true;
    }

    std::string functionName;
    std::vector<std::string> functionParameters;
    if (parseFunctionDeclarationHeader(statement, functionName, functionParameters)) {
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
    int parenDepth = 0;
    int blockDepth = 0;
    std::vector<int> blockStartLines;
    struct PendingStatement {
        std::string text;
        int line;
    };

    struct FunctionDefinition {
        std::vector<std::string> parameters;
        int bodyStartIndex = -1;
        int bodyEndIndex = -1;
    };

    std::vector<PendingStatement> statements;
    std::map<std::string, FunctionDefinition> functions;
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

        if (!inString) {
            if (c == '(') {
                ++parenDepth;
            } else if (c == ')' && parenDepth > 0) {
                --parenDepth;
            }
        }

        if (!inString && c == '{') {
            std::string beforeBrace = trim(current);
            if (!beforeBrace.empty()) {
                FunctionHeaderParseResult funHeader = parseFunctionDeclarationHeaderDetailed(beforeBrace);
                if (funHeader.isFunctionDeclaration && !funHeader.ok) {
                    std::cerr << "[line " << line << "] Error at '" << funHeader.errorToken
                              << "': " << funHeader.errorMessage << std::endl;
                    return 65;
                }

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

        if (!inString && parenDepth == 0 && c == ';') {
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

    struct ReturnSignal {
        Evaluator::Value value;
    };

    struct BreakSignal {};

    struct LoopDepthGuard {
        int& depth;
        explicit LoopDepthGuard(int& d) : depth(d) { ++depth; }
        ~LoopDepthGuard() { --depth; }
    };

    int functionExecutionDepth = 0;
    int loopExecutionDepth = 0;
    std::function<int(const std::string&, const std::vector<Evaluator::Value>&, int, Evaluator::Value&)> invokeUserFunction;
    evaluator.setUserFunctionHandler([&](const std::string& name,
                                        const std::vector<Evaluator::Value>& args,
                                        int callLine,
                                        Evaluator::Value& outValue) -> int {
        if (!invokeUserFunction) {
            return 70;
        }
        return invokeUserFunction(name, args, callLine, outValue);
    });

    auto runRegularStatement = [&](const std::string& statement, int stmtLine) -> int {
        bool isPrintStatement =
            statement.rfind("print", 0) == 0 &&
            (statement.size() == 5 || std::isspace((unsigned char)statement[5]));

        DeclarationStatement declaration = parseDeclarationStatement(statement);

        if (declaration.matches) {
            if (declaration.declaration.empty()) {
                std::cerr << "[line " << stmtLine << "] Error at ';': Expect variable name." << std::endl;
                return 65;
            }

            int code = evaluator.declareVariableFromString(declaration.declaration, declaration.type);
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

    auto runForInitializer = [&](const std::string& initializer, int stmtLine) -> int {
        std::string init = trim(initializer);
        if (init.empty()) {
            return 0;
        }

        DeclarationStatement declaration = parseDeclarationStatement(init);
        if (declaration.matches) {
            if (declaration.declaration.empty()) {
                std::cerr << "[line " << stmtLine << "] Error at ';': Expect variable name." << std::endl;
                return 65;
            }
            return evaluator.declareVariableFromString(declaration.declaration, declaration.type);
        }

        return evaluator.evaluateFromString(init, false);
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
                           const std::string& missingBodyMessage,
                           bool allowVarDeclarationSingleStatement) -> bool {
        if (!inlineBody.empty()) {
            PendingStatement synthetic{trim(inlineBody), statements[headerIndex].line};
            if (synthetic.text.empty()) {
                std::cerr << "[line " << statements[headerIndex].line << "] Error at 'end': " << missingBodyMessage << std::endl;
                errorCode = 65;
                return false;
            }

            DeclarationStatement syntheticDecl = parseDeclarationStatement(synthetic.text);
            if (!allowVarDeclarationSingleStatement && syntheticDecl.matches) {
                std::cerr << "[line " << statements[headerIndex].line << "] Error at '"
                          << syntheticDecl.keyword << "': Expect expression." << std::endl;
                errorCode = 65;
                return false;
            }

            const int insertIndex = headerIndex + 1;
            statements.insert(statements.begin() + insertIndex, synthetic);

            int consumedNext = -1;
            try {
                consumedNext = consumeStatement(insertIndex, shouldExecute);
            } catch (...) {
                statements.erase(statements.begin() + insertIndex);
                throw;
            }

            statements.erase(statements.begin() + insertIndex);

            if (consumedNext < 0) {
                return false;
            }

            outNextIndex = consumedNext > insertIndex ? consumedNext - 1 : consumedNext;
            return true;
        }

        int bodyIndex = headerIndex + 1;
        if (bodyIndex >= (int)statements.size()) {
            std::cerr << "[line " << statements[headerIndex].line << "] Error at 'end': " << missingBodyMessage << std::endl;
            errorCode = 65;
            return false;
        }

        std::string bodyStatement = trim(statements[bodyIndex].text);
        DeclarationStatement bodyDecl = parseDeclarationStatement(bodyStatement);
        if (!allowVarDeclarationSingleStatement && bodyDecl.matches) {
            std::cerr << "[line " << statements[bodyIndex].line << "] Error at '"
                      << bodyDecl.keyword << "': Expect expression." << std::endl;
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

        FunctionHeaderParseResult functionHeader = parseFunctionDeclarationHeaderDetailed(statement);
        if (functionHeader.isFunctionDeclaration && !functionHeader.ok) {
            std::cerr << "[line " << stmtLine << "] Error at '" << functionHeader.errorToken
                      << "': " << functionHeader.errorMessage << std::endl;
            errorCode = 65;
            return -1;
        }

        std::string declaredFunctionName;
        std::vector<std::string> declaredFunctionParameters;
        if (parseFunctionDeclarationHeader(statement, declaredFunctionName, declaredFunctionParameters)) {
            int bodyStart = index + 1;
            if (bodyStart >= (int)statements.size() || trim(statements[bodyStart].text) != "{") {
                std::cerr << "[line " << stmtLine << "] Error at 'end': Expect '{' before function body." << std::endl;
                errorCode = 65;
                return -1;
            }

            int depth = 0;
            int bodyEnd = -1;
            for (int i = bodyStart; i < (int)statements.size(); ++i) {
                std::string token = trim(statements[i].text);
                if (token == "{") {
                    ++depth;
                } else if (token == "}") {
                    --depth;
                    if (depth == 0) {
                        bodyEnd = i;
                        break;
                    }
                }
            }

            if (bodyEnd < 0) {
                std::cerr << "[line " << stmtLine << "] Error at 'end': Expect '}' after function body." << std::endl;
                errorCode = 65;
                return -1;
            }

            if (shouldExecute) {
                functions[declaredFunctionName] = FunctionDefinition{declaredFunctionParameters, bodyStart, bodyEnd};
                evaluator.defineVariable(declaredFunctionName, std::string("<fn ") + declaredFunctionName + ">");
            }

            return bodyEnd + 1;
        }

        if (startsWithKeyword(statement, "return")) {
            if (!shouldExecute) {
                return index + 1;
            }

            if (functionExecutionDepth <= 0) {
                std::cerr << "[line " << stmtLine << "] Error at 'return': Can't return from top-level code." << std::endl;
                errorCode = 65;
                return -1;
            }

            std::string valueExpression = trim(statement.substr(6));
            Evaluator::Value returnValue = std::monostate{};
            if (!valueExpression.empty()) {
                int returnCode = evaluator.evaluateValueFromString(valueExpression, returnValue);
                if (returnCode != 0) {
                    errorCode = returnCode;
                    return -1;
                }
            }

            throw ReturnSignal{returnValue};
        }

        if (startsWithKeyword(statement, "break")) {
            std::string trailing = trim(statement.substr(5));
            if (!trailing.empty()) {
                std::cerr << "[line " << stmtLine << "] Error at '" << tokenAt(trailing, 0)
                          << "': Expect ';' after break." << std::endl;
                errorCode = 65;
                return -1;
            }

            if (!shouldExecute) {
                return index + 1;
            }

            if (loopExecutionDepth <= 0) {
                std::cerr << "[line " << stmtLine << "] Error at 'break': Can't break from top-level code." << std::endl;
                errorCode = 65;
                return -1;
            }

            throw BreakSignal{};
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
                        clauseIsConditional ? "Expect statement after if condition." : "Expect statement after else.",
                        false)) {
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

        std::string whileCondition;
        std::string whileTrailing;
        if (parseWhileStatement(statement, whileCondition, whileTrailing)) {
            if (whileCondition.empty()) {
                std::cerr << "[line " << stmtLine << "] Error at ')': Expect expression." << std::endl;
                errorCode = 65;
                return -1;
            }

            if (!shouldExecute) {
                int nextIndex = -1;
                if (!consumeBody(index, whileTrailing, false, nextIndex, "Expect statement after while condition.", true)) {
                    return -1;
                }
                return nextIndex;
            }

            LoopDepthGuard loopGuard(loopExecutionDepth);

            while (true) {
                bool conditionValue = false;
                int code = evaluator.evaluateConditionFromString(whileCondition, conditionValue);
                if (code != 0) {
                    errorCode = code;
                    return -1;
                }

                int nextIndex = -1;
                if (!conditionValue) {
                    if (!consumeBody(index, whileTrailing, false, nextIndex, "Expect statement after while condition.", true)) {
                        return -1;
                    }
                    return nextIndex;
                }

                try {
                    if (!consumeBody(index, whileTrailing, true, nextIndex, "Expect statement after while condition.", true)) {
                        return -1;
                    }
                } catch (const BreakSignal&) {
                    int breakNext = -1;
                    if (!consumeBody(index, whileTrailing, false, breakNext, "Expect statement after while condition.", true)) {
                        return -1;
                    }
                    return breakNext;
                }
            }
        }

        std::string forInitializer;
        std::string forCondition;
        std::string forIncrement;
        std::string forTrailing;
        if (parseForStatement(statement, forInitializer, forCondition, forIncrement, forTrailing)) {
            if (!shouldExecute) {
                int nextIndex = -1;
                if (!consumeBody(index, forTrailing, false, nextIndex, "Expect statement after for clauses.", true)) {
                    return -1;
                }
                return nextIndex;
            }

            LoopDepthGuard loopGuard(loopExecutionDepth);

            evaluator.beginScope();
            bool forScopeActive = true;

            auto closeForScope = [&]() -> bool {
                if (!forScopeActive) {
                    return true;
                }
                forScopeActive = false;
                int closeCode = evaluator.endScope();
                if (closeCode != 0) {
                    std::cerr << "[line " << stmtLine << "] Error at 'for': Invalid loop scope." << std::endl;
                    errorCode = closeCode;
                    return false;
                }
                return true;
            };

            int initCode = runForInitializer(forInitializer, stmtLine);
            if (initCode != 0) {
                errorCode = initCode;
                closeForScope();
                return -1;
            }

            while (true) {
                bool conditionValue = true;
                if (!forCondition.empty()) {
                    int condCode = evaluator.evaluateConditionFromString(forCondition, conditionValue);
                    if (condCode != 0) {
                        errorCode = condCode;
                        closeForScope();
                        return -1;
                    }
                }

                int nextIndex = -1;
                if (!conditionValue) {
                    if (!consumeBody(index, forTrailing, false, nextIndex, "Expect statement after for clauses.", true)) {
                        closeForScope();
                        return -1;
                    }
                    if (!closeForScope()) {
                        return -1;
                    }
                    return nextIndex;
                }

                try {
                    if (!consumeBody(index, forTrailing, true, nextIndex, "Expect statement after for clauses.", true)) {
                        closeForScope();
                        return -1;
                    }
                } catch (const BreakSignal&) {
                    int breakNext = -1;
                    if (!consumeBody(index, forTrailing, false, breakNext, "Expect statement after for clauses.", true)) {
                        closeForScope();
                        return -1;
                    }
                    if (!closeForScope()) {
                        return -1;
                    }
                    return breakNext;
                }

                if (!forIncrement.empty()) {
                    int incCode = evaluator.evaluateFromString(forIncrement, false);
                    if (incCode != 0) {
                        errorCode = incCode;
                        closeForScope();
                        return -1;
                    }
                }
            }
        }

        if (startsWithKeyword(statement, "while")) {
            std::cerr << "[line " << stmtLine << "] Error at 'while': Expect '(' after 'while'." << std::endl;
            errorCode = 65;
            return -1;
        }

        if (startsWithKeyword(statement, "if")) {
            std::cerr << "[line " << stmtLine << "] Error at 'if': Expect '(' after 'if'." << std::endl;
            errorCode = 65;
            return -1;
        }

        if (startsWithKeyword(statement, "for")) {
            std::cerr << "[line " << stmtLine << "] Error at 'for': Expect '(' after 'for'." << std::endl;
            errorCode = 65;
            return -1;
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

    invokeUserFunction = [&](const std::string& functionName,
                             const std::vector<Evaluator::Value>& args,
                             int callLine,
                             Evaluator::Value& outValue) -> int {
        auto functionIt = functions.find(functionName);
        if (functionIt == functions.end()) {
            std::cerr << "Error: Undefined function '" << functionName << "'." << std::endl;
            return 70;
        }

        const FunctionDefinition& function = functionIt->second;
        if (args.size() != function.parameters.size()) {
            std::cerr << "Expected " << function.parameters.size() << " arguments but got "
                      << args.size() << "." << std::endl;
            std::cerr << "[line " << callLine << "]" << std::endl;
            return 70;
        }

        evaluator.beginScope();
        ++functionExecutionDepth;
        int savedLoopExecutionDepth = loopExecutionDepth;
        loopExecutionDepth = 0;

        for (int argIndex = 0; argIndex < (int)args.size(); ++argIndex) {
            evaluator.defineVariable(function.parameters[argIndex], args[argIndex]);
        }

        outValue = std::monostate{};
        try {
            int bodyCursor = function.bodyStartIndex + 1;
            while (bodyCursor < function.bodyEndIndex) {
                bodyCursor = consumeStatement(bodyCursor, true);
                if (bodyCursor < 0) {
                    int closeCode = evaluator.endScope();
                    --functionExecutionDepth;
                    loopExecutionDepth = savedLoopExecutionDepth;
                    if (closeCode != 0) {
                        return closeCode;
                    }
                    return errorCode == 0 ? 65 : errorCode;
                }
            }
        } catch (const ReturnSignal& ret) {
            outValue = ret.value;
        }

        int closeCode = evaluator.endScope();
        --functionExecutionDepth;
        loopExecutionDepth = savedLoopExecutionDepth;
        if (closeCode != 0) {
            return closeCode;
        }

        return 0;
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
