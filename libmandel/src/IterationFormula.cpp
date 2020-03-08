#include "IterationFormula.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/phoenix/object/construct.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace mnd;
using namespace boost::spirit;
namespace phx = boost::phoenix;

template <typename Parser, typename Skipper, typename ... Args>
void PhraseParseOrDie(
    const std::string& input, const Parser& p, const Skipper& s,
    Args&& ... args)
{
    std::string::const_iterator begin = input.begin(), end = input.end();
    boost::spirit::qi::phrase_parse(
        begin, end, p, s, std::forward<Args>(args) ...);
    if (begin != end) {
        std::cout << "Unparseable: "
            << std::quoted(std::string(begin, end)) << std::endl;
        throw std::runtime_error("Parse error");
    }
}


/*
void simple(int& a) {
    printf("simple: %d\n", a);
    a = a;
}

void dopp(boost::fusion::vector<int, int>& x) {
    int& a = boost::fusion::at_c<0>(x);
    int& b = boost::fusion::at_c<1>(x);
    printf("dopp: %d + %d\n", a, b);
    a = a + b;
}*/

class ArithmeticGrammar4 : public qi::grammar<
    // the string iterator to parse: can also be const char* or templated.
    std::string::const_iterator,
    // return value of the grammar, written in function syntax!
    mnd::Expression(),
    // the _type_ of the skip parser
    ascii::space_type>
{
    using Iterator = std::string::const_iterator;
    qi::rule<Iterator, mnd::Expression(), ascii::space_type> start, group, product, factor;
    qi::rule<Iterator, mnd::Constant(), ascii::space_type> constant;
public:

    ArithmeticGrammar4() : ArithmeticGrammar4::base_type(start)
    {
        using qi::_val;
        using qi::_1;
        using qi::_2;

        auto add = [] (auto&& left, auto&& right) {
            return mnd::Addition{
                std::make_unique<Expression>(std::move(left)),
                std::make_unique<Expression>(std::move(right)),
                false
            };
        };
        auto mul = [] (auto& left, auto& right) {
            return mnd::Multiplication {
                std::make_unique<Expression>(std::move(left)),
                std::make_unique<Expression>(std::move(right))
            };
        };
        auto pow = [] (auto& left, auto& right) {
            return mnd::Pow {
                std::make_unique<Expression>(std::move(left)),
                std::make_unique<Expression>(std::move(right))
            };
        };
        auto id = [] (auto& x) {
            //x = Constant{4};//std::move(*x);
        };

        constant %= qi::int_;
        //start = (constant >> '+' >> constant)[_val = phx::bind(add, _1, _2)];

        start = (constant >> '+' >> constant)[_val = _2]//[_val = phx::bind(add, _1, _2)]
            |
            product;
            //constant[_val = _1];
            //qi::int_[_val = _1];
        product = (constant >> '*' >> constant)[_val = phx::bind(mul, _1, _2)];
        /*product = (factor >> '*' >> product) [_val = phx::bind(mul, _1, _2)]
            |
            factor[_val = phx::bind(std::move, _1)];
        factor %= constant
            |
            group[_val = phx::bind(std::move, _1)];
        group %= ('(' >> start >> ')');*/
    }

    // as before, mirrors the template arguments of qi::grammar.
};

/*
struct calculator : qi::grammar<std::string::const_iterator, int(), ascii::space_type>
{
    using Iterator = std::string::const_iterator;
    qi::rule<Iterator, int(), ascii::space_type>
        expression, addition;
    calculator() : calculator::base_type(expression)
    {
        expression = (addition)[qi::_val = 5];
        addition = (qi::alpha >> *qi::alnum)[qi::_val = 2];
    }
};
*/

namespace mnd
{
    std::unique_ptr<Expression> parse(const std::string& formula)
    {
        mnd::Expression ret = mnd::Constant{ 2.3 };
        //int ret = 7;
        int ret2 = 3;
        //FormulaParser<std::string::const_iterator> grammar;
        ArithmeticGrammar4 c;
        auto begin = formula.begin();
        auto end = formula.end();
        /*PhraseParseOrDie (
            formula,
            c,
            ascii::space,
            ret
        );*/
        bool r = boost::spirit::qi::phrase_parse(
            begin, end, c, ascii::space, ret);

        if (begin != end)
            printf("error parsing");

        //printf("%d, %d\n\n", (int) ret, ret2);
        printf("%s\n\n", toString(ret).c_str());
        return std::make_unique<Expression>(mnd::Constant{ 0.0 });
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
                return std::string("-") + toString(*ex.operand);
            }
            else if constexpr (std::is_same<T, mnd::Addition>::value) {
                return toString(*ex.left) + std::string("+") + toString(*ex.right);
            }
            else if constexpr (std::is_same<T, mnd::Multiplication>::value) {
                return toString(*ex.left) + std::string("*") + toString(*ex.right);
            }
            else if constexpr (std::is_same<T, mnd::Division>::value) {
                return toString(*ex.left) + std::string("/") + toString(*ex.right);
            }
            else if constexpr (std::is_same<T, mnd::Pow>::value) {
                return toString(*ex.left) + std::string("^") + toString(*ex.right);
            }
            return "";
        }, expr);
    }
}



