#include "benchmarkdialog.h"
#include <chrono>

BenchmarkDialog::BenchmarkDialog(mnd::MandelContext& mndContext, QWidget *parent) :
    QDialog(parent),
    mndContext{ mndContext }
{
    ui.setupUi(this);

    auto& devices = mndContext.getDevices();
    int nDevices = devices.size() + 1;
    ui.tableWidget->setColumnCount(2);
    ui.tableWidget->setRowCount(nDevices);
    ui.tableWidget->setHorizontalHeaderLabels({"Single Precision", "Double Precision"});

    QString cpuDesc = ("CPU [" + mndContext.getCpuInfo().getBrand() + "]").c_str();
    ui.tableWidget->setVerticalHeaderItem(0, new QTableWidgetItem(cpuDesc));
    for (int i = 0; i < devices.size(); i++) {
        QString cpuDesc = ("GPU " + std::to_string(i + 1) + " [" + devices[i].getVendor() + " " + devices[i].getName() + "]").c_str();
        ui.tableWidget->setVerticalHeaderItem(i + 1, new QTableWidgetItem(cpuDesc));
    }
}

mnd::MandelViewport BenchmarkDialog::benchViewport(void) const
{
    return mnd::MandelViewport{ -0.758267525104592591494, -0.066895616551111110830, 0.000000043217777777655, 0.000000043217777777655 };
}


double BenchmarkDialog::measureMips(const std::function<Bitmap<float>()>& bench) const
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
    printf("test took %lld nanos\n", duration_cast<nanoseconds>(after - before).count());
    printf("test did %lld iters\n", sum);
    double megaItersPerSecond = iterPerNanos * 1000.0;
    return megaItersPerSecond;
}

QString BenchmarkDialog::benchmarkResult(mnd::Generator& mg, int size, int iters) const
{
    mnd::MandelInfo mi;
    mi.bWidth = size;
    mi.bHeight = size;
    mi.maxIter = iters;
    mi.view = benchViewport();
    double megaItersPerSecond = measureMips([&mg, &mi] () {
        Bitmap<float> bmp(mi.bWidth, mi.bHeight);
        mg.generate(mi, bmp.pixels.get());
        return bmp;
    });

    QString mips = QString::number(megaItersPerSecond, 'f', 2);
    return mips;
}

void BenchmarkDialog::on_run_clicked()
{
    mnd::Generator& cpuf = mndContext.getCpuGeneratorFloat();
    mnd::Generator& cpud = mndContext.getCpuGeneratorDouble();

    ui.tableWidget->setItem(0, 0, new QTableWidgetItem(benchmarkResult(cpuf, 1000, 5000)));
    ui.tableWidget->setItem(0, 1, new QTableWidgetItem(benchmarkResult(cpud, 1000, 5000)));

    auto& devices = mndContext.getDevices();
    for (int i = 0; i < devices.size(); i++) {
        if (mnd::Generator* gpuf; gpuf = devices[i].getGeneratorFloat()) {
            ui.tableWidget->setItem(i + 1, 0, new QTableWidgetItem(benchmarkResult(*gpuf, 4000, 5000)));
        }
        if (mnd::Generator* gpud; gpud = devices[i].getGeneratorDouble()) {
            ui.tableWidget->setItem(i + 1, 1, new QTableWidgetItem(benchmarkResult(*gpud, 4000, 5000)));
        }
    }
//    ui.tableWidget->setItem(0, 1, new QTableWidgetItem(benchmarkResult(clg, 4000, 10000)));
}
