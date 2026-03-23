#ifndef CODECRAFTERS_EVALUATOR_H
#define CODECRAFTERS_EVALUATOR_H

#include <string>
#include <map>
#include <variant>
#include <vector>
#include <functional>

class Evaluator {
public:
    struct NumberLiteral {
        std::string text;
    };

    struct NativeFunction {
        std::string name;
    };

    using Value = std::variant<std::monostate, NumberLiteral, double, bool, std::string, NativeFunction>; // nil, number literal, evaluated number, bool, string, native function
    using UserFunctionHandler = std::function<int(const std::string&, const std::vector<Value>&, Value&)>;

    Evaluator();
    int evaluateFromString(const std::string& source, bool printResult = true);
    int evaluateValueFromString(const std::string& source, Value& outValue);
    int evaluateConditionFromString(const std::string& source, bool& outTruthValue);
    int declareVariableFromString(const std::string& source);
    void defineVariable(const std::string& name, const Value& value);
    void setUserFunctionHandler(UserFunctionHandler handler);
    void beginScope();
    int endScope();

private:
    Value parseExpression(const std::string& source, int& i);
    Value parseAssignment(const std::string& source, int& i);
    Value parseOr(const std::string& source, int& i);
    Value parseAnd(const std::string& source, int& i);
    Value parseEquality(const std::string& source, int& i);
    Value parseComparison(const std::string& source, int& i);
    Value parseAdditive(const std::string& source, int& i);
    Value parseMultiplicative(const std::string& source, int& i);
    Value parseUnary(const std::string& source, int& i);
    Value parseCall(const std::string& source, int& i);
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
    int suppressedEvalDepth_ = 0;
    UserFunctionHandler userFunctionHandler_;
    std::vector<std::map<std::string, Value>> scopes_ = {std::map<std::string, Value>{}};
};

#endif // CODECRAFTERS_EVALUATOR_H
