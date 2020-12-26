#include "Almond.h"
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QScreen>
#include <QSplashScreen>
#include <QTranslator>
#include <cmath>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QTranslator lang;
    bool loaded = lang.load(QLocale::system(), QStringLiteral("qtbase_"));
    lang.load("almond_de-DE");
    printf("loaded %d\n", loaded);
    QApplication a(argc, argv);
    if (loaded)
        a.installTranslator(&lang);

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

