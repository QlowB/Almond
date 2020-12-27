#include "IterationIR.h"


#include <utility>
#include <optional>

using namespace mnd;


namespace mnd
{
    using ir::Node;

    struct ConvertVisitor
    {
        using NodePair = std::pair<Node*, Node*>;
        util::Arena<Node>& arena;
        const mnd::IterationFormula& iterationFormula;

        Node* zero;
        Node* half;
        Node* one;

        ConvertVisitor(util::Arena<Node>& arena, const mnd::IterationFormula& iterationFormula) :
            arena{ arena },
            iterationFormula{ iterationFormula }
        {
            zero = arena.allocate(ir::Constant{ 0.0 });
            half = arena.allocate(ir::Constant{ 0.5 });
            one = arena.allocate(ir::Constant{ 1.0 });
        }

        NodePair operator() (const Constant& c)
        {
            Node* cnst = arena.allocate(ir::Constant{ c.re });
            Node* zero = arena.allocate(ir::Constant{ c.im });

            return { cnst, zero };
        }

        NodePair operator() (const Variable& v)
        {
            //printf("var %s\n", v.name.c_str()); fflush(stdout);
            if (v.name == "i") {
                return { zero, one };
            }
            else if (iterationFormula.containsVariable(v.name)) {
                Node* a = arena.allocate(ir::Variable{ v.name + "_re" });
                Node* b = arena.allocate(ir::Variable{ v.name + "_im" });
                return { a, b };
            }
            else
                throw "unknown variable";
        }

        NodePair operator() (const Negation& v)
        {
            auto [opa, opb] = std::visit(*this, *v.operand);

            Node* a = arena.allocate(ir::Negation{ opa });
            Node* b = arena.allocate(ir::Negation{ opb });

            return { a, b };
        }

        NodePair operator() (const Addition& add)
        {
            auto [lefta, leftb] = std::visit(*this, *add.left);
            auto [righta, rightb] = std::visit(*this, *add.right);

            if (add.subtraction) {
                Node* a = arena.allocate(ir::Subtraction{ lefta, righta });
                Node* b = arena.allocate(ir::Subtraction{ leftb, rightb });

                return { a, b };
            }
            else {
                Node* a = arena.allocate(ir::Addition{ lefta, righta });
                Node* b = arena.allocate(ir::Addition{ leftb, rightb });

                return { a, b };
            }
        }

        NodePair operator() (const Multiplication& mul)
        {
            auto [a, b] = std::visit(*this, *mul.left);
            auto [c, d] = std::visit(*this, *mul.right);

            return multiplication(a, b, c, d);
        }

        NodePair multiplication(Node* a, Node* b, Node* c, Node* d)
        {
            Node* ac = arena.allocate(ir::Multiplication{ a, c });
            Node* bd = arena.allocate(ir::Multiplication{ b, d });
            Node* ad = arena.allocate(ir::Multiplication{ a, d });
            Node* bc = arena.allocate(ir::Multiplication{ b, c });

            Node* newa = arena.allocate(ir::Subtraction{ ac, bd });
            Node* newb = arena.allocate(ir::Addition{ ad, bc });

            return { newa, newb };
        }

        NodePair sq(Node* a, Node* b)
        {
            Node* aa = arena.allocate(ir::Multiplication{ a, a });
            Node* bb = arena.allocate(ir::Multiplication{ b, b });
            Node* ab = arena.allocate(ir::Multiplication{ a, b });

            Node* newa = arena.allocate(ir::Subtraction{ aa, bb });
            Node* newb = arena.allocate(ir::Addition{ ab, ab });

            return { newa, newb };
        }

        NodePair operator() (const Division& div)
        {
            auto [a, b] = std::visit(*this, *div.left);
            auto [c, d] = std::visit(*this, *div.right);

            return division(a, b, c, d);
        }

