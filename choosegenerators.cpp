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
    mnd::MandelInfo{ benchViewport(), 50, 50, 250, false },
    mnd::MandelInfo{ benchViewport(), 50, 50, 500, false },
    mnd::MandelInfo{ benchViewport(), 50, 100, 500, false },
    mnd::MandelInfo{ benchViewport(), 100, 100, 500, false },
    mnd::MandelInfo{ benchViewport(), 100, 100, 1000, false },
    mnd::MandelInfo{ benchViewport(), 100, 200, 1000, false },
    mnd::MandelInfo{ benchViewport(), 200, 200, 1000, false },
    mnd::MandelInfo{ benchViewport(), 200, 200, 2000, false },
    mnd::MandelInfo{ benchViewport(), 200, 400, 2000, false },
    mnd::MandelInfo{ benchViewport(), 400, 400, 2000, false },
    mnd::MandelInfo{ benchViewport(), 400, 400, 4000, false },
    mnd::MandelInfo{ benchViewport(), 400, 800, 4000, false },
    mnd::MandelInfo{ benchViewport(), 800, 800, 4000, false },
    mnd::MandelInfo{ benchViewport(), 800, 800, 8000, false },
    mnd::MandelInfo{ benchViewport(), 800, 800, 16000, false },
    mnd::MandelInfo{ benchViewport(), 800, 1600, 16000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 16000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 32000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 64000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 128000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 256000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 512000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 1024000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 4096000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 8192000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 16384000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 32768000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 65536000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 131072000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 262144000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 524288000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 1048576000, false },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 2097152000, false },
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

double Benchmarker::benchmarkResult(mnd::Generator& mg) const
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

    auto genName = [] (mnd::GeneratorType type) {
        static const std::map<mnd::GeneratorType, QString> names {
            { mnd::GeneratorType::FLOAT, "float" },
            { mnd::GeneratorType::FLOAT_SSE2, "float SSE2" },
            { mnd::GeneratorType::FLOAT_AVX, "float AVX" },
            { mnd::GeneratorType::FLOAT_AVX512, "float AVX512" },
            { mnd::GeneratorType::FLOAT_NEON, "float Neon" },
            { mnd::GeneratorType::DOUBLE, "double" },
            { mnd::GeneratorType::DOUBLE_SSE2, "double SSE2" },
            { mnd::GeneratorType::DOUBLE_AVX, "double AVX" },
            { mnd::GeneratorType::DOUBLE_AVX512, "double AVX512" },
            { mnd::GeneratorType::DOUBLE_NEON, "double Neon" },
            { mnd::GeneratorType::DOUBLE_DOUBLE, "double double" },
            { mnd::GeneratorType::DOUBLE_DOUBLE_AVX, "double double AVX" },
            { mnd::GeneratorType::QUAD_DOUBLE, "quad double" },
            { mnd::GeneratorType::FLOAT128, "float128" },
            { mnd::GeneratorType::FLOAT256, "float256" },
            { mnd::GeneratorType::FIXED512, "fixed512" },
        };

        return names.at(type);
    };

    generators = std::map<QString, mnd::Generator*> {
        { "float", mndCtxt.getCpuGenerator(mnd::GeneratorType::FLOAT) },
        { "double", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE) },
        { "double double", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE_DOUBLE) },
        { "quad double", mndCtxt.getCpuGenerator(mnd::GeneratorType::QUAD_DOUBLE) },
        { "float256", mndCtxt.getCpuGenerator(mnd::GeneratorType::FLOAT256) },
        { "fixed512", mndCtxt.getCpuGenerator(mnd::GeneratorType::FIXED512) },
    };

    if (mndCtxt.getCpuInfo().hasSse2()) {
        generators.insert({ "float SSE2", mndCtxt.getCpuGenerator(mnd::GeneratorType::FLOAT_SSE2) });
        generators.insert({ "double SSE2", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE_SSE2) });
    }
    if (mndCtxt.getCpuInfo().hasAvx()) {
        generators.insert({ "float AVX", mndCtxt.getCpuGenerator(mnd::GeneratorType::FLOAT_AVX) });
        generators.insert({ "double AVX", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE_AVX) });
        if (mndCtxt.getCpuInfo().hasFma()) {
            generators.insert({ "double double AVX", mndCtxt.getCpuGenerator(mnd::GeneratorType::DOUBLE_DOUBLE_AVX) });
        }
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

    std::vector<mnd::GeneratorType> generatorTypes = mndCtxt.getSupportedTypes();
    for (size_t i = 0; i < generatorTypes.size(); i++) {
        ui->generatorTable->insertRow(ui->generatorTable->rowCount());
        ui->generatorTable->setItem(ui->generatorTable->rowCount() - 1, 0, new QTableWidgetItem);
        ui->generatorTable->item(ui->generatorTable->rowCount() - 1, 0)->setText(genName(generatorTypes[i]));
        actualGenerators.push_back(mndCtxt.getCpuGenerator(generatorTypes[i]));
    }

    for (auto& device : mndCtxt.getDevices()) {
        std::vector<mnd::GeneratorType> generatorTypes = device.getSupportedTypes();
        for (size_t i = 0; i < generatorTypes.size(); i++) {
            ui->generatorTable->insertRow(ui->generatorTable->rowCount());
            ui->generatorTable->setItem(ui->generatorTable->rowCount() - 1, 0, new QTableWidgetItem);
            ui->generatorTable->item(ui->generatorTable->rowCount() - 1, 0)->setText(genName(generatorTypes[i]) + " [" + QString::fromStdString(device.getName()) + "]");
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
    this->ui->generatorTable->setItem(row, 1, new QTableWidgetItem);
    this->ui->generatorTable->item(row, 1)->setText(QString::number(result));
    ui->progressBar->setValue(int(percentage * 10.0f));
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


void ChooseGenerators::on_run_clicked()
{
    ui->progressBar->setValue(0);
    for (int i = 0; i < ui->generatorTable->rowCount(); i++) {
        mnd::Generator* gen = actualGenerators.at(i);
        if (gen != nullptr) {
            Benchmarker* bench = new Benchmarker(mndCtxt, *gen, i, 100.0f * (i + 1) / ui->generatorTable->rowCount());
            QObject::connect(bench, &Benchmarker::finished, this, &ChooseGenerators::setBenchmarkResult);
            benchmarker.start(bench);
        }
    }
}


void ChooseGenerators::on_generatorTable_cellDoubleClicked(int row, int column)
{
    if (column == 1) {
        QMessageBox msgBox{ this };
        msgBox.setText("Would you like to benchmark this generator?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int response = msgBox.exec();
        if (response == QMessageBox::Yes) {
            mnd::Generator* gen = actualGenerators.at(row);
            if (gen != nullptr) {
                ui->progressBar->setValue(0);
                Benchmarker* bench = new Benchmarker(mndCtxt, *gen, row, 100.0f);
                QObject::connect(bench, &Benchmarker::finished, this, &ChooseGenerators::setBenchmarkResult);
                benchmarker.start(bench);
            }
        }
    }
}
