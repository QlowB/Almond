#include "IterationIR.h"

#include <utility>

using namespace mnd;


namespace mnd
{
    using ir::Node;

    struct ConvertVisitor
    {
        using NodePair = std::pair<Node*, Node*>;
        util::Arena<Node>& arena;

        ConvertVisitor(util::Arena<Node>& arena) :
            arena{ arena }
        {
        }

        NodePair operator() (const Constant& c)
        {
            Node* cnst = arena.allocate(ir::Constant{ c.value });
            Node* zero = arena.allocate(ir::Constant{ 0.0 });

            return { cnst, zero };
        }

        NodePair operator() (const Variable& v)
        {
            //printf("var %s\n", v.name.c_str()); fflush(stdout);
            if (v.name == "z") {
                Node* a = arena.allocate(ir::Variable{ "a" });
                Node* b = arena.allocate(ir::Variable{ "b" });

                return { a, b };
            }
            else if (v.name == "c") {
                Node* x = arena.allocate(ir::Variable{ "x" });
                Node* y = arena.allocate(ir::Variable{ "y" });

                return { x, y };
            }
            else if (v.name == "i") {
                Node* x = arena.allocate(ir::Constant{ 0.0 });
                Node* y = arena.allocate(ir::Constant{ 1.0 });

                return { x, y };
            }
            else
                throw "unknown variable";
        }

        NodePair operator() (const UnaryOperation& v)
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

            Node* ac = arena.allocate(ir::Multiplication{ a, c });
            Node* bd = arena.allocate(ir::Multiplication{ b, d });
            Node* ad = arena.allocate(ir::Multiplication{ a, d });
            Node* bc = arena.allocate(ir::Multiplication{ b, c });

            Node* newa = arena.allocate(ir::Subtraction{ ac, bd });
            Node* newb = arena.allocate(ir::Addition{ ad, bc });

            return { newa, newb };
        }

        NodePair operator() (const Division& mul)
        {
            // TODO implement
            throw "unimplemented";
            return { nullptr, nullptr };
        }

        NodePair operator() (const Pow& p)
        {
            auto [a, b] = std::visit(*this, *p.left);
            auto [c, d] = std::visit(*this, *p.right);

            auto half = arena.allocate(ir::Constant{ 0.5 });

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
    };

    ir::Formula expand(const mnd::IterationFormula& fmla)
    {
        ir::Formula formula;
        ConvertVisitor cv{ formula.nodeArena };
        std::tie(formula.newA, formula.newB) = std::visit(cv, *fmla.expr);
        return formula;
    }
}


std::string mnd::ir::Formula::toString(void) const
{
    struct ToStringVisitor
    {
        std::string operator()(const ir::Constant& c) {
            return mnd::toString(c.value);
        }

        std::string operator()(const ir::Variable& v) {
            return v.name;
        }

        std::string operator()(const ir::Negation& n) {
            return "-(" + std::visit(*this, *n.value) + ")";
        }

        std::string operator()(const ir::Addition& n) {
            return "(" + std::visit(*this, *n.left) + ") + (" + std::visit(*this, *n.right) + ")";
        }

        std::string operator()(const ir::Subtraction& n) {
            return "(" + std::visit(*this, *n.left) + ") - (" + std::visit(*this, *n.right) + ")";
        }

        std::string operator()(const ir::Multiplication& n) {
            return "(" + std::visit(*this, *n.left) + ") * (" + std::visit(*this, *n.right) + ")";
        }

        std::string operator()(const ir::Atan2& n) {
            return "atan2(" + std::visit(*this, *n.left) + ", " + std::visit(*this, *n.right) + ")";
        }

        std::string operator()(const ir::Pow& n) {
            return std::visit(*this, *n.left) + " ^ " + std::visit(*this, *n.right);
        }

        std::string operator()(const ir::Cos& n) {
            return "cos(" + std::visit(*this, *n.value) + ")";
        }

        std::string operator()(const ir::Sin& n) {
            return "sin(" + std::visit(*this, *n.value) + ")";
        }

        std::string operator()(const ir::Exp& n) {
            return "exp(" + std::visit(*this, *n.value) + ")";
        }

        std::string operator()(const ir::Ln& n) {
            return "ln(" + std::visit(*this, *n.value) + ")";
        }
    };

    return std::string("a = ") + std::visit(ToStringVisitor{}, *this->newA) + 
        "\nb = " + std::visit(ToStringVisitor{}, *this->newB);
}
