#include "NaiveIRGenerator.h"
#include <omp.h>


using mnd::NaiveIRGenerator;

template class mnd::NaiveIRGenerator<float>;
template class mnd::NaiveIRGenerator<double>;
template class mnd::NaiveIRGenerator<mnd::DoubleDouble>;
template class mnd::NaiveIRGenerator<mnd::QuadDouble>;

namespace mnd::eval
{

    using namespace mnd;
    using namespace mnd::ir;
    template<typename T>
    struct ToEvalVisitor
    {
        EvalStruct<T>& es;

        std::unique_ptr<EvalNode> visit(ir::Node* f)
        {
            std::any& nodeData = getNodeData(f);
            if (EvalNode** en = std::any_cast<EvalNode*>(&nodeData)) {
                
                size_t tmpStore;
                if (Store* s = std::get_if<Store>(*en)) {
                    tmpStore = s->index;
                }
                else {
                    tmpStore = createTemp();
                    EvalNode store = Store{ tmpStore, std::make_unique<EvalNode>(std::move(**en)) };
                    **en = std::move(store);
                }
                auto l = std::make_unique<EvalNode>(Load{ tmpStore });
                setNodeData(f, l.get());
                return l;
            }
            EvalNode n = std::visit(*this, *f);
            auto r = std::make_unique<EvalNode>(std::move(n));
            setNodeData(f, r.get());
            return r;
        }

        std::any& getNodeData(ir::Node* n)
        {
            return std::visit([](auto& x) -> std::any& { return x.nodeData; }, *n);
        }

        void setNodeData(ir::Node* n, EvalNode* en)
        {
            std::visit([en](auto& x) { x.nodeData = en; }, *n);
        }

        size_t createTemp(void)
        {
            es.temporaries.push_back(0);
            es.variables.push_back(&es.temporaries[es.temporaries.size() - 1]);
            return es.variables.size() - 1;
        }

        size_t createConstant(mnd::Real& value)
        {
            es.temporaries.push_back(mnd::convert<T>(value));
            es.variables.push_back(&es.temporaries[es.temporaries.size() - 1]);
            return es.variables.size() - 1;
        }

        size_t createVariable(std::string& value)
        {
            es.variables.push_back(nullptr);
            es.variableNames.emplace(value, es.variables.size() - 1);
            return es.variables.size() - 1;
        }

        EvalNode operator()(ir::Constant& x) {
            return Load{ createConstant(x.value) };
        }
        EvalNode operator()(ir::Variable& x) {
            return Load{ createVariable(x.name) };
        }
        EvalNode operator()(ir::Addition& x) {
            auto left = visit(x.left);
            return Add{ std::move(left), visit(x.right) };
        }
        EvalNode operator()(ir::Subtraction& x) {
            auto left = visit(x.left);
            return Sub{ std::move(left), visit(x.right) };
        }
        EvalNode operator()(ir::Multiplication& x) {
            auto left = visit(x.left);
            return Mul{ std::move(left), visit(x.right) };
        }
        EvalNode operator()(ir::Division& x) {
            auto left = visit(x.left);
            return Div{ std::move(left), visit(x.right) };
        }
        EvalNode operator()(ir::Negation& x) {
            return Neg{ visit(x.value) };
        }
        EvalNode operator()(ir::Atan2& x) {
            auto left = visit(x.left);
            return Atan2{ std::move(left), visit(x.right) };
        }
        EvalNode operator()(ir::Pow& x) {
            auto left = visit(x.left);
            return Pow{ std::move(left), visit(x.right) };
        }
        EvalNode operator()(ir::Cos& x) {
            return Cos{ visit(x.value) };
        }
        EvalNode operator()(ir::Sin& x) {
            return Sin{ visit(x.value) };
        }
        EvalNode operator()(ir::Exp& x) {
            return Exp{ visit(x.value) };
        }
        EvalNode operator()(ir::Ln& x) {
            return Ln{ visit(x.value) };
        }
    };



    template<typename T>
    struct EvalVisitor
    {
        mnd::eval::EvalStruct<T>& es;

        T visit(const EvalNode& en) {
            return std::visit(*this, en);
        }

        T operator()(const Load& x) {
            return *es.variables[x.index];
        }

        T operator()(const Store& x) {
            T r = visit(*x.v);
            *es.variables[x.index] = r;
            return r;
        }

        T operator()(const Add& x) {
            return visit(*x.a) + visit(*x.b);
        }

        T operator()(const Sub& x) {
            return visit(*x.a) - visit(*x.b);
        }

        T operator()(const Mul& x) {
            return visit(*x.a) * visit(*x.b);
        }

        T operator()(const Div& x) {
            return visit(*x.a) / visit(*x.b);
        }

        T operator()(const Neg& x) {
            return -visit(*x.a);
        }

        T operator()(const Atan2& x) {
            return mnd::atan2(visit(*x.a), visit(*x.b));
        }

        T operator()(const Pow& x) {
            return mnd::pow(visit(*x.a), visit(*x.b));
        }

        T operator()(const Cos& x) {
            return mnd::cos(visit(*x.a));
        }

        T operator()(const Sin& x) {
            return mnd::sin(visit(*x.a));
        }

        T operator()(const Exp& x) {
            return mnd::exp(visit(*x.a));
        }

        T operator()(const Ln& x) {
            return mnd::log(visit(*x.a));
        }
    };
}


template<typename T>
NaiveIRGenerator<T>::NaiveIRGenerator(const mnd::ir::Formula& irf,
    mnd::Precision prec) :
    mnd::MandelGenerator{ prec },
    form{ irf }
{
    eval::ToEvalVisitor<T> tev{ es };
    newz_re = tev.visit(irf.newA);
    newz_im = tev.visit(irf.newB);
    start_re = tev.visit(irf.startA);
    start_im = tev.visit(irf.startB);
}


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
            T a;
            T b;
            es.prepare(&a, &b, &x, &y);

            eval::EvalVisitor<T> visitor{ es };

            a = visitor.visit(*start_re);
            b = visitor.visit(*start_im);

            int k = 0;
            for (k = 0; k < info.maxIter; k++) {
                T newA = visitor.visit(*newz_re);
                T newB = visitor.visit(*newz_im);
                a = newA;
                b = newB;
                if (a * a + b * b >= 16.0)
                    break;
            }
            data[i + j * info.bWidth] = float(k);
        }
    }
}


