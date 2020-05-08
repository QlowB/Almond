#include "choosegenerators.h"
#include "ui_choosegenerators.h"

#include "Almond.h"

#include <Hardware.h>
#include <IterationCompiler.h>

#include <QComboBox>
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>

#include <cstring>


mnd::MandelViewport Benchmarker::benchViewport(void)
{
    //return mnd::MandelViewport{ -1.250000598933854152929, 0.0001879894057291665530, 0.0000003839916666666565, 0.0000003839916666666565 };
    //return mnd::MandelViewport::centerView();
    return mnd::MandelViewport{ 0, 0, 0.0000003839916666666565, 0.0000003839916666666565 };
}


static std::vector<mnd::MandelInfo> createBenches()
{
    std::vector<mnd::MandelInfo> vec;
    for (int i = 0; i < 50; i++) {
        int expo = i + 14;
        int w = 5;
        int h = 5;

        while (int(expo * 1) - w - h > 15 && w <= 10 && h <= 10) {
            w++;
            if (int(expo * 1) - w - h > 15)
                h++;
        }

        long wi = 1L << w;
        long he = 1L << h;
        long iter = 1L << (expo - w - h);
        vec.push_back(mnd::MandelInfo{ mnd::MandelViewport::centerView(), wi, he, iter, false, false, 0.0, 0.0 });
    }
    return vec;
}


const std::vector<mnd::MandelInfo> Benchmarker::benches = {
    mnd::MandelInfo{ benchViewport(), 32, 32, 15, false },
    mnd::MandelInfo{ benchViewport(), 32, 32, 25, false },
    mnd::MandelInfo{ benchViewport(), 32, 32, 75, false },
    mnd::MandelInfo{ benchViewport(), 32, 32, 125, false },
    mnd::MandelInfo{ benchViewport(), 32, 32, 250, false },
    mnd::MandelInfo{ benchViewport(), 64, 32, 250, false },
    mnd::MandelInfo{ benchViewport(), 64, 64, 250, false },
    mnd::MandelInfo{ benchViewport(), 64, 64, 500, false },
    mnd::MandelInfo{ benchViewport(), 64, 128, 500, false },
    mnd::MandelInfo{ benchViewport(), 128, 128, 500, false },
    mnd::MandelInfo{ benchViewport(), 128, 128, 1000, false },
    mnd::MandelInfo{ benchViewport(), 128, 256, 1000, false },
    mnd::MandelInfo{ benchViewport(), 256, 256, 1000, false },
    mnd::MandelInfo{ benchViewport(), 256, 256, 2000, false },
    mnd::MandelInfo{ benchViewport(), 256, 512, 2000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 2000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 4000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 8000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 16000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 32000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 64000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 128000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 256000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 512000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 1024000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 2048000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 4096000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 8192000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 16384000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 32768000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 65536000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 131072000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 262144000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 524288000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 1048576000, false },
    mnd::MandelInfo{ benchViewport(), 512, 512, 2097152000, false },
};


Benchmarker::~Benchmarker(void)
{
}


std::pair<long long, std::chrono::nanoseconds> Benchmarker::measureMips(
        std::function<void(Bitmap<float>&)> bench, Bitmap<float>& bmp) const
{
    using namespace std::chrono;
    auto before = high_resolution_clock::now();
    bench(bmp);
    auto after = high_resolution_clock::now();

    long long sum = 0;
    for (int i = 0; i < bmp.width * bmp.height; i++) {
        sum += static_cast<long long>(std::floor(bmp.pixels[size_t(i)]));
    }

    return std::make_pair(sum, duration_cast<nanoseconds>(after - before));
}

