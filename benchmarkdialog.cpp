#include "benchmarkdialog.h"
#include <chrono>

BenchmarkDialog::BenchmarkDialog(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);
}

MandelViewport BenchmarkDialog::benchViewport(void) const
{
    return MandelViewport{ -0.758267525104592591494, -0.066895616551111110830, 0.000000043217777777655, 0.000000043217777777655 };
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

QString BenchmarkDialog::benchmarkResult(MandelGenerator& mg, int size, int iters) const
{
    MandelInfo mi;
    mi.bWidth = size;
    mi.bHeight = size;
    mi.maxIter = iters;
    mi.view = benchViewport();
    double megaItersPerSecond = measureMips([&mg, &mi] () { return mg.generateRaw(mi); });
    QString mips = QString::number(megaItersPerSecond, 'f', 2);
    return mips;
}

void BenchmarkDialog::on_run_clicked()
{
    CpuGenerator<double> cpg;
    CpuGenerator<float> cpgf;
    ClGenerator clg;

    ui.tableWidget->setItem(1, 0, new QTableWidgetItem(benchmarkResult(cpg, 1000, 5000)));
    ui.tableWidget->setItem(0, 0, new QTableWidgetItem(benchmarkResult(cpgf, 1000, 5000)));
    ui.tableWidget->setItem(0, 1, new QTableWidgetItem(benchmarkResult(clg, 2000, 8000)));
}
