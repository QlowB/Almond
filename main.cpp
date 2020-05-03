#include "Almond.h"
#include <QtWidgets/QApplication>
#include <QPixmap>
#include <QDesktopWidget>
#include <QSplashScreen>
//#include <QTimer>
#include <cmath>

class AlmondSplashScreen : public QSplashScreen
{
private:
    float animOff = 0.0f;
    //QTimer animUpdate;
    volatile bool updated = true;
public:
    AlmondSplashScreen(QPixmap splash) :
        QSplashScreen{ splash }
        //animUpdate{ this }
    {
        //animUpdate.start(10);
        //loading.start();
        //this->add(loading);
        //connect(&loading, &QMovie::updated, this, &AlmondSplashScreen::nextFrame);
        //connect(&animUpdate, &QTimer::timeout, this, &AlmondSplashScreen::nextFrame);
    }

    ~AlmondSplashScreen(void)
    {
        //animUpdate.stop();
    }

    void drawContents(QPainter* painter) override
    {
        QSplashScreen::drawContents(painter);
        //drawAnimation(painter);
        //updated = true;
    }

    void drawAnimation(QPainter* painter)
    {
        const auto minimum = [] (auto a, auto b, auto c) {
            return a < b ? (a < c ? a : c) : (b < c ? b : c);
        };
        int width = this->width();
        int height = this->height();
        int pieces = 7;
        float off = ::fmod(animOff, width / pieces);
        for (int i = 0; i < pieces; i++) {
            float x = off + i * width / pieces;
            float accelOff = 0;

            if (x < 160)
                accelOff = (160 - x) * (160 - x) / 160 ;
            else if (x > width - 160)
                accelOff = -(width - 160 - x) * (width - 160 - x) / 160 ;
            
            x -= accelOff;

            if (x < 0 || x > width)
                continue;

            float opacity = minimum(x, width - x, 130);
            QPen pen(QColor(255, 255, 255, int(opacity)));
            pen.setWidth(4);
            painter->setPen(pen);
            painter->drawEllipse(QRectF{ x, double(height - 40), 16, 16 });
        }
    }

public slots:
    void nextFrame() //(const QRect& rect)
    {
        emit this->repaint();
        //animOff += 3;
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QRect screenDim = a.desktop()->screenGeometry();
    int splashW = screenDim.width() * 2 / 11;

    QPixmap splashImg(":/splash/splash");
    QPixmap splashScaled = splashImg.scaled(splashW, splashW * splashImg.height() / splashImg.width());
    AlmondSplashScreen splash{ splashScaled };
    a.processEvents();
    splash.show();
    a.processEvents();
    /*for (int i = 0; i < 100; i++) {
        a.processEvents();
        system("sleep 0.03");
    }*/
    Almond w;
    a.processEvents();
    splash.finish(&w);
    a.processEvents();
    w.show();
    return a.exec();
}

