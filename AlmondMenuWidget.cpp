#include "AlmondMenuWidget.h"
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

AlmondMenuWidget::AlmondMenuWidget(QWidget* parent) :
    QWidget{ parent },
    mainMenu{ nullptr }
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);

    rightWidget = new QWidget(this);
    subMenuContainer = new QStackedWidget(this);
    rightOK = new QPushButton("OK", this);
    rightCancel = new QPushButton("Cancel", this);
    subMenuContainer->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* smlayout = new QVBoxLayout(rightWidget);
    smlayout->addWidget(subMenuContainer, 1);
    smlayout->addWidget(rightOK);
    smlayout->addWidget(rightCancel);

    connect(rightOK, &QPushButton::clicked,
            this, &AlmondMenuWidget::clickedRightOK);
    connect(rightCancel, &QPushButton::clicked,
            this, &AlmondMenuWidget::clickedRightCancel);

    layout->addWidget(rightWidget);
    rightWidget->hide();
}


void AlmondMenuWidget::setMainMenu(QWidget* mainMenu)
{
    //mainMenu->setParent(this);
    this->mainMenu = mainMenu;
    this->layout()->addWidget(mainMenu);
}


void AlmondMenuWidget::addSubMenu(QWidget* subMenu)
{
    subMenuContainer->addWidget(subMenu);
    this->subMenus.append(subMenu);
}


QSize AlmondMenuWidget::sizeHint(void) const
{
    QSize hint{ 0, 0 };
    if (mainMenu)
        hint = mainMenu->sizeHint();
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
    if (mainMenu)
        hint = mainMenu->minimumSizeHint();
    for (auto& widget : subMenus) {
        QSize widgetHint = widget->minimumSizeHint();
        if (hint.width() < widgetHint.width())
            hint.setWidth(widgetHint.width());
        if (hint.height() < widgetHint.height())
            hint.setHeight(widgetHint.height());
    }
    return hint;
}


void AlmondMenuWidget::clickedRightOK(void)
{
    emit submenuOK(subMenuContainer->currentIndex());
}


void AlmondMenuWidget::clickedRightCancel(void)
{
    emit submenuCancel(subMenuContainer->currentIndex());
}


void AlmondMenuWidget::showMainMenu(void)
{
    if (!mainMenu)
        return;
    QParallelAnimationGroup* ag = new QParallelAnimationGroup(this);
    QPropertyAnimation* mm = new QPropertyAnimation(mainMenu, "geometry", this);
    QPropertyAnimation* rw = new QPropertyAnimation(rightWidget, "geometry", this);

    mm->setStartValue(rightWidget->geometry().adjusted(-width(), 0, -width(), 0));
    mm->setEndValue(rightWidget->geometry());

    rw->setEndValue(rightWidget->geometry().adjusted(width(), 0, width(), 0));
    rw->setStartValue(rightWidget->geometry());

    mm->setEasingCurve(QEasingCurve::InOutSine);
    rw->setEasingCurve(QEasingCurve::InOutSine);
    mm->setDuration(200);
    rw->setDuration(200);

    ag->addAnimation(mm);
    ag->addAnimation(rw);

    connect(ag, &QParallelAnimationGroup::finished, [this] () { emit rightWidget->hide(); });

    mainMenu->setGeometry(rightWidget->geometry().adjusted(-width(), 0, -width(), 0));
    emit mainMenu->show();
    emit ag->start();
}


void AlmondMenuWidget::showSubMenu(int index)
{
    if (!mainMenu)
        return;
    QParallelAnimationGroup* ag = new QParallelAnimationGroup(this);
    QPropertyAnimation* mm = new QPropertyAnimation(mainMenu, "geometry", this);
    QPropertyAnimation* rw = new QPropertyAnimation(rightWidget, "geometry", this);

    mm->setStartValue(mainMenu->geometry());
    mm->setEndValue(mainMenu->geometry().adjusted(-width(), 0, -width(), 0));

    rw->setStartValue(mainMenu->geometry().adjusted(width(), 0, width(), 0));
    rw->setEndValue(mainMenu->geometry());

    mm->setEasingCurve(QEasingCurve::InOutSine);
    rw->setEasingCurve(QEasingCurve::InOutSine);
    mm->setDuration(200);
    rw->setDuration(200);

    ag->addAnimation(mm);
    ag->addAnimation(rw);

    connect(ag, &QParallelAnimationGroup::finished, [this] () { mainMenu->hide(); });

    subMenuContainer->setCurrentIndex(index);

    rightWidget->setGeometry(mainMenu->geometry().adjusted(width(), 0, width(), 0));
    emit rightWidget->setVisible(true);
    emit ag->start();
}
