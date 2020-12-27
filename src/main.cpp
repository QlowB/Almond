#include "Almond.h"
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QScreen>
#include <QSplashScreen>
#include <QTranslator>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication a(argc, argv);

    QString translationDir = QDir::currentPath() + "/translations";
    QString translationDir2 = QApplication::applicationDirPath() + "/translations";
    QTranslator lang;
    QTranslator qtLang;
    bool loaded = lang.load(QLocale::system(), QLatin1String("almond"), QLatin1String("."), translationDir);
    qtLang.load(QLocale::system(), QLatin1String("qt"));
    if (!loaded) {
        loaded = lang.load(QLocale::system(), QLatin1String("almond"), QLatin1String("."), translationDir2);
    }

    if (loaded) {
        bool installed = a.installTranslator(&lang);
    }
    a.installTranslator(&qtLang);

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

