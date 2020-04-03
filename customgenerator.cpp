#include "customgenerator.h"
#include "ui_customgenerator.h"


#include <IterationCompiler.h>

CustomGenerator::CustomGenerator(mnd::MandelContext& mndCtxt, QWidget *parent) :
    QDialog{ parent },
    mndCtxt{ mndCtxt },
    ui{ new Ui::CustomGenerator }
{
    ui->setupUi(this);
}

CustomGenerator::~CustomGenerator()
{
    delete ui;
}


void CustomGenerator::compile()
{
    QString z0formula = this->ui->formula_z0->text();
    QString ziformula = this->ui->formula_zi->text();
    mnd::IterationFormula zi{ mnd::parse(ziformula.toStdString()), { "c", "z" } };
    mnd::IterationFormula z0{ mnd::parse(z0formula.toStdString()), { "c" } };

    mnd::GeneratorCollection cr;

    try {
        //std::cout << mnd::toString(*z0.expr) << std::endl;
        //std::cout << mnd::toString(*zi.expr) << std::endl;
        cr = mnd::compileFormula(mndCtxt, z0, zi);
    }
    catch(const mnd::ParseError& pe) {
        printf("Parse error: %s\n", pe.what());
        return;
    }
    catch(const std::string& e) {
        printf("error: %s\n", e.c_str());
        return;
    }
    /*catch(const char* e) {
        printf("error: %s\n", e);
        return;
    }*/
    fflush(stdout);
    fractalDefs.push_back(FractalDef {
                "name",
                z0formula,
                ziformula,
                std::move(cr)
    });
}


void CustomGenerator::on_compile_clicked()
{
    compile();
}


FractalDef* CustomGenerator::getLastCompiled(void)
{
    if (!fractalDefs.empty())
        return &fractalDefs[fractalDefs.size() - 1];
    else
        return nullptr;
}

void CustomGenerator::on_buttonBox_accepted()
{
    compile();
}