double Benchmarker::benchmarkResult(mnd::MandelGenerator& mg) const
{
    size_t testIndex = 0;

    for (size_t i = 0; i < benches.size(); i++) {
        const mnd::MandelInfo& mi = benches[i];
        Bitmap<float> bmp(mi.bWidth, mi.bHeight);
        auto [iters, time] = measureMips([&mg, mi](Bitmap<float>& bmp) {
            mg.generate(mi, bmp.pixels.get());
        }, bmp);
        if (time > std::chrono::milliseconds(120)) {
            testIndex = i + 4;
            //printf("testing index for generator %s: %d\n", (mnd::toString(mg.getType()) + ", " + mnd::toString(mg.getExtension())).c_str(), testIndex);
            //printf("    w: %d, h: %d, iter: %d\n", benches[testIndex].bWidth, benches[testIndex].bHeight, benches[testIndex].maxIter);
            //fflush(stdout);
            break;
        }
        else if (time < std::chrono::milliseconds(3)) {
            i += 7;
        }
        else if (time < std::chrono::milliseconds(20)) {
            i += 3;
        }
    }

    try {
        const mnd::MandelInfo& mi = benches[(testIndex >= benches.size()) ? (benches.size() - 1) : testIndex];
        Bitmap<float> bmp(mi.bWidth, mi.bHeight);
        auto [iters, time] = measureMips([&mg, mi](Bitmap<float>& bmp) {
            mg.generate(mi, bmp.pixels.get());
        }, bmp);

        //printf("%lld iterations in %lld microseconds\n\n", iters, time.count() / 1000);

        return double(iters) / time.count() * 1000;
    }
    catch(const std::string& c) {
        printf("error benchmarking: %s\n", c.c_str());
    }
    catch(...) {
        printf("error benchmarking\n");
    }
    return 0;
}


void Benchmarker::run(void)
{
    double result = benchmarkResult(generator);
    emit finished(row, percentage, result);
}


ChooseGenerators::ChooseGenerators(mnd::MandelContext& mndCtxt, mnd::AdaptiveGenerator& generator, Almond& owner) :
    QDialog{ &owner },
    owner{ owner },
    ui{ std::make_unique<Ui::ChooseGenerators>() },
    mndCtxt{ mndCtxt },
    tableContent{},
    generator{ generator }
{
    ui->setupUi(this);
    ui->progressBar->setRange(0, 1000);
    benchmarker.setMaxThreadCount(1);

    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);
    f.setPointSize(12);

    QRegExp floatingpoint{ "^[-+]?(\\d*\\.?\\d+|\\d+\\.?\\d*)([eE][-+]\\d+)?$" };
    floatValidator = std::make_unique<QRegExpValidator>(floatingpoint, this);

    for (auto genType : mndCtxt.getSupportedTypes()) {
        const std::string& typeName = mnd::getGeneratorName(genType);
        generators.insert({ QString::fromStdString(typeName), mndCtxt.getCpuGenerator(genType) });
    }
    for (auto& device : mndCtxt.getDevices()) {
        for (auto genType : device->getSupportedTypes()) {
            const std::string& typeName = mnd::getGeneratorName(genType) + " [" + device->getName() + "]";
            generators.insert({ QString::fromStdString(typeName), device->getGenerator(genType) });
        }
    }

    std::vector<mnd::MandelGenerator*> allGenerators;

    /*for (auto genType : mndCtxt.getSupportedTypes()) {
        allGenerators.push_back(mndCtxt.getCpuGenerator(genType));
    }
    for (auto& device : mndCtxt.getDevices()) {
        for (auto genType : device.getSupportedTypes()) {
            allGenerators.push_back(device.getGenerator(genType));
        }
    }*/

    initializeTables();

    //ui->addRow->setIcon(ui->addRow->style()->standardIcon(QStyle::SP_));
    //ui->moveRowUp->setIcon(ui->moveRowUp->style()->standardIcon(QStyle::SP_ArrowUp));
    //ui->moveRowDown->setIcon(ui->moveRowDown->style()->standardIcon(QStyle::SP_ArrowDown));
}


