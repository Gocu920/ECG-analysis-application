/********************************************************************************
** Form generated from reading UI file 'ecg_data.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ECG_DATA_H
#define UI_ECG_DATA_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ECG_Data
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *ECG_Data)
    {
        if (ECG_Data->objectName().isEmpty())
            ECG_Data->setObjectName("ECG_Data");
        ECG_Data->resize(800, 600);
        centralwidget = new QWidget(ECG_Data);
        centralwidget->setObjectName("centralwidget");
        ECG_Data->setCentralWidget(centralwidget);
        menubar = new QMenuBar(ECG_Data);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 26));
        ECG_Data->setMenuBar(menubar);
        statusbar = new QStatusBar(ECG_Data);
        statusbar->setObjectName("statusbar");
        ECG_Data->setStatusBar(statusbar);

        retranslateUi(ECG_Data);

        QMetaObject::connectSlotsByName(ECG_Data);
    } // setupUi

    void retranslateUi(QMainWindow *ECG_Data)
    {
        ECG_Data->setWindowTitle(QCoreApplication::translate("ECG_Data", "ECG_Data", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ECG_Data: public Ui_ECG_Data {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ECG_DATA_H
