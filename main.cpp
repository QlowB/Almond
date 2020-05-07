#include "Almond.h"
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QDesktopWidget>
#include <QSplashScreen>
#include <cmath>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QRect screenDim = a.desktop()->screenGeometry();
    int splashW = screenDim.width() * 2 / 11;

    QPixmap splashImg(":/splash/splash");
    QPixmap splashScaled = splashImg.scaled(splashW, splashW * splashImg.height() / splashImg.width());
    QSplashScreen splash{ splashScaled };
    a.processEvents();
    splash.show();
    a.processEvents();
    Almond w;
    a.processEvents();
    splash.finish(&w);
    a.processEvents();
    w.showMaximized();
    return a.exec();
}

