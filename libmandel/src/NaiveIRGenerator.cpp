#include "NaiveIRGenerator.h"

#include "IterationIR.h"

using mnd::NaiveIRGenerator;

template<typename T>
NaiveIRGenerator<T>::NaiveIRGenerator(const mnd::ir::Formula& irf,
    mnd::Precision prec) :
    mnd::MandelGenerator{ prec },
    form{ irf }
{
}


template<typename U>
void NaiveIRGenerator<U>::generate(const mnd::MandelInfo& info, float* data)
{
    const MandelViewport& view = info.view;

    const bool parallel = true;
    using T = double;
    T viewx = mnd::convert<T>(view.x);
    T viewy = mnd::convert<T>(view.y);
    T wpp = mnd::convert<T>(view.width / info.bWidth);
    T hpp = mnd::convert<T>(view.height / info.bHeight);

    if constexpr (parallel)
        omp_set_num_threads(omp_get_num_procs());
    //#pragma omp parallel for schedule(static, 1) if (parallel)
    for (long j = 0; j < info.bHeight; j++) {
        T y = viewy + T(double(j)) * hpp;
        long i = 0;
        for (i; i < info.bWidth; i++) {
            T x = viewx + T(double(i)) * wpp;

            T a = calc(form.startA, x, y, 0, 0);
            T b = calc(form.startB, x, y, 0, 0);

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                double newA = calc(form.newA, a, b, x, y);
                double newB = calc(form.newB, a, b, x, y);
                a = newA;
                b = newB;
                if (a * a + b * b >= 16.0)
                    break;
            }
            data[i + j * info.bWidth] = float(k);
        }
    }
}


template<typename T>
struct EvalNode
{

};


template<typename T>
double NaiveIRGenerator<T>::calc(mnd::ir::Node* expr, double a, double b, double x, double y)
{
    struct DoubleVisitor
    {
        double a, b, x, y;
        double visitNode(ir::Node* n) {
            auto& nodeData = getNodeData(n);
            //if (std::get_a)
            return std::visit(*this, *n);
        }

        const std::any& getNodeData(ir::Node* n) {
            return std::visit([](auto& n) {
                return n.nodeData;
            }, *n);
        }

        double operator()(const ir::Constant& c) {
            return mnd::convert<double>(c.value);
        }

        double operator()(const ir::Variable& v) {
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

        double operator()(const ir::Negation& n) {
            return -visitNode(n.value);
        }

        double operator()(const ir::Addition& n) {
            return visitNode(n.left) + visitNode(n.right);
        }

        double operator()(const ir::Subtraction& n) {
            return visitNode(n.left) - visitNode(n.right);
        }

        double operator()(const ir::Multiplication& n) {
            return visitNode(n.left) * visitNode(n.right);
        }

        double operator()(const ir::Division& n) {
            return visitNode(n.left) / visitNode(n.right);
        }

        double operator()(const ir::Atan2& n) {
            return ::atan2(visitNode(n.left), visitNode(n.right));
        }

        double operator()(const ir::Pow& n) {
            return ::pow(visitNode(n.left), visitNode(n.right));
        }

        double operator()(const ir::Cos& n) {
            return ::cos(visitNode(n.value));
        }

        double operator()(const ir::Sin& n) {
            return ::sin(visitNode(n.value));
        }

        double operator()(const ir::Exp& n) {
            return ::exp(visitNode(n.value));
        }

        double operator()(const ir::Ln& n) {
            return ::log(visitNode(n.value));
        }
    };

    DoubleVisitor dv{ a, b, x, y };
    return dv.visitNode(expr);
}
