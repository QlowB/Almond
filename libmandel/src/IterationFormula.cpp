#include "IterationFormula.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/object/construct.hpp>
#include <iostream>
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

int simple(int a) {
    printf("simple: %d\n", a);
    return a;
}

int dopp(boost::fusion::vector<int, int> x) {
    int a = boost::fusion::at_c<0>(x);
    int b = boost::fusion::at_c<1>(x);
    printf("dopp: %d + %d\n", a, b);
    return a + b;
}

class ArithmeticGrammar4 : public qi::grammar<
    // the string iterator to parse: can also be const char* or templated.
    std::string::const_iterator,
    // return value of the grammar, written in function syntax!
    int(),
    // the _type_ of the skip parser
    ascii::space_type>
{
public:
    using Iterator = std::string::const_iterator;

    ArithmeticGrammar4() : ArithmeticGrammar4::base_type(start)
    {
        using qi::_val;
        using qi::_1;
        using qi::_2;

        start = qi::int_[_val = _1];

        /*start = (product >> '+' >> start)[&dopp]
            |
            //product[&simple];
            qi::int_[_val = _1];
        product = (factor >> '*' >> product) [&dopp]
            |
            factor[&simple];
        factor = //qi::int_[&simple]
            //|
            group[&simple];
        group = ('(' >> start >> ')')[&simple];*/
    }

    // as before, mirrors the template arguments of qi::grammar.
    qi::rule<Iterator, int(), ascii::space_type> start, group, product, factor;
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
        //mnd::Expression ret = mnd::Constant{ 1.0 };
        int ret = 0;
        //FormulaParser<std::string::const_iterator> grammar;
        ArithmeticGrammar4 c;
        auto begin = formula.begin();
        auto end = formula.end();
        bool r = true;
        /*PhraseParseOrDie (
            formula,
            c,
            ascii::space,
            ret
        );*/
        boost::spirit::qi::phrase_parse(
            begin, end, c, ascii::space, ret);

        printf("%d\n\n", (int) ret);
        return std::make_unique<Expression>(mnd::Constant{ 0.0 });
    }
}

