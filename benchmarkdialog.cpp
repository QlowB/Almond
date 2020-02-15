#include "benchmarkdialog.h"
#include <chrono>
#include <cmath>


mnd::MandelViewport Benchmarker::benchViewport(void)
{
    return mnd::MandelViewport{ -1.250000598933854152929, 0.0001879894057291665530, 0.0000003839916666666565, 0.0000003839916666666565 };
}

const std::vector<mnd::MandelInfo> Benchmarker::benches {
    mnd::MandelInfo{ benchViewport(), 50, 50, 250 },
    mnd::MandelInfo{ benchViewport(), 50, 50, 500 },
    mnd::MandelInfo{ benchViewport(), 50, 100, 500 },
    mnd::MandelInfo{ benchViewport(), 100, 100, 500 },
    mnd::MandelInfo{ benchViewport(), 100, 100, 1000 },
    mnd::MandelInfo{ benchViewport(), 100, 200, 1000 },
    mnd::MandelInfo{ benchViewport(), 200, 200, 1000 },
    mnd::MandelInfo{ benchViewport(), 200, 200, 2000 },
    mnd::MandelInfo{ benchViewport(), 200, 400, 2000 },
    mnd::MandelInfo{ benchViewport(), 400, 400, 2000 },
    mnd::MandelInfo{ benchViewport(), 400, 400, 4000 },
    mnd::MandelInfo{ benchViewport(), 400, 800, 4000 },
    mnd::MandelInfo{ benchViewport(), 800, 800, 4000 },
    mnd::MandelInfo{ benchViewport(), 800, 800, 8000 },
    mnd::MandelInfo{ benchViewport(), 800, 800, 16000 },
    mnd::MandelInfo{ benchViewport(), 800, 1600, 16000 },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 16000 },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 32000 },
    mnd::MandelInfo{ benchViewport(), 1600, 1600, 64000 },
    mnd::MandelInfo{ benchViewport(), 1600, 3200, 64000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 64000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 128000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 256000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 512000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 1024000 },
    mnd::MandelInfo{ benchViewport(), 3200, 3200, 2048000 },
    mnd::MandelInfo{ benchViewport(), 3200, 6400, 2048000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 2048000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 4096000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 8192000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 16384000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 32768000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 65536000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 131072000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 262144000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 524288000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 1048576000 },
    mnd::MandelInfo{ benchViewport(), 6400, 6400, 2097152000 },
};


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
        //auto data = std::make_unique<float[]>(size_t(mi.bWidth * mi.bHeight));
        Bitmap<float> bmp(mi.bWidth, mi.bHeight);
        auto [iters, time] = measureMips([&mg, &mi, &bmp]() {
            mg.generate(mi, bmp.pixels.get());
            return &bmp;
        });
        //printf("benchmark lvl %d, time %d ms\n", i, time.count() / 1000 / 1000);
        //fflush(stdout);
        if (time > std::chrono::milliseconds(1000)) {
            testIndex = i + 1;
            break;
        }
    }


    const mnd::MandelInfo& mi = benches[(testIndex >= benches.size()) ? (benches.size() - 1) : testIndex];
    //auto data = std::make_unique<float[]>(mi.bWidth * mi.bHeight);
    Bitmap<float> bmp(mi.bWidth, mi.bHeight);
    auto [iters, time] = measureMips([&mg, &mi, &bmp]() {
        mg.generate(mi, bmp.pixels.get());
        return &bmp;
    });
    //printf("bench time %d ms\n", time.count() / 1000 / 1000);
    //fflush(stdout);

    return double(iters) / time.count() * 1000;
}


