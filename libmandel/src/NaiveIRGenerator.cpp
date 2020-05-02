#include "NaiveIRGenerator.h"
#include <omp.h>


using mnd::NaiveIRGenerator;


namespace mnd
{
    template class NaiveIRGenerator<float>;
    template class NaiveIRGenerator<double>;
    template class NaiveIRGenerator<mnd::DoubleDouble>;
    template class NaiveIRGenerator<mnd::QuadDouble>;
}

namespace eval {
    struct Load;
    struct Store;
    struct Add;
    struct Sub;
    struct Mul;
    struct Div;
    struct Neg;    
    struct Atan;
    struct Pow;
    struct Cos;
    struct Sin;
    struct Exp;
    struct Ln;

    using EvalNode = std::variant<
        Load,
        Store,
        Add,
        Sub,
        Mul,
        Div,
        Neg,
        Atan,
        Pow,
        Cos,
        Sin,
        Exp,
        Ln
    >;

    struct Load { int index; };
    struct Store { int index; };

    struct BinaryOperation
    {
        std::unique_ptr<EvalNode> a;
        std::unique_ptr<EvalNode> b;
    };

    struct UnaryOperation
    {
        std::unique_ptr<EvalNode> a;
    };

    struct Add : BinaryOperation {};
    struct Sub : BinaryOperation {};
    struct Mul : BinaryOperation {};
    struct Div : BinaryOperation {};

    struct Neg : UnaryOperation {};


    struct Atan : BinaryOperation {};
    struct Pow : BinaryOperation {};
    struct Cos : UnaryOperation {};
    struct Sin : UnaryOperation {};
    struct Exp : UnaryOperation {};
    struct Ln : UnaryOperation {};
}


template<typename T>
NaiveIRGenerator<T>::NaiveIRGenerator(const mnd::ir::Formula& irf,
    mnd::Precision prec) :
    mnd::MandelGenerator{ prec },
    form{ irf }
{
}


template<typename T>
struct EvalRes
{
    size_t incVal;
    T result;
};


using namespace mnd;
template<typename T>
struct TVisitor
{
    T a, b, x, y;
    size_t incrementValue;

    T visitNode(ir::Node* n) {
        EvalRes<T>* nodeData = getNodeData(n);
        if (nodeData) {
            if (nodeData->incVal == incrementValue)
                return nodeData->result;
        }
        T res = std::visit(*this, *n);
        if (nodeData) {
            nodeData->incVal = incrementValue;
            nodeData->result = res;
        }
        return res;
    }

    EvalRes<T>* getNodeData(ir::Node* n) {
        assert(n != nullptr);
        std::any& x = std::visit([](auto& n) -> std::any& {
            return n.nodeData;
        }, *n);
        if (auto* v = std::any_cast<EvalRes<T>>(&x))
            return v;
        else
            return nullptr;
    }

    T operator()(const ir::Constant& c) {
        return mnd::convert<double>(c.value);
    }

    T operator()(const ir::Variable& v) {
        if (v.name == "z_re")
            return a;
        else if (v.name == "z_im")
            return b;
        else if (v.name == "c_re")
            return x;
        else if (v.name == "c_im")
            return y;
        else
            return 0.0;
    }

    T operator()(const ir::Negation& n) {
        return -visitNode(n.value);
    }

    T operator()(const ir::Addition& n) {
        return visitNode(n.left) + visitNode(n.right);
    }

    T operator()(const ir::Subtraction& n) {
        return visitNode(n.left) - visitNode(n.right);
    }

    T operator()(const ir::Multiplication& n) {
        return visitNode(n.left) * visitNode(n.right);
    }

    T operator()(const ir::Division& n) {
        return visitNode(n.left) / visitNode(n.right);
    }

    T operator()(const ir::Atan2& n) {
        return ::atan2(visitNode(n.left), visitNode(n.right));
    }

    T operator()(const ir::Pow& n) {
        return ::pow(visitNode(n.left), visitNode(n.right));
    }

    T operator()(const ir::Cos& n) {
        return ::cos(visitNode(n.value));
    }

    T operator()(const ir::Sin& n) {
        return ::sin(visitNode(n.value));
    }

    T operator()(const ir::Exp& n) {
        return ::exp(visitNode(n.value));
    }

    T operator()(const ir::Ln& n) {
        return ::log(visitNode(n.value));
    }
};


template<typename T>
void NaiveIRGenerator<T>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    const bool parallel = true;
    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);

#if defined(_OPENMP)
    if constexpr (parallel)
        omp_set_num_threads(omp_get_num_procs());
#   pragma omp parallel for schedule(static, 1) if (parallel)
#endif
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        long i = 0;
        for (i; i < info.bWidth; i++) {
            T x = viewx + T(double(i)) * wpp;

            TVisitor<T> beforeVisitor{ 0, 0, x, y, 0 };

            T a = beforeVisitor.visitNode(form.startA);
            T b = beforeVisitor.visitNode(form.startB);

            TVisitor<T> visitor{ a, b, x, y, 0 };
            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                T newA = visitor.visitNode(form.newA);
                T newB = visitor.visitNode(form.newB);
                a = newA;
                b = newB;
                if (a * a + b * b >= 16.0)
                    break;
                visitor.incrementValue++;
            }
            data[i + j * info.bWidth] = float(k);
        }
    }
}


