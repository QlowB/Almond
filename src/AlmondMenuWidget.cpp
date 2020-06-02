#include "AlmondMenuWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

AlmondSubMenu::AlmondSubMenu(QWidget* widget) :
    w{ widget }
{
}


QWidget* AlmondSubMenu::widget(void)
{
    return w;
}


AlmondMenuWidget::AlmondMenuWidget(QWidget* parent) :
    QFrame{ parent },
    mainMenu{ nullptr }
{
    rightWidget = new QWidget(this);
    subMenuContainer = new QStackedWidget(rightWidget);
    subMenuContainer->setContentsMargins(0, 0, 0, 0);
    rightOK = new QPushButton("OK", rightWidget);
    rightCancel = new QPushButton("Cancel", rightWidget);

    leftWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(leftWidget);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* smlayout = new QVBoxLayout(rightWidget);
    smlayout->addWidget(subMenuContainer, 1);
    smlayout->addWidget(rightOK);
    smlayout->addWidget(rightCancel);

    connect(rightOK, &QPushButton::clicked,
            this, &AlmondMenuWidget::clickedRightOK);
    connect(rightCancel, &QPushButton::clicked,
            this, &AlmondMenuWidget::clickedRightCancel);

    //layout->addWidget(rightWidget);
    //layout->addWidget(leftWidget);
    rightWidget->setVisible(false);

    /*states = new QStateMachine(this);
    QState* mainMenuShow = new QState(states);
    QState* subMenuShow = new QState(states);

    mainMenuShow->assignProperty(leftWidget, "visible", true);
    mainMenuShow->assignProperty(rightWidget, "visible", false);

    subMenuShow->assignProperty(leftWidget, "visible", false);
    subMenuShow->assignProperty(leftWidget, "pos", QPoint(100, 100));
    subMenuShow->assignProperty(rightWidget, "visible", true);

    mainMenuShow->addTransition(this, SIGNAL(showLeft()), subMenuShow);
    subMenuShow->addTransition(this, SIGNAL(showRight()), mainMenuShow);
    states->start();*/
}


AlmondMenuWidget::~AlmondMenuWidget(void)
{
    for (auto& a : subMenus) {
        delete a;
    }
}


void AlmondMenuWidget::setMainMenu(QWidget* mainMenu)
{
    //mainMenu->setParent(this);
    this->mainMenu = mainMenu;
    leftWidget->layout()->addWidget(mainMenu);
}


AlmondSubMenu* AlmondMenuWidget::addSubMenu(QWidget* subMenu)
{
    subMenuContainer->addWidget(subMenu);
    AlmondSubMenu* almsm = new AlmondSubMenu(subMenu);
    this->subMenus.append(almsm);
    return almsm;
}


QSize AlmondMenuWidget::sizeHint(void) const
{
    QSize hint{ 0, 0 };
    hint = leftWidget->sizeHint();
    for (auto& subMenu : subMenus) {
        const auto& widget = subMenu->widget();
        QSize widgetHint = widget->sizeHint();
        if (hint.width() < widgetHint.width())
            hint.setWidth(widgetHint.width());
        if (hint.height() < widgetHint.height())
            hint.setHeight(widgetHint.height());
    }
    QMargins m = contentsMargins();
    m += subMenuContainer->contentsMargins();
    hint.setWidth(hint.width() + m.left() + m.right());
    hint.setHeight(hint.height() + m.top() + m.bottom());
    return hint;
}


QSize AlmondMenuWidget::minimumSizeHint(void) const
{
    QSize hint{ 0, 0 };
    hint = leftWidget->minimumSizeHint();
    for (auto& subMenu : subMenus) {
        const auto& widget = subMenu->widget();
        QSize widgetHint = widget->minimumSizeHint();
        if (hint.width() < widgetHint.width())
            hint.setWidth(widgetHint.width());
        if (hint.height() < widgetHint.height())
            hint.setHeight(widgetHint.height());
    }
    QMargins m = contentsMargins();
    m += subMenuContainer->contentsMargins();
    hint.setWidth(hint.width() + m.left() + m.right());
    hint.setHeight(hint.height() + m.top() + m.bottom());
    return hint;
}


