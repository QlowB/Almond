#include "benchmarkdialog.h"
#include <chrono>
#include <cmath>


mnd::MandelViewport Benchmarker::benchViewport(void) const
{
    return mnd::MandelViewport{ -0.758267525104592591494, -0.066895616551111110830, 0.000000043217777777655, 0.000000043217777777655 };
}


double Benchmarker::measureMips(const std::function<Bitmap<float>()>& bench) const
{
    using namespace std::chrono;
    auto before = high_resolution_clock::now();
    auto bitmap = bench();
    auto after = high_resolution_clock::now();

    long long sum = 0;
    for (int i = 0; i < bitmap.width * bitmap.height; i++) {
        sum += std::floor(bitmap.pixels[size_t(i)]);
    }

    double iterPerNanos = double(sum) / duration_cast<nanoseconds>(after - before).count();
    printf("test took %lld millis\n", duration_cast<milliseconds>(after - before).count());
    printf("test did %lld iters\n", sum);
    double megaItersPerSecond = iterPerNanos * 1000.0;
    return megaItersPerSecond;
}

double Benchmarker::benchmarkResult(mnd::Generator& mg) const
{
    // create testbenchmark
    mnd::MandelInfo mi;
    mi.bWidth = 250;
    mi.bHeight = 250;
    mi.maxIter = 4000;
    mi.view = benchViewport();
    double testValue = measureMips([&mg, &mi] () {
        Bitmap<float> bmp(mi.bWidth, mi.bHeight);
        mg.generate(mi, bmp.pixels.get());
        return bmp;
    });

    printf("testbench: %lf\n", testValue);

    std::vector<std::pair<double, mnd::MandelInfo>> benches {
        { 200, mnd::MandelInfo{ benchViewport(), 750, 750, 5000} },
        { 500, mnd::MandelInfo{ benchViewport(), 2000, 1000, 7500} },
        { 2000, mnd::MandelInfo{ benchViewport(), 2000, 2000, 15000} },
        { 5000, mnd::MandelInfo{ benchViewport(), 3000, 3000, 30000} },
        { 10000, mnd::MandelInfo{ benchViewport(), 4000, 4000, 75000} },
        { 100000, mnd::MandelInfo{ benchViewport(), 6000, 6000, 750000} },
        { std::numeric_limits<double>::max(), mnd::MandelInfo{ benchViewport(), 7000, 7000, 1000000} }
    };

    double megaItersPerSecond = 0.0;
    if (testValue < 100) {
        megaItersPerSecond = testValue;
    }
    else {
        for (auto& [thresh, info] : benches) {
            auto& m = info;
            if (testValue < thresh) {
                megaItersPerSecond = measureMips([&mg, &m] () {
                    Bitmap<float> bmp(m.bWidth, m.bHeight);
                    mg.generate(m, bmp.pixels.get());
                    return bmp;
                });
                break;
            }
        }
    }



    return megaItersPerSecond;
}


void Benchmarker::start(void)
{
    mnd::Generator& cpuf = mndContext.getCpuGeneratorFloat();
    mnd::Generator& cpud = mndContext.getCpuGeneratorDouble();
    mnd::Generator& cpu128 = mndContext.getCpuGenerator128();

    double nTests = 3;

    auto& devices = mndContext.getDevices();
    for (int i = 0; i < devices.size(); i++) {
        if (mnd::Generator* gpuf; gpuf = devices[i].getGeneratorFloat()) {
            nTests++;
        }
        if (mnd::Generator* gpud; gpud = devices[i].getGeneratorDouble()) {
            nTests++;
        }
    }

    double progress = 90.0 / nTests;

    BenchmarkResult br;
    br.values.push_back({});
    br.percentage = 10;

    emit update(br);

    std::vector<double>& cpu = br.values[0];
    cpu.push_back(benchmarkResult(cpuf));
    br.percentage += progress;
    emit update(br);
    cpu.push_back(benchmarkResult(cpud));
    br.percentage += progress;
    emit update(br);
    cpu.push_back(benchmarkResult(cpu128));
    br.percentage += progress;
    emit update(br);

    for (int i = 0; i < devices.size(); i++) {
        br.values.push_back({});
        std::vector<double>& gpu = br.values[br.values.size() - 1];
        if (mnd::Generator* gpuf; gpuf = devices[i].getGeneratorFloat()) {
            gpu.push_back(benchmarkResult(*gpuf));
            br.percentage += progress;
            emit update(br);
        }
        if (mnd::Generator* gpud; gpud = devices[i].getGeneratorDouble()) {
            gpu.push_back(benchmarkResult(*gpud));
            br.percentage += progress;
            emit update(br);
        }
    }
    printf("benchmark finished\n");
    emit update(br);
    emit finished();
}


BenchmarkDialog::BenchmarkDialog(mnd::MandelContext& mndContext, QWidget *parent) :
    QDialog(parent),
    mndContext{ mndContext },
    benchmarker{ mndContext }
{
    ui.setupUi(this);

    auto& devices = mndContext.getDevices();
    int nDevices = devices.size() + 1;
    ui.tableWidget->setColumnCount(3);
    ui.tableWidget->setRowCount(nDevices);
    ui.tableWidget->setHorizontalHeaderLabels({"Single Precision", "Double Precision", "128-bit Fixed Point"});

    QString cpuDesc = ("CPU [" + mndContext.getCpuInfo().getBrand() + "]").c_str();
    ui.tableWidget->setVerticalHeaderItem(0, new QTableWidgetItem(cpuDesc));
    for (int i = 0; i < devices.size(); i++) {
        std::string cpuDescS = std::string("GPU ") + std::to_string(i + 1) + " [" + devices[i].getName().c_str() + "]";
        QString cpuDesc = QString::fromLatin1(cpuDescS.c_str());
        /*printf("brand [%d]: --> %s <--\n", (int) cpuDescS.size(), cpuDescS.c_str());
        for (int x = 0; x < cpuDescS.size(); x++) {
            printf("%d\n", cpuDescS[x]);
        }
        printf("\n");*/
        auto label = new QTableWidgetItem(cpuDesc);
        label->setStatusTip(QString::fromLatin1(devices[i].getName().c_str()));
        ui.tableWidget->setVerticalHeaderItem(i + 1, label);
    }

    qRegisterMetaType<BenchmarkResult>();

    benchmarker.moveToThread(&benchThread);
    connect(&benchThread, &QThread::started, &benchmarker, &Benchmarker::start);
    connect(&benchmarker, SIGNAL (finished()), &benchThread, SLOT (quit()));
    connect(&benchmarker, SIGNAL (update(BenchmarkResult)), this, SLOT (update(BenchmarkResult)));
}


void BenchmarkDialog::update(BenchmarkResult br)
{
    std::vector<double> cpu = br.values[0];
    for (int j = 0; j < int(br.values.size()); j++) {
        for (int i = 0; i < int(br.values[j].size()); i++) {
            ui.tableWidget->setItem(j, i, new QTableWidgetItem(QString::number(br.values[j][i])));
        }
    }
    ui.progressBar->setValue(int(br.percentage));
}


void BenchmarkDialog::on_run_clicked()
{
    if (!benchThread.isRunning()) {
        /*for (int i = 0; i < ui.tableWidget->columnCount(); i++) {
            for (int j = 0; j < ui.tableWidget->rowCount(); j++) {
                ui.tableWidget->setItem(j, i, new QTableWidgetItem(""));
            }
        }*/

        benchThread.start();
    }

//    ui.tableWidget->setItem(0, 1, new QTableWidgetItem(benchmarkResult(clg, 4000, 10000)));
}
