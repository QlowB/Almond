#include "choosegenerators.h"
#include "ui_choosegenerators.h"

#include "Hardware.h"

#include <QComboBox>
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>



mnd::MandelViewport Benchmarker::benchViewport(void)
{
    return mnd::MandelViewport{ -1.250000598933854152929, 0.0001879894057291665530, 0.0000003839916666666565, 0.0000003839916666666565 };
}


const std::vector<mnd::MandelInfo> Benchmarker::benches {
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
    mnd::MandelInfo{ benchViewport(), 512, 1024, 4000, false },
    mnd::MandelInfo{ benchViewport(), 1024, 1024, 4000, false },
    mnd::MandelInfo{ benchViewport(), 1024, 1024, 8000, false },
    mnd::MandelInfo{ benchViewport(), 1024, 1024, 16000, false },
    mnd::MandelInfo{ benchViewport(), 1024, 2048, 16000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 16000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 32000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 64000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 128000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 256000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 512000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 1024000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 4096000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 8192000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 16384000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 32768000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 65536000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 131072000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 262144000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 524288000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 1048576000, false },
    mnd::MandelInfo{ benchViewport(), 2048, 2048, 2097152000, false },
};


Benchmarker::~Benchmarker(void)
{
}


std::pair<long long, std::chrono::nanoseconds> Benchmarker::measureMips(const std::function<Bitmap<float>*()>& bench) const
{
    using namespace std::chrono;
    auto before = high_resolution_clock::now();
    auto* bitmap = bench();
    auto after = high_resolution_clock::now();

    long long sum = 0;
    for (int i = 0; i < bitmap->width * bitmap->height; i++) {
        sum += static_cast<long long>(std::floor(bitmap->pixels[size_t(i)]));
    }

    return std::make_pair(sum, duration_cast<nanoseconds>(after - before));
}

double Benchmarker::benchmarkResult(mnd::MandelGenerator& mg) const
{
    size_t testIndex = 0;

    for (size_t i = 0; i < benches.size(); i++) {
        const mnd::MandelInfo& mi = benches[i];
        Bitmap<float> bmp(mi.bWidth, mi.bHeight);
        auto [iters, time] = measureMips([&mg, &mi, &bmp]() {
            mg.generate(mi, bmp.pixels.get());
            return &bmp;
        });
        if (time > std::chrono::milliseconds(500)) {
            testIndex = i + 2;
            //printf("testing index %d\n", testIndex);
            fflush(stdout);
            break;
        }
        else if (time < std::chrono::milliseconds(10)) {
            i += 7;
        }
    }


    const mnd::MandelInfo& mi = benches[(testIndex >= benches.size()) ? (benches.size() - 1) : testIndex];
    Bitmap<float> bmp(mi.bWidth, mi.bHeight);
    auto [iters, time] = measureMips([&mg, &mi, &bmp]() {
        mg.generate(mi, bmp.pixels.get());
        return &bmp;
    });

    return double(iters) / time.count() * 1000;
}


void Benchmarker::run(void)
{
    double result = benchmarkResult(generator);
    emit finished(row, percentage, result);
}


ChooseGenerators::ChooseGenerators(mnd::MandelContext& mndCtxt, QWidget *parent) :
    QDialog{ parent },
    ui{ std::make_unique<Ui::ChooseGenerators>() },
    mndCtxt{ mndCtxt },
    tableContent{}
{
    ui->setupUi(this);
    ui->progressBar->setRange(0, 1000);
    benchmarker.setMaxThreadCount(1);

    QRegExp floatingpoint{ "^[-+]?(\\d*\\.?\\d+|\\d+\\.?\\d*)([eE][-+]\\d+)?$" };
    floatValidator = std::make_unique<QRegExpValidator>(floatingpoint, this);

    for (auto genType : mndCtxt.getSupportedTypes()) {
        const std::string& typeName = mnd::getGeneratorName(genType);
        generators.insert({ QString::fromStdString(typeName), mndCtxt.getCpuGenerator(genType) });
    }
    for (auto& device : mndCtxt.getDevices()) {
        for (auto genType : device.getSupportedTypes()) {
            const std::string& typeName = mnd::getGeneratorName(genType) + " [" + device.getName() + "]";
            generators.insert({ QString::fromStdString(typeName), device.getGenerator(genType) });
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
        le->setText(QString::fromStdString(mnd::toLegibleString(prec)));
        comboBox->adjustSize();
        le->adjustSize();
    }
    ui->table->resizeColumnsToContents();

    std::vector<mnd::GeneratorType> generatorTypes = mndCtxt.getSupportedTypes();
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
    }

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


void ChooseGenerators::setBenchmarkResult(int row, float percentage, double result)
{
    this->ui->generatorTable->setItem(row, 2, new QTableWidgetItem);
    this->ui->generatorTable->item(row, 2)->setText(QString::number(result));
    ui->progressBar->setValue(int(percentage * 10.0f));
}


void ChooseGenerators::on_buttonBox_accepted()
{
    if (!createdGenerator)
        createdGenerator = std::make_unique<mnd::AdaptiveGenerator>();
    createdGenerator->clear();
    try {
        for (size_t i = 0; i < tableContent.size(); i++) {
            QString precString = tableContent.at(i).first->text();
            QString genString = tableContent.at(i).second->currentText();

            mnd::Real precision = mnd::Real(precString.toStdString().c_str());
            mnd::MandelGenerator* generator = generators.at(genString);
            if (generator)
                createdGenerator->addGenerator(precision, *generator);
        }
    }
    catch(...) {
        // TODO
        createdGenerator = nullptr;
    }
}


void ChooseGenerators::on_run_clicked()
{
    ui->progressBar->setValue(0);
    for (int i = 0; i < ui->generatorTable->rowCount(); i++) {
        mnd::MandelGenerator* gen = actualGenerators.at(i);
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
