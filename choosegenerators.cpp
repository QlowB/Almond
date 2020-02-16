#include "choosegenerators.h"
#include "ui_choosegenerators.h"

#include <QComboBox>

ChooseGenerators::ChooseGenerators(QWidget *parent) :
    QDialog{ parent },
    ui{ std::make_unique<Ui::ChooseGenerators>() }
{
    ui->setupUi(this);

    ui->table->insertRow(0);
    ui->table->setCellWidget(0, 1, new QComboBox(ui->table));
}


ChooseGenerators::~ChooseGenerators()
{
}
