#include "Almond.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Almond w;
    w.show();
    return a.exec();
}
