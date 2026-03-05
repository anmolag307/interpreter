#ifndef CODECRAFTERS_EVALUATOR_H
#define CODECRAFTERS_EVALUATOR_H

#include <string>
#include <variant>

class Evaluator {
public:
    Evaluator() = default;
    int evaluateFromString(const std::string& source);

private:
    using Value = std::variant<std::monostate, double, bool, std::string>; // nil, number, bool, string

    Value parseExpression(const std::string& source, int& i);
    Value parseEquality(const std::string& source, int& i);
    Value parseComparison(const std::string& source, int& i);
    Value parseAdditive(const std::string& source, int& i);
    Value parseMultiplicative(const std::string& source, int& i);
    Value parseUnary(const std::string& source, int& i);
    Value parsePrimary(const std::string& source, int& i);

    bool isTruthy(const Value& value) const;
    bool isEqual(const Value& left, const Value& right) const;
    std::string stringify(const Value& value) const;
    int lineNumberAt(const std::string& source, int index) const;
    void reportExpectExpression(const std::string& source, int index);

    bool hasError_ = false;
    int errorCode_ = 0;
};

#endif // CODECRAFTERS_EVALUATOR_H
