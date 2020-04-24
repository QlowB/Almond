#ifndef MANDEL_NAIVEIRGENERATOR_H
#define MANDEL_NAIVEIRGENERATOR_H


namespace mnd
{
    template<typename T>
    class NaiveIRGenerator;
}



template<typename T>
class mnd::NaiveIRGenerator : public mnd::MandelGenerator
{
    const ir::Formula& form;
public:
    NaiveIRGenerator(const ir::Formula& irf, mnd::Precision prec);
    NaiveIRGenerator(NaiveIRGenerator&&) = default;

    virtual void generate(const MandelInfo& info, float* data);
    double calc(ir::Node* expr, double a, double b, double x, double y);
};


#endif // MANDEL_NAIVEIRGENERATOR_H
