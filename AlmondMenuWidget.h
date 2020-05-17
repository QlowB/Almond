#ifndef ALMONDMENUWIDGET_H
#define ALMONDMENUWIDGET_H

#include <QFrame>
#include <QList>
#include <QStackedWidget>
#include <QPushButton>
#include <QStateMachine>

class AlmondSubMenu :
    public QObject
{
    Q_OBJECT

    QWidget* w;
public:
    AlmondSubMenu(QWidget* widget);
    QWidget* widget(void);

signals:
    void accepted(void);
    void cancelled(void);
};

class AlmondMenuWidget :
    public QFrame
{
    Q_OBJECT
        
    QWidget* mainMenu;
    QWidget* rightWidget;
    QWidget* leftWidget;
    QPushButton* rightOK;
    QPushButton* rightCancel;
    QStackedWidget* subMenuContainer;
    QList<AlmondSubMenu*> subMenus;
    QStateMachine* states;
public:
    AlmondMenuWidget(QWidget* parent = nullptr);
    ~AlmondMenuWidget(void);

    void setMainMenu(QWidget* mainMenu);
    AlmondSubMenu* addSubMenu(QWidget* subMenu);

    virtual QSize sizeHint(void) const override;
    virtual QSize minimumSizeHint(void) const override;
protected:
    virtual void resizeEvent(QResizeEvent* event) override;

protected slots:
    void clickedRightOK(void);
    void clickedRightCancel(void);

public slots:
    void showMainMenu(void);
    void showSubMenu(int index);
    void showSubMenu(void);

signals:
    void submenuOK(int index);
    void submenuCancel(int index);

    void showLeft();
    void showRight();
};

#endif // ALMONDMENUWIDGET_H