        NodePair division(Node* a, Node* b, Node* c, Node* d)
        {
            Node* ac = arena.allocate(ir::Multiplication{ a, c });
            Node* bd = arena.allocate(ir::Multiplication{ b, d });
            Node* bc = arena.allocate(ir::Multiplication{ b, c });
            Node* ad = arena.allocate(ir::Multiplication{ a, d });

            Node* cc = arena.allocate(ir::Multiplication{ c, c });
            Node* dd = arena.allocate(ir::Multiplication{ d, d });

            Node* ac_bd = arena.allocate(ir::Addition{ ac, bd });
            Node* bc_ad = arena.allocate(ir::Subtraction{ bc, ad });

            Node* den = arena.allocate(ir::Addition{ cc, dd });
            Node* factor = arena.allocate(ir::Division{ one, den });

            Node* re = arena.allocate(ir::Multiplication{ factor, ac_bd });
            Node* im = arena.allocate(ir::Multiplication{ factor, bc_ad });

            return { re, im };
        }

        NodePair oneOver(Node* a, Node* b)
        {
            Node* cc = arena.allocate(ir::Multiplication{ a, a });
            Node* dd = arena.allocate(ir::Multiplication{ b, b });

            Node* den = arena.allocate(ir::Addition{ cc, dd });
            Node* factor = arena.allocate(ir::Division{ one, den });

            Node* re = arena.allocate(ir::Multiplication{ factor, a });
            Node* im = arena.allocate(ir::Negation{ arena.allocate(ir::Multiplication{ factor, b }) });

            return { re, im };
        }

        NodePair operator() (const Pow& p)
        {
            auto [a, b] = std::visit(*this, *p.left);
            auto [c, d] = std::visit(*this, *p.right);

            if (p.integerExponent) {
                if (auto* ex = std::get_if<ir::Constant>(c)) {
                    if (int(ex->value) <= 20) {
                        return intPow({ a, b }, int(ex->value));
                    }
                    else {
                        return realPow({ a, b }, c);
                    }
                }
            }
            if (p.realExponent) {
                return realPow({ a, b }, c);
            }

            auto arg = arena.allocate(ir::Atan2{ b, a });
            auto aa = arena.allocate(ir::Multiplication{ a, a });
            auto bb = arena.allocate(ir::Multiplication{ b, b });
            auto absSq = arena.allocate(ir::Addition{ aa, bb });

            auto halfc = arena.allocate(ir::Multiplication{ c, half });
            auto darg = arena.allocate(ir::Multiplication{ d, arg });
            auto minusdarg = arena.allocate(ir::Negation{ darg });

            auto abspowc = arena.allocate(ir::Pow{ absSq, halfc });
            auto expdarg = arena.allocate(ir::Exp{ minusdarg });

            auto newAbs = arena.allocate(ir::Multiplication{ abspowc, expdarg });
            auto carg = arena.allocate(ir::Multiplication{ arg, c });

            auto halfd = arena.allocate(ir::Multiplication{ d, half });
                //absSq = arena.allocate(ir::Addition{ absSq, half });
            auto lnabsSq = arena.allocate(ir::Ln{ absSq });
            auto halfdlnabsSq = arena.allocate(ir::Multiplication{ halfd, lnabsSq });
            auto newArg = arena.allocate(ir::Addition{ halfdlnabsSq, carg });

            auto cosArg = arena.allocate(ir::Cos{ newArg });
            auto sinArg = arena.allocate(ir::Sin{ newArg });

            auto newA = arena.allocate(ir::Multiplication{ cosArg, newAbs });
            auto newB = arena.allocate(ir::Multiplication{ sinArg, newAbs });

            return { newA, newB };
        }

        NodePair intPow(NodePair val, int exponent) {
            auto [a, b] = val;

            if (exponent < 0) {
                auto [inva, invb] = intPow(val, -exponent);
                return oneOver(inva, invb);
            }

            if (exponent == 0)
                return { one, zero };
            else if (exponent == 1)
                return val;
            else if (exponent == 2)
                return sq(a, b);
            else {
                bool isEven = (exponent % 2) == 0;
                if (isEven) {
                    NodePair square = sq(a, b);
                    return intPow(square, exponent / 2);
                }
                else {
                    int expm1 = exponent - 1;
                    NodePair square = sq(a, b);
                    auto[pa, pb] = intPow(square, expm1 / 2);
                    return multiplication(pa, pb, a, b);
                }
            }
        }

