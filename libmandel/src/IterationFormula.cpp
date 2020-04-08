#include "IterationFormula.h"

#include <sstream>
#include <vector>
#include <stack>
#include <regex>
#include <optional>

using mnd::ParseError;


mnd::IterationFormula::IterationFormula(std::unique_ptr<Expression> expr, const std::vector<std::string>& variables) :
    expr{ std::move(expr) },
    variables{ variables }
{
    this->variables.push_back("i");
    auto maybeUnknown = findUnknownVariables(*this->expr);
    if (maybeUnknown.has_value()) {
        throw ParseError(std::string("unknown variable: ") + maybeUnknown.value());
    }
}


mnd::IterationFormula::IterationFormula(mnd::Expression expr, const std::vector<std::string>& variables) :
    IterationFormula{ std::make_unique<mnd::Expression>(std::move(expr)), variables }
{
}


struct SimpleOptimizer
{
    using Ret = std::optional<mnd::Expression>;
    void visitExpr(std::unique_ptr<mnd::Expression>& expr)
    {
        Ret replacement = std::visit(*this, *expr);
        if (replacement.has_value()) {
            expr = std::make_unique<mnd::Expression>(std::move(replacement.value()));
        }
    }

    Ret operator() (mnd::Constant& c)
    {
        return std::nullopt;
    }

    Ret operator() (mnd::Variable& v)
    {
        if (v.name == "i") {
            return mnd::Constant{ 0.0, 1.0 };
        }
        else {
            return std::nullopt;
        }
    }

    Ret operator() (mnd::Negation& n)
    {
        visitExpr(n.operand);
        auto* valConst = std::get_if<mnd::Constant>(n.operand.get());

        if (valConst) {
            return mnd::Constant {
                -valConst->re,
                -valConst->im
            };
        }
        return std::nullopt;
    }

    Ret operator() (mnd::Addition& a)
    {
        visitExpr(a.left);
        visitExpr(a.right);
        auto* leftConst = std::get_if<mnd::Constant>(a.left.get());
        auto* rightConst = std::get_if<mnd::Constant>(a.right.get());

        if (leftConst && rightConst) {
            if (a.subtraction) {
                return mnd::Constant {
                    leftConst->re - rightConst->re,
                    leftConst->im - rightConst->im
                };
            }
            else {
                return mnd::Constant{
                    leftConst->re + rightConst->re,
                    leftConst->im + rightConst->im
                };
            }
        }
        return std::nullopt;
    }

    Ret operator() (mnd::Multiplication& a)
    {
        visitExpr(a.left);
        visitExpr(a.right);
        auto* leftConst = std::get_if<mnd::Constant>(a.left.get());
        auto* rightConst = std::get_if<mnd::Constant>(a.right.get());

        if (leftConst && rightConst) {
            return mnd::Constant {
                leftConst->re * rightConst->re - leftConst->im * rightConst->im,
                (leftConst->re * rightConst->im + leftConst->im * rightConst->re) * 2
            };
        }
        return std::nullopt;
    }

    Ret operator() (mnd::Division& a)
    {
        visitExpr(a.left);
        visitExpr(a.right);

        auto* leftConst = std::get_if<mnd::Constant>(a.left.get());
        auto* rightConst = std::get_if<mnd::Constant>(a.right.get());

        if (leftConst && rightConst) {
            return mnd::Constant {
                (leftConst->re * rightConst->re + leftConst->im * rightConst->im) /
                (rightConst->re * rightConst->re + rightConst->im * rightConst->im),
                (leftConst->im * rightConst->re - leftConst->re * rightConst->im) /
                (rightConst->re * rightConst->re + rightConst->im * rightConst->im)
            };
        }

        return std::nullopt;
    }

    Ret operator() (mnd::Pow& a)
    {
        visitExpr(a.left);
        visitExpr(a.right);
        auto* leftConst = std::get_if<mnd::Constant>(a.left.get());
        auto* rightConst = std::get_if<mnd::Constant>(a.right.get());

        if (rightConst) {
            if (rightConst->im == 0) {
                a.realExponent = true;
                if (int(rightConst->re) == rightConst->re) {
                    a.integerExponent = true;
                }
            }
        }

        return std::nullopt;
    }
};


