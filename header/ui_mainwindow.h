/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionWczytaj_plik;
    QAction *actionWy_wietl_EKG;
    QAction *actionFiltracja;
    QAction *actionAnaliza;
    QAction *actionRaport;
    QAction *actionWyjd;
    QAction *actionJak_korzysta_z_aplikacji;
    QWidget *centralwidget;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QPushButton *pushButtonWczytajPlik;
    QTextEdit *textEdit;
    QStackedWidget *stackedWidget;
    QWidget *page_3;
    QWidget *page_4;
    QToolBar *toolBar_2;
    QToolBar *toolBar;
    QMenuBar *menubar;
    QMenu *menuMenu;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(791, 593);
        actionWczytaj_plik = new QAction(MainWindow);
        actionWczytaj_plik->setObjectName("actionWczytaj_plik");
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/health-insurance-icon.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionWczytaj_plik->setIcon(icon);
        actionWy_wietl_EKG = new QAction(MainWindow);
        actionWy_wietl_EKG->setObjectName("actionWy_wietl_EKG");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/ecg-monitoring-icon.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionWy_wietl_EKG->setIcon(icon1);
        actionFiltracja = new QAction(MainWindow);
        actionFiltracja->setObjectName("actionFiltracja");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/sliders-icon.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionFiltracja->setIcon(icon2);
        actionAnaliza = new QAction(MainWindow);
        actionAnaliza->setObjectName("actionAnaliza");
        actionAnaliza->setCheckable(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/market-research-icon.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionAnaliza->setIcon(icon3);
        actionRaport = new QAction(MainWindow);
        actionRaport->setObjectName("actionRaport");
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/diagnostic-icon.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionRaport->setIcon(icon4);
        actionWyjd = new QAction(MainWindow);
        actionWyjd->setObjectName("actionWyjd");
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/close-round-icon.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionWyjd->setIcon(icon5);
        actionJak_korzysta_z_aplikacji = new QAction(MainWindow);
        actionJak_korzysta_z_aplikacji->setObjectName("actionJak_korzysta_z_aplikacji");
        QIcon icon6(QIcon::fromTheme(QIcon::ThemeIcon::HelpFaq));
        actionJak_korzysta_z_aplikacji->setIcon(icon6);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayoutWidget = new QWidget(centralwidget);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(90, 120, 451, 311));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(verticalLayoutWidget);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        pushButtonWczytajPlik = new QPushButton(verticalLayoutWidget);
        pushButtonWczytajPlik->setObjectName("pushButtonWczytajPlik");

        verticalLayout->addWidget(pushButtonWczytajPlik);

        textEdit = new QTextEdit(verticalLayoutWidget);
        textEdit->setObjectName("textEdit");

        verticalLayout->addWidget(textEdit);

        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(240, 30, 120, 80));
        page_3 = new QWidget();
        page_3->setObjectName("page_3");
        stackedWidget->addWidget(page_3);
        page_4 = new QWidget();
        page_4->setObjectName("page_4");
        stackedWidget->addWidget(page_4);
        MainWindow->setCentralWidget(centralwidget);
        toolBar_2 = new QToolBar(MainWindow);
        toolBar_2->setObjectName("toolBar_2");
        toolBar_2->setMinimumSize(QSize(0, 0));
        toolBar_2->setIconSize(QSize(90, 70));
        toolBar_2->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
        MainWindow->addToolBar(Qt::ToolBarArea::LeftToolBarArea, toolBar_2);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName("toolBar");
        toolBar->setMovable(false);
        MainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 791, 26));
        menuMenu = new QMenu(menubar);
        menuMenu->setObjectName("menuMenu");
        MainWindow->setMenuBar(menubar);

        toolBar_2->addAction(actionWczytaj_plik);
        toolBar_2->addSeparator();
        toolBar_2->addAction(actionWy_wietl_EKG);
        toolBar_2->addSeparator();
        toolBar_2->addAction(actionFiltracja);
        toolBar_2->addSeparator();
        toolBar_2->addAction(actionAnaliza);
        toolBar_2->addSeparator();
        toolBar_2->addAction(actionRaport);
        toolBar_2->addSeparator();
        menubar->addAction(menuMenu->menuAction());
        menuMenu->addAction(actionJak_korzysta_z_aplikacji);
        menuMenu->addAction(actionWyjd);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionWczytaj_plik->setText(QCoreApplication::translate("MainWindow", "Wczytaj plik", nullptr));
        actionWy_wietl_EKG->setText(QCoreApplication::translate("MainWindow", "Wy\305\233wietl EKG", nullptr));
        actionFiltracja->setText(QCoreApplication::translate("MainWindow", "Filtracja", nullptr));
        actionAnaliza->setText(QCoreApplication::translate("MainWindow", "Analiza", nullptr));
        actionRaport->setText(QCoreApplication::translate("MainWindow", "Raport", nullptr));
        actionWyjd->setText(QCoreApplication::translate("MainWindow", "Wyjd\305\272", nullptr));
        actionJak_korzysta_z_aplikacji->setText(QCoreApplication::translate("MainWindow", "Jak korzysta\304\207 z aplikacji?", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        pushButtonWczytajPlik->setText(QCoreApplication::translate("MainWindow", "PushButton", nullptr));
        toolBar_2->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar_2", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
        menuMenu->setTitle(QCoreApplication::translate("MainWindow", "Menu", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
