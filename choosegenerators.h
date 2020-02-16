#ifndef CHOOSEGENERATORS_H
#define CHOOSEGENERATORS_H

#include <QDialog>
#include <memory>

namespace Ui
{
    class ChooseGenerators;
}


class ChooseGenerators : public QDialog
{
    Q_OBJECT
private:
    std::unique_ptr<Ui::ChooseGenerators> ui;
public:
    explicit ChooseGenerators(QWidget* parent = nullptr);
    ~ChooseGenerators();


};

#endif // CHOOSEGENERATORS_H

