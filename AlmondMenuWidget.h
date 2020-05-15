#ifndef ALMONDMENUWIDGET_H
#define ALMONDMENUWIDGET_H

#include <QWidget>
#include <QList>
#include <QStackedWidget>
#include <QPushButton>

class AlmondMenuWidget : public QWidget
{
    Q_OBJECT

    QWidget* mainMenu;
    QWidget* rightWidget;
    QPushButton* rightOK;
    QPushButton* rightCancel;
    QStackedWidget* subMenuContainer;
    QList<QWidget*> subMenus;
public:
    AlmondMenuWidget(QWidget* parent = nullptr);

    void setMainMenu(QWidget* mainMenu);
    void addSubMenu(QWidget* subMenu);

    virtual QSize sizeHint(void) const override;
    virtual QSize minimumSizeHint(void) const override;

protected slots:
    void clickedRightOK(void);
    void clickedRightCancel(void);

public slots:
    void showMainMenu(void);
    void showSubMenu(int index);

signals:
    void submenuOK(int index);
    void submenuCancel(int index);
};

#endif // ALMONDMENUWIDGET_H