ChooseGenerators::ChooseGenerators(mnd::MandelContext& mndCtxt, mnd::GeneratorCollection& gc,
                                   mnd::AdaptiveGenerator& generator, Almond& owner) :
    QDialog{ &owner },
    owner{ owner },
    ui{ std::make_unique<Ui::ChooseGenerators>() },
    mndCtxt{ mndCtxt },
    tableContent{},
    generator{ generator }
{
    ui->setupUi(this);
    ui->progressBar->setRange(0, 1000);
    benchmarker.setMaxThreadCount(1);

    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);
    f.setPointSize(12);

    QRegExp floatingpoint{ "^[-+]?(\\d*\\.?\\d+|\\d+\\.?\\d*)([eE][-+]\\d+)?$" };
    floatValidator = std::make_unique<QRegExpValidator>(floatingpoint, this);

    for (auto& gen : gc.cpuGenerators) {
        const mnd::Precision p = gen->getType();
        const mnd::CpuExtension ce = gen->getExtension();
        std::string typeName = mnd::toString(p) + " " + mnd::toString(ce);
        generators.insert({ QString::fromStdString(typeName), gen.get() });
    }


    for (auto& gen : gc.clGenerators) {
        const mnd::Precision p = gen->getType();
        const mnd::CpuExtension ce = gen->getExtension();
        mnd::MandelDevice* dev = gen->getDevice();
        std::string devString = dev != nullptr ? dev->getName() : "";
        std::string typeName = mnd::toString(p) + " " + mnd::toString(ce) + " [" + devString + "]";
        generators.insert({ QString::fromStdString(typeName), gen.get() });
    }

    std::vector<mnd::MandelGenerator*> allGenerators;

    /*for (auto& gen : gc.cpuGenerators) {
        allGenerators.push_back(gen.get());
    }
    for (auto& gen : gc.clGenerators) {
        allGenerators.push_back(gen.get());
    }*/

    initializeTables();
}


ChooseGenerators::~ChooseGenerators()
{
}



