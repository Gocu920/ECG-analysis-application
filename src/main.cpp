#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QException>
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <hilberttransform.h>
#include <mainwindow.h>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>

using namespace std;


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setStyle("Fusion");

    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
