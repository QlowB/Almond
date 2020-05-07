#include "Almond.h"
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QScreen>
#include <QSplashScreen>
#include <cmath>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSize screenDim = QGuiApplication::screens()[0]->size();
    int splashW = screenDim.width() * 2 / 11;

    QPixmap splashImg(":/splash/splash");
    QPixmap splashScaled = splashImg.scaled(splashW, splashW * splashImg.height() / splashImg.width());
    QSplashScreen splash{ splashScaled };
    a.processEvents();
    splash.show();
    Almond w;
    splash.finish(&w);
    a.processEvents();
    w.showMaximized();
    return a.exec();
}