void ChooseGenerators::initializeTables(void)
{
    for (auto it = generator.getGenerators().rbegin(); it != generator.getGenerators().rend(); it++) {
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
        le->setText(QString::fromStdString(mnd::toLegibleString(prec)));
        comboBox->adjustSize();
        le->adjustSize();
    }
    ui->table->resizeColumnsToContents();

    int i = 0;
    for (auto it = generators.begin(); it != generators.end(); ++it, ++i) {
        int rowCount = ui->generatorTable->rowCount();
        ui->generatorTable->insertRow(rowCount);
        ui->generatorTable->setItem(rowCount, 0, new QTableWidgetItem);
        ui->generatorTable->setItem(rowCount, 1, new QTableWidgetItem);
        mnd::MandelGenerator& gene = *it->second;
        mnd::Precision p = gene.getType();
        mnd::CpuExtension ex = gene.getExtension();
        mnd::MandelDevice* dev = gene.getDevice();

        std::string desc = mnd::toString(p) + " " + mnd::toString(ex);
        if (dev)
            desc += "[" + dev->getName() + "]";
        const mnd::Real& prec = gene.getPrecision();
        ui->generatorTable->item(rowCount, 0)->setText(QString::fromStdString(desc));
        ui->generatorTable->item(rowCount, 1)->setText(QString::fromStdString(mnd::toLegibleString(prec)));
        actualGenerators.push_back(&gene);
    }

    /*std::vector<mnd::GeneratorType> generatorTypes = mndCtxt.getSupportedTypes();
    for (size_t i = 0; i < generatorTypes.size(); i++) {
        int rowCount = ui->generatorTable->rowCount();
        ui->generatorTable->insertRow(rowCount);
        ui->generatorTable->setItem(rowCount, 0, new QTableWidgetItem);
        ui->generatorTable->setItem(rowCount, 1, new QTableWidgetItem);
        const std::string& genName = mnd::getGeneratorName(generatorTypes[i]);
        const mnd::Real& prec = mndCtxt.getCpuGenerator(generatorTypes[i])->getPrecision();
        ui->generatorTable->item(rowCount, 0)->setText(QString::fromStdString(genName));
        ui->generatorTable->item(rowCount, 1)->setText(QString::fromStdString(mnd::toLegibleString(prec)));
        actualGenerators.push_back(mndCtxt.getCpuGenerator(generatorTypes[i]));
    }

    for (auto& device : mndCtxt.getDevices()) {
        std::vector<mnd::GeneratorType> generatorTypes = device.getSupportedTypes();
        for (size_t i = 0; i < generatorTypes.size(); i++) {
            int rowCount = ui->generatorTable->rowCount();
            ui->generatorTable->insertRow(rowCount);
            ui->generatorTable->setItem(rowCount, 0, new QTableWidgetItem);
            ui->generatorTable->setItem(rowCount, 1, new QTableWidgetItem);
            const std::string& genName = mnd::getGeneratorName(generatorTypes[i]) + " [" + device.getName() + "]";
            const mnd::Real& prec = device.getGenerator(generatorTypes[i])->getPrecision();
            ui->generatorTable->item(rowCount, 0)->setText(QString::fromStdString(genName));
            ui->generatorTable->item(rowCount, 1)->setText(QString::fromStdString(mnd::toLegibleString(prec)));
            actualGenerators.push_back(device.getGenerator(generatorTypes[i]));
        }
    }*/
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


void ChooseGenerators::addRowAt(int index)
{
    ui->table->insertRow(index);
    QLineEdit* le = createFloatText();
    QComboBox* comboBox = createComboBox();
    le->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    comboBox->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    ui->table->setCellWidget(index, 0, le);
    ui->table->setCellWidget(index, 1, comboBox);
    tableContent.push_back({ le, comboBox });
    comboBox->setCurrentIndex(0);
    le->setText("0.0");
    comboBox->adjustSize();
    le->adjustSize();
}


void ChooseGenerators::setBenchmarkResult(int row, float percentage, double result)
{
    this->ui->generatorTable->setItem(row, 2, new QTableWidgetItem);
    this->ui->generatorTable->item(row, 2)->setText(QString::number(result));
    ui->progressBar->setValue(int(percentage * 10.0f));
}


void ChooseGenerators::on_buttonBox_accepted()
{
    //if (!chosenGenerator)
    chosenGenerator = std::make_unique<mnd::AdaptiveGenerator>();
    //createdGenerator->clear();
    try {
        /*for (size_t i = 0; i < tableContent.size(); i++) {
            QString precString = tableContent.at(i).first->text();
            QString genString = tableContent.at(i).second->currentText();

            mnd::Real precision = mnd::Real(precString.toStdString().c_str());
            mnd::MandelGenerator* generator = generators.at(genString);
            if (generator)
                chosenGenerator->addGenerator(precision, *generator);
        }*/
        for (int i = 0; i < ui->table->rowCount(); i++) {
            QLineEdit* precItem = dynamic_cast<QLineEdit*>(ui->table->cellWidget(i, 0));
            QWidget* genWidget = ui->table->cellWidget(i, 1);
            QComboBox* genItem = dynamic_cast<QComboBox*>(genWidget);
            if (precItem && genItem) {
                QString precString = precItem->text();
                QString genString = genItem->currentText();

                printf("%s, %s\n", precString.toStdString().c_str(), genString.toStdString().c_str()); fflush(stdout);

                mnd::Real precision = mnd::Real(precString.toStdString().c_str());
                mnd::MandelGenerator* generator = generators.at(genString);
                if (generator)
                    chosenGenerator->addGenerator(precision, *generator);
            }
        }
    }
    catch(...) {
        // TODO
        chosenGenerator = nullptr;
    }
}


void ChooseGenerators::on_run_clicked()
{
    ui->progressBar->setValue(0);
    for (int i = 0; i < ui->generatorTable->rowCount(); i++) {
        mnd::MandelGenerator* gen = actualGenerators.at(unsigned(i));
        if (gen != nullptr) {
            Benchmarker* bench = new Benchmarker(mndCtxt, *gen, i, 100.0f * (i + 1) / ui->generatorTable->rowCount());
            QObject::connect(bench, &Benchmarker::finished, this, &ChooseGenerators::setBenchmarkResult);
            benchmarker.start(bench);
        }
    }
}


void ChooseGenerators::on_generatorTable_cellDoubleClicked(int row, int column)
{
    if (column == 2) {
        QMessageBox msgBox{ this };
        msgBox.setText("Would you like to benchmark this generator?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int response = msgBox.exec();
        if (response == QMessageBox::Yes) {
            mnd::MandelGenerator* gen = actualGenerators.at(row);
            if (gen != nullptr) {
                ui->progressBar->setValue(0);
                Benchmarker* bench = new Benchmarker(mndCtxt, *gen, row, 100.0f);
                QObject::connect(bench, &Benchmarker::finished, this, &ChooseGenerators::setBenchmarkResult);
                benchmarker.start(bench);
            }
        }
    }
}



void ChooseGenerators::on_addRow_clicked()
{
    int index = ui->table->rowCount();
    auto selected = ui->table->selectedRanges();
    if (!selected.empty())
        index = selected.last().bottomRow() + 1;
    addRowAt(index);
}

void ChooseGenerators::on_removeRow_clicked()
{
    auto selected = ui->table->selectedRanges();
    if (!selected.empty())
        ui->table->removeRow(selected.last().bottomRow());
}
