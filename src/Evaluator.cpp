#include "../include/Evaluator.h"
#include "Utils.h"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

int Evaluator::evaluateFromString(const std::string& source, bool printResult) {
    hasError_ = false;
    errorCode_ = 0;

    int i = 0;
    while (i < (int)source.size() && isspace((unsigned char)source[i])) {
        ++i;
    }

    Value result = parseExpression(source, i);
    if (hasError_) {
        return errorCode_;
    }

    while (i < (int)source.size() && isspace((unsigned char)source[i])) {
        ++i;
    }

    if (i < (int)source.size()) {
        reportExpectExpression(source, i);
        return errorCode_;
    }

    if (printResult) {
        std::cout << stringify(result) << std::endl;
    }
    return 0;
}

Evaluator::Value Evaluator::parseExpression(const std::string& source, int& i) {
    return parseEquality(source, i);
}

Evaluator::Value Evaluator::parseEquality(const std::string& source, int& i) {
    auto skipWhitespace = [&](void) {
        while (i < (int)source.size() && isspace((unsigned char)source[i])) {
            ++i;
        }
    };

    Value left = parseComparison(source, i);
    if (hasError_) return Value{};

    while (true) {
        skipWhitespace();
        if (i + 1 >= (int)source.size()) break;

        std::string op = source.substr(i, 2);
        if (op != "==" && op != "!=") break;

        i += 2;
        Value right = parseComparison(source, i);
        if (hasError_) return Value{};

        bool equal = isEqual(left, right);
        left = (op == "==") ? Value(equal) : Value(!equal);
    }

    return left;
}

Evaluator::Value Evaluator::parseComparison(const std::string& source, int& i) {
    auto skipWhitespace = [&](void) {
        while (i < (int)source.size() && isspace((unsigned char)source[i])) {
            ++i;
        }
    };

    Value left = parseAdditive(source, i);
    if (hasError_) return Value{};

    while (true) {
        skipWhitespace();
        if (i >= (int)source.size()) break;

        std::string op;
        if (i + 1 < (int)source.size()) {
            std::string two = source.substr(i, 2);
            if (two == "<=" || two == ">=") {
                op = two;
                i += 2;
            }
        }
        if (op.empty() && (source[i] == '<' || source[i] == '>')) {
            op = std::string(1, source[i]);
            ++i;
        }
        if (op.empty()) break;

        Value right = parseAdditive(source, i);
        if (hasError_) return Value{};

        double l = 0.0;
        double r = 0.0;
        if (!tryGetNumber(left, l) || !tryGetNumber(right, r)) {
            int line = lineNumberAt(source, i);
            std::cerr << "[line " << line << "] Error: Operands must be number." << std::endl;
            hasError_ = true;
            errorCode_ = 70;
            return Value{};
        }

        if (op == "<") left = Value(l < r);
        else if (op == ">") left = Value(l > r);
        else if (op == "<=") left = Value(l <= r);
        else left = Value(l >= r);
    }

    return left;
}

Evaluator::Value Evaluator::parseAdditive(const std::string& source, int& i) {
    auto skipWhitespace = [&](void) {
        while (i < (int)source.size() && isspace((unsigned char)source[i])) {
            ++i;
        }
    };

    Value left = parseMultiplicative(source, i);
    if (hasError_) return Value{};

    while (true) {
        skipWhitespace();
        if (i >= (int)source.size()) break;
        char op = source[i];
        if (op != '+' && op != '-') break;

        ++i;
        Value right = parseMultiplicative(source, i);
        if (hasError_) return Value{};

        if (op == '+') {
            double l = 0.0;
            double r = 0.0;
            if (tryGetNumber(left, l) && tryGetNumber(right, r)) {
                left = Value(l + r);
            } else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                left = Value(std::get<std::string>(left) + std::get<std::string>(right));
            } else {
                int line = lineNumberAt(source, i);
                std::cerr << "[line " << line << "] Error: Operands must be number." << std::endl;
                hasError_ = true;
                errorCode_ = 70;
                return Value{};
            }
        } else {
            double l = 0.0;
            double r = 0.0;
            if (!tryGetNumber(left, l) || !tryGetNumber(right, r)) {
                int line = lineNumberAt(source, i);
                std::cerr << "[line " << line << "] Error: Operands must be number." << std::endl;
                hasError_ = true;
                errorCode_ = 70;
                return Value{};
            }
            left = Value(l - r);
        }
    }

    return left;
}

Evaluator::Value Evaluator::parseMultiplicative(const std::string& source, int& i) {
    auto skipWhitespace = [&](void) {
        while (i < (int)source.size() && isspace((unsigned char)source[i])) {
            ++i;
        }
    };

    Value left = parseUnary(source, i);
    if (hasError_) return Value{};

    while (true) {
        skipWhitespace();
        if (i >= (int)source.size()) break;
        char op = source[i];
        if (op != '*' && op != '/') break;

        ++i;
        Value right = parseUnary(source, i);
        if (hasError_) return Value{};

        double l = 0.0;
        double r = 0.0;
        if (!tryGetNumber(left, l) || !tryGetNumber(right, r)) {
            int line = lineNumberAt(source, i);
            std::cerr << "[line " << line << "] Error: Operands must be number." << std::endl;
            hasError_ = true;
            errorCode_ = 70;
            return Value{};
        }

        if (op == '*') {
            left = Value(l * r);
        } else {
            left = Value(l / r);
        }
    }

    return left;
}