void AlmondMenuWidget::resizeEvent(QResizeEvent* event)
{
    QRect leftGeom = contentsRect();
    QRect rightGeom = contentsRect();
    //leftGeom.setSize(event->size());
    //rightGeom.setSize(event->size());
    leftWidget->setGeometry(leftGeom);
    rightWidget->setGeometry(rightGeom);
}


void AlmondMenuWidget::clickedRightOK(void)
{
    int index = subMenuContainer->currentIndex();
    emit submenuOK(index);
    emit subMenus.at(index)->accepted();
}


void AlmondMenuWidget::clickedRightCancel(void)
{
    int index = subMenuContainer->currentIndex();
    emit submenuCancel(subMenuContainer->currentIndex());
    emit subMenus.at(index)->cancelled();
}


void AlmondMenuWidget::showMainMenu(void)
{
    emit showLeft();
    if (!mainMenu)
        return;
    QParallelAnimationGroup* ag = new QParallelAnimationGroup(this);
    QPropertyAnimation* mm = new QPropertyAnimation(leftWidget, "geometry", this);
    QPropertyAnimation* rw = new QPropertyAnimation(rightWidget, "geometry", this);

    mm->setStartValue(contentsRect().adjusted(-width(), 0, -width(), 0));
    mm->setEndValue(contentsRect());

    rw->setEndValue(contentsRect().adjusted(width(), 0, width(), 0));
    rw->setStartValue(contentsRect());

    mm->setEasingCurve(QEasingCurve::InOutSine);
    rw->setEasingCurve(QEasingCurve::InOutSine);
    mm->setDuration(200);
    rw->setDuration(200);

    ag->addAnimation(mm);
    ag->addAnimation(rw);

    connect(ag, &QParallelAnimationGroup::finished, [this] () {
        emit rightWidget->hide();
        leftWidget->setGeometry(contentsRect());
    });

    leftWidget->setGeometry(rightWidget->geometry().adjusted(-width(), 0, -width(), 0));
    emit leftWidget->show();
    emit ag->start(QAbstractAnimation::DeleteWhenStopped);
}


void AlmondMenuWidget::showSubMenu(int index)
{
    subMenuContainer->setCurrentIndex(index);
    /*while (subMenuContainer->layout()->count() > 0)
        subMenuContainer->layout()->takeAt(0);
    subMenuContainer->layout()->update();
    rightWidget->setGeometry(leftWidget->geometry().adjusted(width(), 0, width(), 0));
    //subMenuContainer->layout()->addWidget(subMenus.at(index));
    subMenuContainer->layout()->update();*/

    showSubMenu();
}


void AlmondMenuWidget::showSubMenu(void)
{
    emit showRight();
    if (!mainMenu)
    return;
    QParallelAnimationGroup* ag = new QParallelAnimationGroup(this);
    QPropertyAnimation* mm = new QPropertyAnimation(leftWidget, "geometry", this);
    QPropertyAnimation* rw = new QPropertyAnimation(rightWidget, "geometry", this);

    mm->setStartValue(contentsRect());
    mm->setEndValue(contentsRect().adjusted(-width(), 0, -width(), 0));

    rw->setStartValue(contentsRect().adjusted(width(), 0, width(), 0));
    rw->setEndValue(contentsRect());

    mm->setEasingCurve(QEasingCurve::InOutSine);
    rw->setEasingCurve(QEasingCurve::InOutSine);
    mm->setDuration(200);
    rw->setDuration(200);

    ag->addAnimation(mm);
    ag->addAnimation(rw);

    connect(ag, &QParallelAnimationGroup::finished, [this] () {
        leftWidget->hide();
        rightWidget->setGeometry(contentsRect());
    });

    rightWidget->setGeometry(mainMenu->geometry().adjusted(width(), 0, width(), 0));
    emit rightWidget->setVisible(true);
    emit ag->start(QAbstractAnimation::DeleteWhenStopped);
}
