#include "IterationFormula.h"

#include <sstream>
#include <vector>
#include <stack>
#include <regex>

using mnd::ParseError;


mnd::IterationFormula::IterationFormula(mnd::Expression expr) :
    expr{ std::make_unique<mnd::Expression>(std::move(expr)) }
{
}


static const std::string regexIdent = "[A-Za-z][A-Za-z0-9]*";
static const std::string regexNum = "[1-9][0-9]*";
static const std::string regexFloat = "(\\d*\\.?\\d+|\\d+\\.?\\d*)([eE][-+]\\d+)?";


class Parser
{
    static const std::regex tokenize;
    static const std::regex ident;
    static const std::regex num;
    static const std::regex floatNum;
    std::string in;
    std::regex_iterator<std::string::iterator> rit;


    std::vector<std::string> tokens;
    std::stack<char> operators;
    std::vector<mnd::Expression> output;
public:
    Parser(const std::string& s) :
        in{ s },
        rit{ in.begin(), in.end(), tokenize }
    {}

    void parse(void)
    {
        std::string token;
        while (getToken(token)) {
            if (std::regex_match(token, num)) {
                output.push_back(mnd::Constant{ std::atof(token.c_str()) });
            }
            else if (std::regex_match(token, floatNum)) {
                output.push_back(mnd::Constant{ std::atof(token.c_str()) });
            }
            else if (std::regex_match(token, ident)) {
                output.push_back(mnd::Variable{ token });
            }
            else if (token == "+" || token == "-") {
                while (!operators.empty() && getTopPrecedence() >= 1) {
                    popOperator();
                }
                operators.push(token[0]);
            }
            else if (token == "*" || token == "/") {
                while (!operators.empty() && getTopPrecedence() >= 2) {
                    popOperator();
                }
                operators.push(token[0]);
            }
            else if (token == "^") {
                while (!operators.empty() && getTopPrecedence() > 3) {
                    popOperator();
                }
                operators.push(token[0]);
            }
            else if (token == "(") {
                operators.push(token[0]);
            }
            else if (token == ")") {
                while (operators.top() != '(') {
                    popOperator();
                }
                operators.pop();
            }
        }
        while (!operators.empty())
            popOperator();
    }

    mnd::Expression& getExpression(void) {
        return output[0];
    }

    void popOperator(void) {
        if (operators.empty()) {
            throw ParseError("error parsing expression");
        }
        char top = operators.top();
        if (output.size() < 2) {
            throw ParseError(std::string("not enough operands for operator '") + top + "'");
        }
        operators.pop();
        mnd::Expression& left = output.at(output.size() - 2);
        mnd::Expression& right = output.at(output.size() - 1);
        mnd::Expression newExpr = mnd::Constant{ 0.0 };
        if (top == '+' || top == '-') {
            newExpr = mnd::Addition {
                std::make_unique<mnd::Expression>(std::move(left)),
                std::make_unique<mnd::Expression>(std::move(right)),
                top == '-'
            };
        }
        else if (top == '*') {
            newExpr = mnd::Multiplication {
                std::make_unique<mnd::Expression>(std::move(left)),
                std::make_unique<mnd::Expression>(std::move(right))
            };
        }
        else if (top == '/') {
            newExpr = mnd::Division {
                std::make_unique<mnd::Expression>(std::move(left)),
                std::make_unique<mnd::Expression>(std::move(right))
            };
        }
        else if (top == '^') {
            newExpr = mnd::Pow {
                std::make_unique<mnd::Expression>(std::move(left)),
                std::make_unique<mnd::Expression>(std::move(right))
            };
        }
        else {
            throw ParseError(std::string("not a valid operator: ") + top);
        }
        output.pop_back();
        output.pop_back();
        output.push_back(std::move(newExpr));
    }

    int getTopPrecedence(void) const {
        return getPrecedence(operators.top());
    }

    int getPrecedence(char op) const {
        char t = op;
        if (t == '+' || t == '-')
            return 1;
        else if (t == '*' || t == '/')
            return 2;
        else if (t == '^')
            return 3;
        return 0;
    }

private:
    bool getToken(std::string& token)
    {
        if (rit != std::regex_iterator<std::string::iterator>()) {
            token = rit->str();
            ++rit;
            return true;
        }
        return false;
    }
};

const std::regex Parser::tokenize = std::regex(regexIdent + "|" + regexFloat + "|[\\+\\-\\*/\\^]|[\\(\\)]");
const std::regex Parser::ident = std::regex(regexIdent);
const std::regex Parser::num = std::regex(regexNum);
const std::regex Parser::floatNum = std::regex(regexFloat);



namespace mnd
{
    Expression parse(const std::string& formula)
    {
        Parser p(formula);
        p.parse();
        mnd::Expression& res = p.getExpression();
        //printf("expr: %s\n", toString(res).c_str());
        return std::move(res);
    }

    std::string toString(const mnd::Expression& expr)
    {
        return std::visit([] (auto&& ex) -> std::string {
            std::stringstream ss;
            using T = std::decay_t<decltype(ex)>;
            if constexpr (std::is_same<T, mnd::Constant>::value) {
                ss << "const[" << ex.value << "]";
                return ss.str();
            }
            else if constexpr (std::is_same<T, mnd::Variable>::value) {
                return std::string("var[") + ex.name + "]";
            }
            else if constexpr (std::is_same<T, mnd::UnaryOperation>::value) {
                return std::string("-(") + toString(*ex.operand) + ")";
            }
            else if constexpr (std::is_same<T, mnd::Addition>::value) {
                return std::string("(") + toString(*ex.left) + std::string(") + (") + toString(*ex.right) + ")";
            }
            else if constexpr (std::is_same<T, mnd::Multiplication>::value) {
                return std::string("(") + toString(*ex.left) + std::string(") * (") + toString(*ex.right) + ")";
            }
            else if constexpr (std::is_same<T, mnd::Division>::value) {
                return std::string("(") + toString(*ex.left) + std::string(") / (") + toString(*ex.right) + ")";
            }
            else if constexpr (std::is_same<T, mnd::Pow>::value) {
                return std::string("(") + toString(*ex.left) + std::string(") ^ (") + toString(*ex.right) + ")";
            }
            return "";
        }, expr);
    }
}



