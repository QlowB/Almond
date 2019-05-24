#ifndef EXPORTDIALOGS_H
#define EXPORTDIALOGS_H

#include "Mandel.h"

#include <QtWidgets/QDialog>

#include "ui_exportimagedialog.h"
#include "ui_exportvideodialog.h"

class ExportImageDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::ExportImageDialog eid;
public:
    ExportImageDialog(QWidget* parent);

    int getMaxIterations(void) const;
    int getWidth(void) const;
    int getHeight(void) const;
    QString getPath(void) const;
private slots:
    void on_pushButton_clicked();
    void on_buttonBox_accepted();
};


struct ExportVideoInfo {
    mnd::MandelViewport start;
    mnd::MandelViewport end;

    int width;
    int height;
    int maxIterations;

    QString path;
};


class ExportVideoDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::ExportVideoDialog evd;

    ExportVideoInfo evi;
public:
    ExportVideoDialog(QWidget* parent, const ExportVideoInfo& evi);

    const ExportVideoInfo& getExportVideoInfo(void) const;
private slots:
    void on_buttonBox_accepted();
    void on_pushButton_clicked();
};

#endif // EXPORTDIALOGS_H
