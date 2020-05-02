#include "Almond.h"
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QDesktopWidget>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QRect screenDim = a.desktop()->screenGeometry();
    int splashW = screenDim.width() * 2 / 11;

    QPixmap splashImg(":/splash/splash");
    QPixmap splashScaled = splashImg.scaled(splashW, splashW * splashImg.height() / splashImg.width());
    QSplashScreen splash{ splashScaled };
    splash.show();
    a.processEvents();

    Almond w;
    w.show();
    splash.finish(&w);
    return a.exec();
}