std::optional<std::string> mnd::IterationFormula::findUnknownVariables(const Expression& expr)
{
    std::string unknownVariable;
    std::function<bool(const Expression&)> isCorrect;
    auto corrLambda = [this, &isCorrect, &unknownVariable](const auto& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same<T, mnd::Variable>::value) {
            if (containsVariable(x.name)) {
                return true;
            }
            else {
                unknownVariable = x.name;
                return false;
            }
        }
        else if constexpr (std::is_same<T, mnd::Negation>::value) {
            return isCorrect(*x.operand);
        }
        else if constexpr (std::is_same<T, mnd::Addition>::value ||
            std::is_same<T, mnd::Multiplication>::value ||
            std::is_same<T, mnd::Division>::value ||
            std::is_same<T, mnd::Pow>::value) {
            return isCorrect(*x.left) && isCorrect(*x.right);
        }
        return true;
    };
    isCorrect = [corrLambda](const mnd::Expression& x) {
        return std::visit(corrLambda, x);
    };
    bool allCorrect = isCorrect(expr);
    if (allCorrect) {
        return std::nullopt;
    }
    else {
        return unknownVariable;
    }
}


void mnd::IterationFormula::optimize(void)
{
    SimpleOptimizer so;
    so.visitExpr(this->expr);
}


bool mnd::IterationFormula::containsVariable(const std::string& name) const
{
    for (const auto& varname : variables) {
        if (varname == name)
            return true;
    }
    return false;
}


mnd::IterationFormula mnd::IterationFormula::clone(void) const
{
    std::function<std::unique_ptr<mnd::Expression>(const mnd::Expression&)> cloner;
    cloner = [&cloner](const mnd::Expression& e) {
        return std::make_unique<mnd::Expression>(std::visit([&cloner](const auto& x) -> mnd::Expression {
            using T = std::decay_t<decltype(x)>;
            if constexpr (std::is_same<T, mnd::Constant>::value) {
                return x;
            }
            else if constexpr (std::is_same<T, mnd::Variable>::value) {
                return mnd::Variable{ x.name };
            }
            else if constexpr (std::is_same<T, mnd::Negation>::value) {
                return mnd::Negation{ cloner(*x.operand) };
            }
            else if constexpr (std::is_same<T, mnd::Addition>::value) {
                return mnd::Addition{ cloner(*x.left), cloner(*x.right), x.subtraction };
            }
            else {
                return T{ cloner(*x.left), cloner(*x.right) };
            }
        }, e));
    };
    IterationFormula cl{ cloner(*expr), this->variables };
    return cl;
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

    bool expectingBinaryOperator;
public:
    Parser(const std::string& s) :
        in{ s },
        rit{ in.begin(), in.end(), tokenize },
	expectingBinaryOperator{ false }
    {}

    void parse(void)
    {
        std::string token;
        while (getToken(token)) {
            if (std::regex_match(token, num) || std::regex_match(token, floatNum)) {
                output.push_back(mnd::Constant{ std::atof(token.c_str()) });
                expectingBinaryOperator = true;
            }
            else if (std::regex_match(token, ident)) {
                output.push_back(mnd::Variable{ token });
                expectingBinaryOperator = true;
            }
            else if (token == "+" || token == "-") {
                if (expectingBinaryOperator) {
                    while (!operators.empty() && getTopPrecedence() > 3) {
                        popOperator();
                    }
                    operators.push(token[0]);
                    expectingBinaryOperator = false;
                }
                else { // unary op
                    if (token == "-")
                        operators.push('m');
                    else
                        throw ParseError("unary '+' is not allowed");
                }
            }
            else if (token == "*" || token == "/") {
                while (!operators.empty() && getTopPrecedence() >= 2) {
                    popOperator();
                }
                operators.push(token[0]);
                expectingBinaryOperator = false;
            }
            else if (token == "^") {
                while (!operators.empty() && getTopPrecedence() > 3) {
                    popOperator();
                }
                operators.push(token[0]);
                expectingBinaryOperator = false;
            }
            else if (token == "(") {
                operators.push(token[0]);
                expectingBinaryOperator = false;
            }
            else if (token == ")") {
                while (operators.top() != '(') {
                    popOperator();
                }
                operators.pop();
                expectingBinaryOperator = true;
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

        if (output.size() < 1) {
            throw ParseError("not enough operands for unary operator '-'");
        }


        mnd::Expression& unaryOperand = output.at(output.size() - 1);
        // handle unary minus separately
        if (top == 'm') {
            auto neg = mnd::Negation{ std::make_unique<mnd::Expression>(std::move(unaryOperand)) };
            output.pop_back();
            output.push_back(std::move(neg));
            return;
        }

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
        if (t == '+' || t == '-') // 'm' == unary minus
            return 1;
        else if (t == '*' || t == '/')
            return 2;
        else if (t == '^' || t == 'm')
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
                ss << "const[" << ex.re << "+" << ex.im << "i" << "]";
                return ss.str();
            }
            else if constexpr (std::is_same<T, mnd::Variable>::value) {
                return std::string("var[") + ex.name + "]";
            }
            else if constexpr (std::is_same<T, mnd::Negation>::value) {
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



