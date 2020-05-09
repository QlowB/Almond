#include "Almond.h"
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QScreen>
#include <QSplashScreen>
#include <QStyleFactory>
#include <cmath>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    QSize screenDim = QGuiApplication::screens()[0]->size();
    int splashW = screenDim.width() * 2 / 11;

    QPixmap splashImg(":/splash/splash");
    QPixmap splashScaled = splashImg.scaled(splashW, splashW * splashImg.height() / splashImg.width());
    QSplashScreen splash{ splashScaled };
    a.processEvents();
    splash.show();

    auto list = QStyleFactory::keys();
    for (auto& str : list)
        printf("key: %s\n", str.toStdString().c_str());
    //a.setStyle(QStyleFactory::create("plastique"));
    /*QPalette p = qApp->palette();
    p.setColor(QPalette::Window, QColor(53,53,53));
    p.setColor(QPalette::Button, QColor(53,53,53));
    p.setColor(QPalette::Highlight, QColor(142,45,197));
    p.setColor(QPalette::ButtonText, QColor(255,255,255));
    p.setColor(QPalette::WindowText, QColor(255,255,255));
    qApp->setPalette(p);*/
    Almond w;
    splash.finish(&w);
    a.processEvents();
    w.showMaximized();
    return a.exec();
}

