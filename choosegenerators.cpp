#include "choosegenerators.h"
#include "ui_choosegenerators.h"

#include "Hardware.h"

#include <QComboBox>
#include <QRegExp>
#include <QRegExpValidator>

ChooseGenerators::ChooseGenerators(mnd::MandelContext& mndCtxt, QWidget *parent) :
    QDialog{ parent },
    ui{ std::make_unique<Ui::ChooseGenerators>() },
    mndCtxt{ mndCtxt },
    tableContent{}
{
    ui->setupUi(this);

    QRegExp floatingpoint{ "^[-+]?(\\d*\\.?\\d+|\\d+\\.?\\d*)([eE][-+]\\d+)?$" };
    floatValidator = std::make_unique<QRegExpValidator>(floatingpoint, this);

    generators = std::map<QString, mnd::Generator*> {
        { "float", mndCtxt.getCpuGenerator(mnd::GeneratorType::FLOAT) },
        { "double", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE) },
        { "double double", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE_DOUBLE) },
        { "quad double", mndCtxt.getCpuGenerator(mnd::GeneratorType::QUAD_DOUBLE) },
        { "float256", mndCtxt.getCpuGenerator(mnd::GeneratorType::FLOAT256) },
    };

    if (mndCtxt.getCpuInfo().hasSse2()) {
        generators.insert({ "float SSE2", mndCtxt.getCpuGenerator(mnd::GeneratorType::FLOAT_SSE2) });
        generators.insert({ "double SSE2", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE_SSE2) });
    }
    if (mndCtxt.getCpuInfo().hasAvx()) {
        generators.insert({ "float AVX", mndCtxt.getCpuGenerator(mnd::GeneratorType::FLOAT_AVX) });
        generators.insert({ "double AVX", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE_AVX) });
    }
    if (mndCtxt.getCpuInfo().hasNeon()) {
        generators.insert({ "float Neon", mndCtxt.getCpuGenerator(mnd::GeneratorType::FLOAT_NEON) });
        generators.insert({ "double Neon", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE_NEON) });
    }
    for (auto& device : mndCtxt.getDevices()) {
        if (mnd::Generator* gen; (gen = device.getGenerator(mnd::GeneratorType::FLOAT))) {
            generators.insert({ QString("float ") + QString::fromStdString(device.getName()),
                                gen });
        }
        if (mnd::Generator* gen; (gen = device.getGenerator(mnd::GeneratorType::DOUBLE))) {
            generators.insert({ QString("double ") + QString::fromStdString(device.getName()),
                                gen });
        }
        if (mnd::Generator* gen; (gen = device.getGenerator(mnd::GeneratorType::DOUBLE_DOUBLE))) {
            generators.insert({ QString("double double ") + QString::fromStdString(device.getName()),
                                gen });
        }
    }

    auto& defGen = mndCtxt.getDefaultGenerator();
    for (auto it = defGen.getGenerators().rbegin(); it != defGen.getGenerators().rend(); it++) {
        auto& [prec, gen] = *it;
        ui->table->insertRow(0);
        QLineEdit* le = createFloatText();
        QComboBox* comboBox = createComboBox();
        le->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        comboBox->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        ui->table->setCellWidget(0, 0, le);
        ui->table->setCellWidget(0, 1, comboBox);
        tableContent.push_back({ le, comboBox });
        for (auto [n, g] : generators) {
            if (gen == g) {
                comboBox->setCurrentText(n);
            }
        }
        le->setText(QString::number(static_cast<double>(prec)));
        comboBox->adjustSize();
        le->adjustSize();
    }
    ui->table->resizeColumnsToContents();


    //ui->addRow->setIcon(ui->addRow->style()->standardIcon(QStyle::SP_));
    //ui->moveRowUp->setIcon(ui->moveRowUp->style()->standardIcon(QStyle::SP_ArrowUp));
    //ui->moveRowDown->setIcon(ui->moveRowDown->style()->standardIcon(QStyle::SP_ArrowDown));
}


ChooseGenerators::~ChooseGenerators()
{
}


QComboBox* ChooseGenerators::createComboBox(void)
{
    QComboBox* qcb = new QComboBox(ui->table);

    for (auto [name, type] : generators) {
        qcb->addItem(name);
    }

    return qcb;
}


QLineEdit* ChooseGenerators::createFloatText(void)
{
    QLineEdit* le = new QLineEdit(ui->table);
    le->setValidator(floatValidator.get());
    return le;
}


void ChooseGenerators::on_buttonBox_accepted()
{
    if (!createdGenerator)
        createdGenerator = std::make_unique<mnd::AdaptiveGenerator>();
    createdGenerator->clear();
    try {
        for (size_t i = 0; i < tableContent.size(); i++) {
            //std::cout << tableContent.at(i).first << std::endl;
            //std::cout << tableContent.at(i).second << std::endl;
            QString precString = tableContent.at(i).first->text();
            QString genString = tableContent.at(i).second->currentText();

            mnd::Real precision = mnd::Real(precString.toStdString().c_str());
            mnd::Generator* generator = generators.at(genString);
            if (generator)
                createdGenerator->addGenerator(precision, *generator);
        }
    }
    catch(...) {
        // TODO
        createdGenerator = nullptr;
    }
}

