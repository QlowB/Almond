#ifndef MANDEL_ITERATIONIR_H
#define MANDEL_ITERATIONIR_H

#include <string>
#include <vector>
#include <variant>
#include <any>

#include "IterationFormula.h"
#include "Real.h"
#include "Arena.h"

namespace mnd
{
    namespace ir
    {
        struct NodeBase;

        struct Constant;
        struct Variable;
        struct UnaryOperation;
        struct Negation;
        struct BinaryOperation;
        struct Addition;
        struct Subtraction;
        struct Multiplication;
        struct Division;
//        struct CPow;
//        struct RPow;
        struct Atan2;
        struct Pow;
        struct Cos;
        struct Sin;
        struct Exp;
        struct Ln;

        using Node = std::variant<
            Constant,
            Variable,
            Negation,
            Addition,
            Subtraction,
            Multiplication,
            Division,
//            CPow,
//            RPow,
            Atan2,
            Pow,
            Cos,
            Sin,
            Exp,
            Ln
        >;

        struct Formula
        {
            util::Arena<Node> nodeArena;
            Node* startA;
            Node* startB;

            Node* newA;
            Node* newB;

            std::string toString(void) const;

            void constantPropagation(void);
            void optimize(void);
            void clearNodeData(void);
        };
    }

    ir::Formula expand(const mnd::IterationFormula& z0, const mnd::IterationFormula& zi);
}


struct mnd::ir::NodeBase
{
    std::any nodeData;
};


struct mnd::ir::Constant : NodeBase
{
    mnd::Real value;
    inline Constant(const mnd::Real& val) : value{ val } {}
};


struct mnd::ir::Variable : NodeBase
{
    std::string name;
    inline Variable(const std::string name) : name{ name } {}
};


struct mnd::ir::UnaryOperation : NodeBase
{
    Node* value;
    inline UnaryOperation(Node* value) : value{ value } {}
};


struct mnd::ir::Negation : mnd::ir::UnaryOperation
{
    inline Negation(Node* value) : UnaryOperation{ value } {}
};


struct mnd::ir::BinaryOperation : NodeBase
{
    Node* left;
    Node* right;
    inline BinaryOperation(Node* left, Node* right) :
        left{ left }, right{ right } {}
};


struct mnd::ir::Addition : mnd::ir::BinaryOperation
{
    inline Addition(Node* left, Node* right) :
        BinaryOperation{ left, right } {}
};


struct mnd::ir::Subtraction : mnd::ir::BinaryOperation
{
    inline Subtraction(Node* left, Node* right) :
        BinaryOperation{ left, right } {}
};


struct mnd::ir::Multiplication : mnd::ir::BinaryOperation
{
    inline Multiplication(Node* left, Node* right) :
        BinaryOperation{ left, right } {}
};


struct mnd::ir::Division : mnd::ir::BinaryOperation
{
    inline Division(Node* left, Node* right) :
        BinaryOperation{ left, right } {}
};

/*
struct mnd::ir::CPow : mnd::ir::NodeBase
{
    Node* re;
    Node* im;
    Node* ere;
    Node* eim;
    inline CPow(Node* re, Node* im, Node* ere, Node* eim) :
        re{ re }, im{ im }, ere{ ere }, eim{ eim }
    {}
};


struct mnd::ir::RPow : mnd::ir::NodeBase
{
    Node* re;
    Node* im;
    Node* exponent;
    inline RPow(Node* re, Node* im, Node* exponent) :
        re{ re }, im{ im }, exponent{ exponent }
    {}
};*/


struct mnd::ir::Atan2 : mnd::ir::BinaryOperation
{
    inline Atan2(Node* left, Node* right) :
        BinaryOperation{ left, right } {}
};


struct mnd::ir::Pow : mnd::ir::BinaryOperation
{
    inline Pow(Node* left, Node* right) :
        BinaryOperation{ left, right } {}
};


struct mnd::ir::Cos : mnd::ir::UnaryOperation
{
    inline Cos(Node* value) : UnaryOperation{ value } {}
};


struct mnd::ir::Sin : mnd::ir::UnaryOperation
{
    inline Sin(Node* value) : UnaryOperation{ value } {}
};


struct mnd::ir::Exp : mnd::ir::UnaryOperation
{
    inline Exp(Node* value) : UnaryOperation{ value } {}
};


struct mnd::ir::Ln : mnd::ir::UnaryOperation
{
    inline Ln(Node* value) : UnaryOperation{ value } {}
};


#endif // MANDEL_ITERATIONIR_H
