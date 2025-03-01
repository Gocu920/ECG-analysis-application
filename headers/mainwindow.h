#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ecg_data.h"
#include <QComboBox>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // na przyszłość bo może trrzeba będzie z tego skorzystać do raportów xd

    // float sampling_freq = 360.0;

    // QComboBox* algorithmBox;

    // QVector<double> filteredData;
    // QVector<double> channelData;
    // QVector<int> peaksResult;

    // QVector<double> hrv1_timeParams;
    // QVector<double> hrv1_freqParams;

    // QList<double> hrv2_params;
    // QPair<QVector<double>, QVector<double>> hrv2_poincare;
    // QPair<QVector<double>, QVector<double>> hrv2_histogram;

    // QPair<double, double> hrv_dfa_alphas;

    QVector<QString> heartClassifications;
    bool isFilterApplied = false;

private slots:
    void on_actionWczytaj_plik_triggered();
    void on_actionWy_wietl_EKG_triggered();
    void on_actionFiltracja_triggered();
    void on_actionAnaliza_triggered();
    void on_actionRaport_triggered();
    void on_actionWyjd_triggered();
 //   void hilberttompkins(bool pantompkinsRadio,bool hilbertRadio);

    void on_actionJak_korzysta_z_aplikacji_triggered();

    // void onXAxisRangeChanged(const QCPRange &newRange);


private:
    Ui::MainWindow *ui;
    ECG_Data ecgData;
    QCustomPlot *customPlot;

    QWidget* create_HRV1Page();
    QWidget* create_HRV2Page();
    QWidget* create_HRVDFAPage();
    QWidget* create_HeartClassPage();

    double globalYMin = std::numeric_limits<double>::max();
    double globalYMax = std::numeric_limits<double>::lowest();

};

#endif // MAINWINDOW_H
