#include "AlmondMenuWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

AlmondMenuWidget::AlmondMenuWidget(QWidget* parent) :
    QFrame{ parent },
    mainMenu{ nullptr }
{
    this->setContentsMargins(0, 0, 0, 0);
    rightWidget = new QWidget(this);
    subMenuContainer = new QStackedWidget(rightWidget);
    rightOK = new QPushButton("OK", rightWidget);
    rightCancel = new QPushButton("Cancel", rightWidget);
    subMenuContainer->setContentsMargins(0, 0, 0, 0);
    subMenuContainer->setLayout(new QVBoxLayout());

    leftWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(leftWidget);
    mainLayout->setMargin(0);

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


void AlmondMenuWidget::setMainMenu(QWidget* mainMenu)
{
    //mainMenu->setParent(this);
    this->mainMenu = mainMenu;
    leftWidget->layout()->addWidget(mainMenu);
}


void AlmondMenuWidget::addSubMenu(QWidget* subMenu)
{
    subMenuContainer->addWidget(subMenu);
    this->subMenus.append(subMenu);
}


QSize AlmondMenuWidget::sizeHint(void) const
{
    QSize hint{ 0, 0 };
    hint = leftWidget->sizeHint();
    for (auto& widget : subMenus) {
        QSize widgetHint = widget->sizeHint();
        if (hint.width() < widgetHint.width())
            hint.setWidth(widgetHint.width());
        if (hint.height() < widgetHint.height())
            hint.setHeight(widgetHint.height());
    }
    return hint;
}


QSize AlmondMenuWidget::minimumSizeHint(void) const
{
    QSize hint{ 0, 0 };
    hint = leftWidget->minimumSizeHint();
    for (auto& widget : subMenus) {
        QSize widgetHint = widget->minimumSizeHint();
        if (hint.width() < widgetHint.width())
            hint.setWidth(widgetHint.width());
        if (hint.height() < widgetHint.height())
            hint.setHeight(widgetHint.height());
    }
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
    emit submenuOK(subMenuContainer->currentIndex());
}


void AlmondMenuWidget::clickedRightCancel(void)
{
    emit submenuCancel(subMenuContainer->currentIndex());
    //submenuCancel(0);
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