        NodePair realPow(NodePair val, Node* exponent) {
            auto [a, b] = val;

            auto arg = arena.allocate(ir::Atan2{ b, a });
            auto aa = arena.allocate(ir::Multiplication{ a, a });
            auto bb = arena.allocate(ir::Multiplication{ b, b });
            auto absSq = arena.allocate(ir::Addition{ aa, bb });

            auto halfc = arena.allocate(ir::Multiplication{ exponent, half });

            auto newAbs = arena.allocate(ir::Pow{ absSq, halfc });

            auto newArg = arena.allocate(ir::Multiplication{ arg, exponent });

            auto cosArg = arena.allocate(ir::Cos{ newArg });
            auto sinArg = arena.allocate(ir::Sin{ newArg });

            auto newA = arena.allocate(ir::Multiplication{ cosArg, newAbs });
            auto newB = arena.allocate(ir::Multiplication{ sinArg, newAbs });

            return { newA, newB };
        }
    };

    ir::Formula expand(const mnd::IterationFormula& z0, const mnd::IterationFormula& zi)
    {
        ir::Formula formula;
        ConvertVisitor cv0{ formula.nodeArena, z0 };
        ConvertVisitor cvi{ formula.nodeArena, zi };
        std::tie(formula.startA, formula.startB) = std::visit(cv0, *z0.expr);
        std::tie(formula.newA, formula.newB) = std::visit(cvi, *zi.expr);
        return formula;
    }    
}


using namespace std::string_literals;
struct ToStringVisitor
{
    // return string and precedence
    using Ret = std::pair<std::string, int>;

    Ret operator()(const ir::Constant& c) {
        return { mnd::toString(c.value), 0 };
    }

    Ret operator()(const ir::Variable& v) {
        return { v.name, 0 };
    }

    Ret operator()(const ir::Negation& n) {
        auto [str, prec] = std::visit(*this, *n.value);
        if (prec > 0)
            return { "-("s + str + ")", 2 };
        else
            return { "-"s + str, 2 };
    }

    Ret operator()(const ir::Addition& n) {
        auto [strl, precl] = std::visit(*this, *n.left);
        auto [strr, precr] = std::visit(*this, *n.right);
        std::string ret;
        if (precl > 4)
            ret += strl + " + ";
        else
            ret += "(" + strl + ") + ";
        if (precr > 4)
            ret += strr;
        else
            ret += "(" + strr + ")";
        return { ret, 4 };
    }

    Ret operator()(const ir::Subtraction& n) {
        auto [strl, precl] = std::visit(*this, *n.left);
        auto [strr, precr] = std::visit(*this, *n.right);
        std::string ret;
        if (precl > 4)
            ret += strl + " - ";
        else
            ret += "(" + strl + ") - ";
        if (precr >= 4)
            ret += strr;
        else
            ret += "(" + strr + ")";
        return { ret, 4 };
    }

    Ret operator()(const ir::Multiplication& n) {
        auto [strl, precl] = std::visit(*this, *n.left);
        auto [strr, precr] = std::visit(*this, *n.right);
        std::string ret;
        if (precl > 3)
            ret += strl + " * ";
        else
            ret += "(" + strl + ") * ";
        if (precr > 3)
            ret += strr;
        else
            ret += "(" + strr + ")";
        return { ret, 3 };
    }

    Ret operator()(const ir::Division& n) {
        auto [strl, precl] = std::visit(*this, *n.left);
        auto [strr, precr] = std::visit(*this, *n.right);
        std::string ret;
        if (precl > 3)
            ret += strl + " / ";
        else
            ret += "(" + strl + ") / ";
        if (precr >= 3)
            ret += strr;
        else
            ret += "(" + strr + ")";
        return { ret, 3 };
    }

