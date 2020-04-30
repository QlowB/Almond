#ifndef EXPORTDIALOGS_H
#define EXPORTDIALOGS_H

#include "Mandel.h"

#include <QtWidgets/QDialog>

#include "ui_exportimagedialog.h"
#include "ui_exportvideodialog.h"

#include "MandelVideoGenerator.h"

class Almond;

class ExportImageDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::ExportImageDialog eid;
public:
    ExportImageDialog(QWidget* parent);

    void setMaxIterations(int mi);
    int getMaxIterations(void) const;
    int getWidth(void) const;
    int getHeight(void) const;
    QString getPath(void) const;
private slots:
    void on_pushButton_clicked();
    void on_buttonBox_accepted();
};


class ExportVideoDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::ExportVideoDialog evd;

    ExportVideoInfo evi;
    Almond* almond;
public:
    ExportVideoDialog(Almond* parent, const ExportVideoInfo& evi);

    const ExportVideoInfo& getExportVideoInfo(void) const;
private slots:
    void on_buttonBox_accepted();
    void on_pushButton_clicked();
};

#endif // EXPORTDIALOGS_H