void Benchmarker::start(void)
{
    mnd::Generator& cpuf = mndContext.getCpuGeneratorFloat();
    mnd::Generator& cpud = mndContext.getCpuGeneratorDouble();
    mnd::Generator* cpudd = mndContext.getCpuGeneratorDD();
    mnd::Generator* cpuqd = mndContext.getCpuGeneratorQD();
    mnd::Generator* cpu128 = mndContext.getCpuGeneratorQuad();
    mnd::Generator* cpu256 = mndContext.getCpuGeneratorOct();

    double nTests = 2;

    if (cpudd)
        nTests++;
    if (cpuqd)
        nTests++;
    if (cpu128)
        nTests++;
    if (cpu256)
        nTests++;

    auto& devices = mndContext.getDevices();
    for (size_t i = 0; i < devices.size(); i++) {
        if (mnd::Generator* gpuf; (gpuf = devices[i].getGeneratorFloat())) {
            nTests++;
        }
        if (mnd::Generator* gpud; (gpud = devices[i].getGeneratorDouble())) {
            nTests++;
        }
        /*
        if (mnd::Generator* gpu128; (gpu128 = devices[i].getGeneratorQuad())) {
            nTests++;
        }*/
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

    if (cpudd) {
        cpu.push_back(benchmarkResult(*cpudd));
        br.percentage += progress;
        emit update(br);
    }
    if (cpuqd) {
        cpu.push_back(benchmarkResult(*cpuqd));
        br.percentage += progress;
        emit update(br);
    }
    if (cpu128) {
        cpu.push_back(benchmarkResult(*cpu128));
        br.percentage += progress;
        emit update(br);
    }
    if (cpu256) {
        cpu.push_back(benchmarkResult(*cpu256));
        br.percentage += progress;
        emit update(br);
    }

    for (size_t i = 0; i < devices.size(); i++) {
        br.values.push_back({});
        std::vector<double>& gpu = br.values[br.values.size() - 1];
        if (mnd::Generator* gpuf; (gpuf = devices[i].getGeneratorFloat())) {
            gpu.push_back(benchmarkResult(*gpuf));
            br.percentage += progress;
            emit update(br);
        }
        if (mnd::Generator* gpud; (gpud = devices[i].getGeneratorDouble())) {
            gpu.push_back(benchmarkResult(*gpud));
            br.percentage += progress;
            emit update(br);
        }
        /*if (mnd::Generator* gpu128; (gpu128 = devices[i].getGenerator128())) {
            gpu.push_back(benchmarkResult(*gpu128));
            br.percentage += progress;
            emit update(br);
        }*/
    }
    emit update(br);
    emit finished();
}


BenchmarkDialog::BenchmarkDialog(mnd::MandelContext& mndContext, QWidget *parent) :
    QDialog(parent),
    mndContext{ mndContext },
    benchmarker{ mndContext }
{
    ui.setupUi(this);
    printf("bench!\n"); fflush(stdout);

    auto& devices = mndContext.getDevices();
    size_t nDevices = devices.size() + 1;
    ui.tableWidget->setColumnCount(6);
    ui.tableWidget->setRowCount(int(nDevices));
    ui.tableWidget->setHorizontalHeaderLabels({"Single Precision", "Double Precision", "Double-Double Precision", "Quad-Double Precision", "Quad Precision", "Oct Precision"});

    QString cpuDesc = ("CPU [" + mndContext.getCpuInfo().getBrand() + "]").c_str();
    ui.tableWidget->setVerticalHeaderItem(0, new QTableWidgetItem(cpuDesc));
    for (size_t i = 0; i < devices.size(); i++) {
        std::string cpuDescS = std::string("GPU ") + std::to_string(i + 1) + " [" + devices[i].getName().c_str() + "]";
        QString cpuDesc = QString::fromLatin1(cpuDescS.c_str());
        /*printf("brand [%d]: --> %s <--\n", (int) cpuDescS.size(), cpuDescS.c_str());
        for (int x = 0; x < cpuDescS.size(); x++) {
            printf("%d\n", cpuDescS[x]);
        }
        printf("\n");*/
        auto label = new QTableWidgetItem(cpuDesc);
        label->setStatusTip(QString::fromLatin1(devices[i].getName().c_str()));
        ui.tableWidget->setVerticalHeaderItem(int(i + 1), label);
    }

    qRegisterMetaType<BenchmarkResult>();

    benchmarker.moveToThread(&benchThread);
    connect(&benchThread, &QThread::started, &benchmarker, &Benchmarker::start);
    connect(&benchmarker, SIGNAL (finished()), &benchThread, SLOT (quit()));
    connect(&benchmarker, SIGNAL (update(BenchmarkResult)), this, SLOT (update(BenchmarkResult)));

    ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}


void BenchmarkDialog::update(BenchmarkResult br)
{
    std::vector<double> cpu = br.values[0];
    for (size_t j = 0; j < br.values.size(); j++) {
        for (size_t i = 0; i < br.values[j].size(); i++) {
            ui.tableWidget->setItem(int(j), int(i), new QTableWidgetItem(QString::number(br.values[j][i])));
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