    Ret operator()(const ir::Atan2& n) {
        return { "atan2(" + std::visit(*this, *n.left).first + ", " + std::visit(*this, *n.right).first + ")", 1 };
    }

    Ret operator()(const ir::Pow& n) {
        auto [strl, precl] = std::visit(*this, *n.left);
        auto [strr, precr] = std::visit(*this, *n.right);
        std::string ret;
        if (precl >= 2)
            ret += strl + " ^ ";
        else
            ret += "(" + strl + ") ^ ";
        if (precr > 2)
            ret += strr;
        else
            ret += "(" + strr + ")";
        return { ret, 2 };
    }

    Ret operator()(const ir::Cos& n) {
        return { "cos(" + std::visit(*this, *n.value).first + ")", 1 };
    }

    Ret operator()(const ir::Sin& n) {
        return { "sin(" + std::visit(*this, *n.value).first + ")", 1 };
    }

    Ret operator()(const ir::Exp& n) {
        return { "exp(" + std::visit(*this, *n.value).first + ")", 1 };
    }

    Ret operator()(const ir::Ln& n) {
        return { "ln(" + std::visit(*this, *n.value).first + ")", 1 };
    }
};

std::string mnd::ir::Formula::toString(void) const
{
    return std::string("a = ") + std::visit(ToStringVisitor{}, *this->newA).first + 
        "\nb = " + std::visit(ToStringVisitor{}, *this->newB).first +
        "\nx = " + std::visit(ToStringVisitor{}, *this->startA).first +
        "\ny = " + std::visit(ToStringVisitor{}, *this->startB).first;
}

struct ConstantPropagator
{
    mnd::ir::Formula& formula;
    mnd::util::Arena<Node>& arena;

    using MaybeNode = std::optional<Node>;

    ConstantPropagator(mnd::ir::Formula& formula) :
        formula{ formula },
        arena{ formula.nodeArena }
    {
    }

    void propagateConstants(void) {
        visitNode(formula.newA);
        visitNode(formula.newB);
        visitNode(formula.newA);
        visitNode(formula.newB);
    }

    bool hasBeenVisited(Node* n) {
        return std::visit([] (auto& x) {
            if (auto* b = std::any_cast<bool>(&x.nodeData))
                return *b;
            else
                return false;
        }, *n);
    }

    void visitNode(Node* n) {
        if (!hasBeenVisited(n)) {
            MaybeNode mbn = std::visit(*this, *n);
            if (mbn.has_value()) {
                *n = std::move(mbn.value());
            }
            std::visit([] (auto& x) { x.nodeData = true; }, *n);
        }
    }

    ir::Constant* getIfConstant(Node* n) {
        return std::get_if<ir::Constant>(n);
    }

    MaybeNode operator()(ir::Constant& x) {
        return std::nullopt;
    }
    MaybeNode operator()(ir::Variable& x) {
        return std::nullopt;
    }

