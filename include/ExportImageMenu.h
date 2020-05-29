#ifndef EXPORTIMAGEMENU_H
#define EXPORTIMAGEMENU_H

#include <QWidget>

namespace Ui {
class ExportImageMenu;
}

class ExportImageMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ExportImageMenu(QWidget *parent = nullptr);
    ~ExportImageMenu();

    int getMaxIterations(void) const;
    int getWidth(void) const;
    int getHeight(void) const;
    QString getPath(void) const;

private slots:
    void on_pathBtn_clicked();

private:
    Ui::ExportImageMenu *ui;
};

#endif // EXPORTIMAGEMENU_H
