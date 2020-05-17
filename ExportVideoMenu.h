#ifndef EXPORTVIDEOMENU_H
#define EXPORTVIDEOMENU_H

#include <QWidget>

#include "MandelVideoGenerator.h"

namespace Ui
{
    class ExportVideoMenu;
}

class ExportVideoMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ExportVideoMenu(QWidget *parent = nullptr);
    ~ExportVideoMenu();

    ExportVideoInfo getInfo(void) const;
    void setStartViewport(const mnd::MandelViewport& mv);
    void setEndViewport(const mnd::MandelViewport& mv);
private slots:
    void on_pathBtn_clicked();

private:
    Ui::ExportVideoMenu *ui;
};

#endif // EXPORTVIDEOMENU_H