    MaybeNode operator()(ir::Negation& n) {
        visitNode(n.value);
        if (auto* c = getIfConstant(n.value)) {
            return ir::Constant{ -c->value };
        }
        if (auto* neg = std::get_if<ir::Negation>(n.value)) {
            return *neg->value;
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Addition& n) {
        visitNode(n.left);
        visitNode(n.right);
        auto* ca = getIfConstant(n.left);
        auto* cb = getIfConstant(n.right);
        if (ca && cb) {
            return ir::Constant{ ca->value + cb->value };
        }
        else if (ca && ca->value == 0) {
            return *n.right;
        }
        else if (cb && cb->value == 0) {
            return *n.left;
        }
        else if (cb) {
            // move constants to the left
            std::swap(n.left, n.right);
        }
        else if (auto* nright = std::get_if<ir::Negation>(n.right)) {
            return ir::Subtraction{ n.left, nright->value };
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Subtraction& n) {
        visitNode(n.left);
        visitNode(n.right);
        auto* ca = getIfConstant(n.left);
        auto* cb = getIfConstant(n.right);
        if (ca && cb) {
            return ir::Constant{ ca->value - cb->value };
        }
        else if (ca && ca->value == 0) {
            return ir::Negation{ n.right };
        }
        else if (cb && cb->value == 0) {
            return *n.left;
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Multiplication& n) {
        visitNode(n.left);
        visitNode(n.right);
        printf("simpifying mul: %s * %s\n", std::visit(ToStringVisitor{}, *n.left).first.c_str(), std::visit(ToStringVisitor{}, *n.right).first.c_str());
        auto* ca = getIfConstant(n.left);
        auto* cb = getIfConstant(n.right);
        if (ca && cb) {
            return ir::Constant{ ca->value * cb->value };
        }
        if (ca && ca->value == 0) {
            return ir::Constant{ 0 };
        }
        if (cb && cb->value == 0) {
            return ir::Constant{ 0 };
        }
        if (ca && ca->value == 1) {
            return *n.right;
        }
        if (cb && cb->value == 1) {
            return *n.left;
        }
        if (ca) {
            auto* rightMul = std::get_if<ir::Multiplication>(n.right);
            if (rightMul) {
                auto* clr = getIfConstant(rightMul->left);
                if (clr) {
                    printf("left %s, right %s\n", mnd::toString(ca->value).c_str(), mnd::toString(clr->value).c_str());
                    //ca->value *= clr->value;
                    n.right = rightMul->right;
                    auto mul = ir::Multiplication{ arena.allocate(ir::Constant{ ca->value * clr->value }), rightMul->right };//
                    auto maybeBetter = this->operator()(mul);
                    return maybeBetter.value_or(mul);
                }
            }
        }
        if (cb) {
            // move constants to the left
            std::swap(n.left, n.right);
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Division& n) {
        visitNode(n.left);
        visitNode(n.right);
        auto* ca = getIfConstant(n.left);
        auto* cb = getIfConstant(n.right);
        if (ca && cb) {
            return ir::Constant{ ca->value / cb->value };
        }
        else if (ca && ca->value == 0) {
            return ir::Constant{ 0 };
        }
        else if (cb && cb->value == 1) {
            return *n.left;
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Atan2& n) {
        visitNode(n.left);
        visitNode(n.right);
        auto* ca = getIfConstant(n.left);
        auto* cb = getIfConstant(n.right);
        if (ca && cb) {
            return ir::Constant{ mnd::atan2(ca->value, cb->value) };
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Pow& n) {
        visitNode(n.left);
        visitNode(n.right);
        auto* ca = getIfConstant(n.left);
        auto* cb = getIfConstant(n.right);
        if (ca && cb) {
            return ir::Constant{ mnd::pow(ca->value, cb->value) };
        }
        else if (cb && cb->value == 1) {
            return *n.left;
        }
        else if (cb && cb->value == 1) {
            return ir::Constant{ 1 };
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Cos& n) {
        visitNode(n.value);
        auto* ca = getIfConstant(n.value);
        if (ca) {
            return ir::Constant{ mnd::cos(ca->value) };
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Sin& n) {
        visitNode(n.value);
        auto* ca = getIfConstant(n.value);
        if (ca) {
            return ir::Constant{ mnd::sin(ca->value) };
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Exp& n) {
        visitNode(n.value);
        auto* ca = getIfConstant(n.value);
        if (ca) {
            return ir::Constant{ mnd::exp(ca->value) };
        }
        return std::nullopt;
    }

    MaybeNode operator()(ir::Ln& n) {
        visitNode(n.value);
        auto* ca = getIfConstant(n.value);
        if (ca) {
            return ir::Constant{ mnd::log(ca->value) };
        }
        return std::nullopt;
    }
};

void mnd::ir::Formula::constantPropagation(void)
{
    ConstantPropagator cp { *this };
    cp.propagateConstants();
}


void mnd::ir::Formula::optimize(void)
{
    constantPropagation();
}


void mnd::ir::Formula::clearNodeData(void)
{
    nodeArena.forAll([] (Node& n) {
        std::visit([] (auto& x) {
                x.nodeData.reset();
        }, n);
    });
}


