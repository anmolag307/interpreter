#ifndef CODECRAFTERS_EVALUATOR_H
#define CODECRAFTERS_EVALUATOR_H

#include <string>
#include <map>
#include <variant>
#include <vector>

class Evaluator {
public:
    Evaluator() = default;
    int evaluateFromString(const std::string& source, bool printResult = true);
    int declareVariableFromString(const std::string& source);
    void beginScope();
    int endScope();

private:
    struct NumberLiteral {
        std::string text;
    };

    using Value = std::variant<std::monostate, NumberLiteral, double, bool, std::string>; // nil, number literal, evaluated number, bool, string

    Value parseExpression(const std::string& source, int& i);
    Value parseAssignment(const std::string& source, int& i);
    Value parseEquality(const std::string& source, int& i);
    Value parseComparison(const std::string& source, int& i);
    Value parseAdditive(const std::string& source, int& i);
    Value parseMultiplicative(const std::string& source, int& i);
    Value parseUnary(const std::string& source, int& i);
    Value parsePrimary(const std::string& source, int& i);

    bool isTruthy(const Value& value) const;
    bool isEqual(const Value& left, const Value& right) const;
    std::string stringify(const Value& value) const;
    bool tryGetNumber(const Value& value, double& out) const;
    std::string normalizeNumericText(const std::string& text) const;
    int lineNumberAt(const std::string& source, int index) const;
    void reportExpectExpression(const std::string& source, int index);
    bool isIdentifierStart(char c) const;
    bool isIdentifierPart(char c) const;
    bool matchesKeyword(const std::string& source, int index, const std::string& keyword) const;

    bool hasError_ = false;
    int errorCode_ = 0;
    std::vector<std::map<std::string, Value>> scopes_ = {std::map<std::string, Value>{}};
};

#endif // CODECRAFTERS_EVALUATOR_H
