/********************************************************************************
** Form generated from reading UI file 'hrv2.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HRV2_H
#define UI_HRV2_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HRV2
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *HRV2)
    {
        if (HRV2->objectName().isEmpty())
            HRV2->setObjectName("HRV2");
        HRV2->resize(800, 600);
        centralwidget = new QWidget(HRV2);
        centralwidget->setObjectName("centralwidget");
        HRV2->setCentralWidget(centralwidget);
        menubar = new QMenuBar(HRV2);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 25));
        HRV2->setMenuBar(menubar);
        statusbar = new QStatusBar(HRV2);
        statusbar->setObjectName("statusbar");
        HRV2->setStatusBar(statusbar);

        retranslateUi(HRV2);

        QMetaObject::connectSlotsByName(HRV2);
    } // setupUi

    void retranslateUi(QMainWindow *HRV2)
    {
        HRV2->setWindowTitle(QCoreApplication::translate("HRV2", "HRV2", nullptr));
    } // retranslateUi

};

namespace Ui {
    class HRV2: public Ui_HRV2 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HRV2_H