Evaluator::Value Evaluator::parseUnary(const std::string& source, int& i) {
    auto skipWhitespace = [&](void) {
        while (i < (int)source.size() && isspace((unsigned char)source[i])) {
            ++i;
        }
    };

    skipWhitespace();
    if (i < (int)source.size() && (source[i] == '!' || source[i] == '-')) {
        char op = source[i++];
        Value right = parseUnary(source, i);
        if (hasError_) return Value{};

        if (op == '!') {
            return Value(!isTruthy(right));
        }

        double r = 0.0;
        if (!tryGetNumber(right, r)) {
            int line = lineNumberAt(source, i);
            std::cerr << "[line " << line << "] Error: Operands must be number." << std::endl;
            hasError_ = true;
            errorCode_ = 70;
            return Value{};
        }
        return Value(-r);
    }

    return parsePrimary(source, i);
}

Evaluator::Value Evaluator::parsePrimary(const std::string& source, int& i) {
    auto skipWhitespace = [&](void) {
        while (i < (int)source.size() && isspace((unsigned char)source[i])) {
            ++i;
        }
    };

    skipWhitespace();
    if (i >= (int)source.size()) {
        reportExpectExpression(source, i);
        return Value{};
    }

    if (source[i] == '(') {
        ++i;
        Value expr = parseExpression(source, i);
        if (hasError_) return Value{};

        skipWhitespace();
        if (i < (int)source.size() && source[i] == ')') {
            ++i;
            return expr;
        }
        int line = lineNumberAt(source, i);
        std::cerr << "[line " << line << "] Error at 'end': Expect ')' after expression." << std::endl;
        hasError_ = true;
        errorCode_ = 65;
        return Value{};
    }

    if (i + 3 <= (int)source.size() && source.substr(i, 4) == "true") {
        i += 4;
        return Value(true);
    }
    if (i + 4 <= (int)source.size() && source.substr(i, 5) == "false") {
        i += 5;
        return Value(false);
    }
    if (i + 2 <= (int)source.size() && source.substr(i, 3) == "nil") {
        i += 3;
        return Value(std::monostate{});
    }

    if (isdigit((unsigned char)source[i])) {
        std::string number(1, source[i]);
        ++i;
        while (i < (int)source.size() && (isdigit((unsigned char)source[i]) || source[i] == '.')) {
            number += source[i];
            ++i;
        }
        return Value(NumberLiteral{number});
    }

    if (source[i] == '"') {
        std::string str;
        ++i;
        while (i < (int)source.size() && source[i] != '"') {
            str += source[i];
            ++i;
        }
        if (i >= (int)source.size()) {
            int line = lineNumberAt(source, i);
            std::cerr << "[line " << line << "] Error: Unterminated string." << std::endl;
            hasError_ = true;
            errorCode_ = 65;
            return Value{};
        }
        ++i;
        return Value(str);
    }

    reportExpectExpression(source, i);
    if (i < (int)source.size()) {
        ++i;
    }
    return Value{};
}

bool Evaluator::isTruthy(const Value& value) const {
    if (std::holds_alternative<std::monostate>(value)) return false;
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
    return true;
}

bool Evaluator::isEqual(const Value& left, const Value& right) const {
    double l = 0.0;
    double r = 0.0;
    if (tryGetNumber(left, l) && tryGetNumber(right, r)) {
        return l == r;
    }

    if (left.index() != right.index()) return false;
    if (std::holds_alternative<std::monostate>(left)) return true;
    if (std::holds_alternative<bool>(left)) return std::get<bool>(left) == std::get<bool>(right);
    if (std::holds_alternative<std::string>(left)) return std::get<std::string>(left) == std::get<std::string>(right);
    return false;
}

std::string Evaluator::stringify(const Value& value) const {
    if (std::holds_alternative<std::monostate>(value)) return "nil";
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
    if (std::holds_alternative<NumberLiteral>(value)) {
        return normalizeNumericText(std::get<NumberLiteral>(value).text);
    }
    if (std::holds_alternative<double>(value)) {
        std::ostringstream out;
        // Use a practical precision for display to avoid binary floating-point tails.
        out << std::setprecision(15);
        out << std::get<double>(value);
        return normalizeNumericText(out.str());
    }
    return std::get<std::string>(value);
}

bool Evaluator::tryGetNumber(const Value& value, double& out) const {
    if (std::holds_alternative<double>(value)) {
        out = std::get<double>(value);
        return true;
    }
    if (std::holds_alternative<NumberLiteral>(value)) {
        const std::string& text = std::get<NumberLiteral>(value).text;
        try {
            out = std::stod(text);
            return true;
        } catch (...) {
            return false;
        }
    }
    return false;
}

std::string Evaluator::normalizeNumericText(const std::string& text) const {
    std::string s = text;
    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') {
            s.pop_back();
        }
        if (!s.empty() && s.back() == '.') {
            s.pop_back();
        }
    }
    return s;
}

int Evaluator::lineNumberAt(const std::string& source, int index) const {
    int line = 1;
    for (int j = 0; j < index && j < (int)source.size(); ++j) {
        if (source[j] == '\n') {
            ++line;
        }
    }
    return line;
}

void Evaluator::reportExpectExpression(const std::string& source, int index) {
    int line = lineNumberAt(source, index);
    if (index < (int)source.size()) {
        std::cerr << "[line " << line << "] Error at '" << source[index] << "': Expect expression." << std::endl;
    } else {
        std::cerr << "[line " << line << "] Error at 'end': Expect expression." << std::endl;
    }
    hasError_ = true;
    errorCode_ = 65;
}
