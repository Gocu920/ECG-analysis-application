#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ecg_data.h"
#include <QFileDialog>  // Do otwierania okna wyboru pliku
#include <QFile>        // Do pracy z plikami
#include <QTextStream>  // Do czytania tekstu z pliku
#include <QMessageBox> // Do wyskakujących okien
#include <QInputDialog> //Do wprowadzania inputu użytkownika
#include "filter.h"
#include <QMap>
#include <HilbertTransform.h>
#include "pantompkins.h"
#include "butterworth.h"
#include "movingaverage.h"
#include "savitzkygolay.h"
#include "lms_filter.h"
#include "hrv1.h"
#include "hrv2.h"
#include "hrv_dfa.h"
#include "waves.h"
#include "heart_class.h"
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QComboBox>
#include <QBarSeries>
#include <QBarSet>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include "qcustomplot.h" // do wizualizacji
#include <QtCharts/QBarCategoryAxis>
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QSet>
#include<QDebug>

QComboBox *algorithmBox;
QVector<int> peaksResult;
QString selectedAlgorithm; // tylko do wyswietlania
QString selectedFilter;

QVector<double> channelData;
QVector<double> filteredData;
float sampling_freq = 360.0; // zahardkodowana wartosc sampling rate

QVector <double> timeparam;
QVector <double> freqparam;
QVector <double> result_freq;
QVector <double> result;
int method_counter=0;
bool choose_canal;
// RAPORT
struct RaportInfo {
    QString moduleName;
    QStringList parameters; // Lista parametrów
    QPixmap chartPixmap;
    QPixmap chartPixmap1;
};
QVector<RaportInfo> raport_vector;

// ekran powitalny

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,ecgData(sampling_freq)
{
    qDebug() << "Czy ikona istnieje?" << QFile::exists(":/images/close-round-icon.png");
    ui->setupUi(this);
    ekranPowitalny();
    QAction *toggleSidebarAction = new QAction( this);
    QIcon icon(":/images/list-round-bullet-icon2.png");
    toggleSidebarAction->setIcon(icon);
    connect(toggleSidebarAction, &QAction::triggered, this, &MainWindow::toggleSidebar);
    QAction *existingAction = ui->menubar->actions().at(0);
    ui->menubar->insertAction(existingAction, toggleSidebarAction);

    QAction *welcomeScreenAction = new QAction(QIcon(":/images/home-page-white-icon.png"), "Ekran powitalny", this);
    connect(welcomeScreenAction, &QAction::triggered, this, &MainWindow::ekranPowitalny);

    QAction *existingAction3 = ui->menubar->actions().at(1);
    ui->menubar->insertAction(existingAction3, welcomeScreenAction);

    this->setWindowIcon(QIcon(":/images/logo.png"));
    this->setWindowTitle("ECGuider");
    applyStyle();

}

MainWindow::~MainWindow()
{
    delete ui;
    raport_vector.clear();
}
void MainWindow::on_actionWczytaj_plik_triggered()
{
    QString fileName;
    channelData.clear();
    fileName = QFileDialog::getOpenFileName(nullptr, "Wybierz plik .dat", "", "Pliki .dat (*.dat)");
    if (!fileName.isEmpty()) {
        // otwarcie pliku jeżeli został jakiś wybrany
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            file.close();
            QMessageBox::information(this, "Wczytano", "Plik poprawnie wczytano! :) Przejdź do kolejnych kroków");
            qDebug()<<"first";
            ;
            qDebug()<<"second";

            // Zerowanie globalnych wartości Y po załadowaniu nowego pliku
            globalYMin = std::numeric_limits<double>::max();
            globalYMax = std::numeric_limits<double>::lowest();

            bool ok = false;
            QString selectedItem;
            QStringList canals = {"Kanał 1", "Kanał 2"};

            while (!ok) {
                selectedItem = QInputDialog::getItem(this, tr("Wybierz kanał"),
                                                     tr("Dostępne kanały:"),
                                                     canals, 0, false, &ok);
                if (!ok) {
                    QMessageBox::warning(this, "Błąd", "Nie wybrano kanału. Spróbuj ponownie.");
                }
            }

            // Przetwarzanie danych na podstawie wybranego kanału
            if (selectedItem == "Kanał 1") {
                choose_canal=true;
            } else if (selectedItem == "Kanał 2") {

                choose_canal=false;
            }
            ecgData.loadData(fileName);
            qDebug()<<"Choose canal"<<choose_canal;
            channelData = ecgData.getRawCh(choose_canal);
        } else {
            QMessageBox::warning(this, "Błąd", "Nie udało się otworzyć pliku. Spróbuj ponownie.");
        }
    } else {
        QMessageBox::information(this, "Błąd", "Nie wybrano pliku. Spróbuj ponownie.");
    }

}


void MainWindow::on_actionWy_wietl_EKG_triggered()
{

    if (channelData.isEmpty()) {
        QMessageBox::warning(this, "Brak danych", "Dane EKG nie zostały wczytane. Wczytaj plik przed wyświetleniem wykresu.");
        return;
    }
    QWidget *newWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(newWidget);

    QCustomPlot *customPlot = new QCustomPlot(newWidget);

    customPlot->setOpenGl(true);
    customPlot->setAntialiasedElements(QCP::aeNone);
    customPlot->setNotAntialiasedElements(QCP::aeAll);
    customPlot->setMinimumSize(600,300);

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal);
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    customPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->graph(0)->setName("Surowy sygnał");
    customPlot->xAxis->setLabel("Czas [s]");
    customPlot->yAxis->setLabel("Amplituda [mV]");

    // Wartości globalne dla osi Y
    double globalYMin = std::numeric_limits<double>::max();
    double globalYMax = std::numeric_limits<double>::lowest();

    // Obliczenie min i max wartości na osi Y
    double yMin = *std::min_element(channelData.begin(), channelData.end());
    double yMax = *std::max_element(channelData.begin(), channelData.end());
    globalYMin = std::min(globalYMin, yMin);
    globalYMax = std::max(globalYMax, yMax);



    QVector<double> xData, yData;
    int initialLimit = qMin(1000, channelData.size());
    for (int i = 0; i < initialLimit; ++i) {
        xData.append(i / sampling_freq);
        yData.append(channelData[i]);
    }

    customPlot->graph(0)->setData(xData, yData);
    customPlot->rescaleAxes();
    customPlot->xAxis->setRange(0, initialLimit / sampling_freq);
    customPlot->yAxis->setRange(globalYMin, globalYMax);

    // Obsługa zmiany zakresu osi X z downsamplingiem
    connect(customPlot->xAxis,
            static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged),
            this,
            [this, customPlot, globalYMin, globalYMax](const QCPRange &newRange) {
                QCPRange adjustedRange = newRange;
                if (adjustedRange.lower < 0) {
                    adjustedRange.lower = 0;
                }

                customPlot->xAxis->setRange(adjustedRange);


                int start = static_cast<int>(adjustedRange.lower * sampling_freq);
                int end = static_cast<int>(adjustedRange.upper * sampling_freq);

                start = qMax(0, start);
                end = qMin(channelData.size(), end);

                QVector<double> xData, yData;
                // int step = qMax(1, (end - start));

                for (int i = start; i < end; i++) {
                    xData.append(i / sampling_freq);
                    yData.append(channelData[i]);
                }

                customPlot->graph(0)->setData(xData, yData);
                customPlot->yAxis->setRange(globalYMin, globalYMax);
                customPlot->replot();
            });

    customPlot->legend->setVisible(true);
    customPlot->legend->setFont(QFont("Arial", 10));
    customPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 150)));


    layout->setContentsMargins(20, 10, 20, 10);
    layout->addWidget(customPlot, 1, 0, 5, 2);

    newWidget->setLayout(layout);

    setCentralWidget(newWidget);

    // Dodanie do raportu
    RaportInfo obj;
    obj.moduleName = "Surowe EKG";
    obj.chartPixmap = customPlot->toPixmap(800, 600);
    raport_vector.append(obj);
}


void MainWindow::on_actionFiltracja_triggered()
{
    if(channelData.isEmpty()){
        QMessageBox::information(this, "Błąd", "Brak danych do przefiltrowania. Wczytaj dane zanim przystąpisz do obróbki sygnału.");
        return;
    }
    QWidget *filterWidget = new QWidget(this);
    filterWidget->setContentsMargins(0, 0, 0, 0);

    QGridLayout *layout = new QGridLayout(filterWidget);
    layout->setContentsMargins(20, 10, 20, 10);
    layout->setSpacing(10);

    QLabel *currentFilter = new QLabel("Aktualny filtr: brak", filterWidget);
    currentFilter->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    QFont font;
    font.setPointSize(12);
    currentFilter->setFont(font);
    layout->addWidget(currentFilter, 0, 0, 1, 1);

    customPlot = new QCustomPlot(filterWidget);
    customPlot->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");
    customPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    customPlot->addGraph(); // Dodanie grafu
    customPlot->xAxis->grid()->setVisible(true);
    customPlot->yAxis->grid()->setVisible(true);
    customPlot->xAxis->grid()->setPen(QPen(Qt::gray, 0.5, Qt::DashLine));
    customPlot->yAxis->grid()->setPen(QPen(Qt::gray, 0.5, Qt::DashLine));
    customPlot->xAxis->setLabel("Czas [s]");
    customPlot->yAxis->setLabel("Amplituda [mV]");


    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal| Qt::Vertical);
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);

    layout->addWidget(customPlot, 1, 0, 5, 2);

    QWidget *controlsWidget = new QWidget(filterWidget);
    QHBoxLayout *controlsLayout = new QHBoxLayout(controlsWidget);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->setSpacing(10);

    QComboBox *filtersBox = new QComboBox(controlsWidget);
    filtersBox->addItems({"Brak filtracji", "Średnia krocząca", "Filtr Butterwortha", "Filtr Savitzky-Golay", "Filtr LMS"});
    filtersBox->setFixedWidth(200);
    controlsLayout->addWidget(filtersBox);

    QPushButton *applyButton = new QPushButton("Zastosuj", controlsWidget);
    applyButton->setFixedWidth(100);
    controlsLayout->addWidget(applyButton);

    layout->addWidget(controlsWidget, 6, 0, 1, 2, Qt::AlignCenter);

    setCentralWidget(filterWidget);

    QVector<double> xData, yData;

    if (!filteredData.isEmpty()) {
        for (int i = 0; i < qMin(1000, filteredData.size()); ++i) {
            xData.append(i / sampling_freq);
            yData.append(filteredData[i]);
        }
        currentFilter->setText("Aktualny filtr: " + selectedFilter);
    } else {
        channelData = ecgData.getRawCh(choose_canal);
        for (int i = 0; i < qMin(1000, channelData.size()); ++i) {
            xData.append(i / sampling_freq);
            yData.append(channelData[i]);
        }
    }

    double yMin = *std::min_element(yData.begin(), yData.end());
    double yMax = *std::max_element(yData.begin(), yData.end());
    globalYMin = std::min(globalYMin, yMin);
    globalYMax = std::max(globalYMax, yMax);

    customPlot->graph(0)->setData(xData, yData);
    customPlot->yAxis->setRange(globalYMin, globalYMax);
    customPlot->rescaleAxes();
    customPlot->replot();

    connect(applyButton, &QPushButton::clicked, this, [this, filtersBox, currentFilter, xData, yData]() {
        QString selectedItem = filtersBox->currentText();
        QScopedPointer<Filter> filter;

        channelData = ecgData.getRawCh(choose_canal);

        if (selectedItem == "Średnia krocząca") {
            filter.reset(new MovingAverage);
        } else if (selectedItem == "Filtr Butterwortha") {
            filter.reset(new Butterworth);
        } else if (selectedItem == "Filtr Savitzky-Golay") {
            filter.reset(new SavitzkyGolay);
        } else if (selectedItem == "Filtr LMS") {
            filter.reset(new LMS_filter);
        }

        selectedFilter = selectedItem;

        currentFilter->setText("Aktualny filtr: " + selectedItem);

        if (filter) {
            filteredData = filter->applyFilter(channelData);
            method_counter=0;
            qDebug() << "Zastosowano filtr: " << selectedItem;
        } else {
            filteredData = channelData;
            method_counter=0;
            qDebug() << "Brak filtracji.";
        }

        QVector<double> xData, yData;
        int limit = qMin(1000, filteredData.size());
        for (int i = 0; i < limit; ++i) {
            xData.append(i / sampling_freq);
            yData.append(filteredData[i]);
        }

        double yMin = *std::min_element(yData.begin(), yData.end());
        double yMax = *std::max_element(yData.begin(), yData.end());
        globalYMin = std::min(globalYMin, yMin);
        globalYMax = std::max(globalYMax, yMax);


        QCPRange xRange = customPlot->xAxis->range();
        if (xRange.lower < 0) {
            xRange.lower = 0;
            customPlot->xAxis->setRange(xRange);
        }

        customPlot->rescaleAxes();
        customPlot->yAxis->setRange(globalYMin, globalYMax);
        customPlot->graph(0)->setData(xData, yData);

        connect(customPlot->xAxis,
                static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged),
                this,
                [this](const QCPRange &newRange) {
                    QCPRange adjustedRange = newRange;
                    if (adjustedRange.lower < 0) {
                        adjustedRange.lower = 0;
                    }
                    customPlot->xAxis->setRange(adjustedRange);

                    int start = static_cast<int>(adjustedRange.lower * sampling_freq);
                    int end = static_cast<int>(adjustedRange.upper * sampling_freq);

                    start = qMax(0, start);
                    end = qMin(filteredData.size(), end);

                    QVector<double> xData, yData;

                    for (int i = start; i < end; ++i) {
                        xData.append(i / sampling_freq);
                        yData.append(filteredData[i]);
                    }

                    customPlot->graph(0)->setData(xData, yData);
                    customPlot->yAxis->setRange(globalYMin, globalYMax);
                    customPlot->replot();
                });


        customPlot->rescaleAxes();
        customPlot->yAxis->setRange(globalYMin, globalYMax);
        customPlot->replot(); // Aktualizacja wykresu
        //RAPORT
        RaportInfo obj;
        obj.moduleName = "Rodzaj filtracji: " + selectedItem;
        obj.chartPixmap = customPlot->toPixmap(800, 600);
        raport_vector.append(obj);

    });

    isFilterApplied = true;
}


void MainWindow::on_actionAnaliza_triggered()
{
    QWidget *analysis = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(analysis);


    QHBoxLayout *boxLayout = new QHBoxLayout();
    QLabel *algorithmLabel = new QLabel("Wybierz algorytm wykrywania załamków R w sygnale EKG:", this);

    algorithmBox = new QComboBox(this);

    algorithmBox->addItem("Pan-Tompkins");
    algorithmBox->addItem("Transformata Hilberta");
    algorithmBox->setCurrentIndex(0);

    boxLayout->addWidget(algorithmLabel);        // Zakładki
    boxLayout->addWidget(algorithmBox); // Napis
    boxLayout->addStretch();

    QTabWidget *pagesTab = new QTabWidget(this);

    QWidget *hrv1Page = create_HRV1Page();
    pagesTab->addTab(hrv1Page, "HRV 1");


    QWidget *hrv2Page = create_HRV2Page();
    pagesTab->addTab(hrv2Page, "HRV 2");


    QWidget *dfaPage = create_HRVDFAPage();
    pagesTab->addTab(dfaPage, "HRV DFA");


    QWidget *heartclassPage = create_HeartClassPage();
    pagesTab->addTab(heartclassPage, "Heart Class");

    mainLayout->addLayout(boxLayout);
    mainLayout->addWidget(pagesTab);

    setCentralWidget(analysis);

    connect(algorithmBox, &QComboBox::currentTextChanged, this, [](const QString &selectedAlgorithm) {
        qDebug() << "ustawiono algorytm:" << selectedAlgorithm;
        peaksResult.clear(); // czyszcze wektor peaksów po zmianie algorytmu
        method_counter=0;
    });
}

QWidget* MainWindow::create_HRV1Page() {
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *label = new QLabel("Parametry wyznaczane podczas analizy HRV 1");
    label->setAlignment(Qt::AlignCenter);

    QFont font = label->font();
    font.setPointSize(14);
    label->setFont(font);
    layout->addWidget(label);

    QHBoxLayout *paramsLayout = new QHBoxLayout();

    QStringList timeparams = {"RR mean [ms]", "SDNN [ms]", "RMSSD [ms]", "NN50", "pNN50 [%]"};
    QMap<QString, QLabel*> timeparamsLabels;

    QVBoxLayout *timeparamsLayout = new QVBoxLayout();
    for (const QString &param : timeparams) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        QLabel *paramLabel = new QLabel(param + ":");
        QLabel *valueLabel = new QLabel("-"); // Domyślna wartość to "-"
        timeparamsLabels[param] = valueLabel;

        rowLayout->addWidget(paramLabel);
        rowLayout->addWidget(valueLabel);
        rowLayout->addStretch();
        timeparamsLayout->addLayout(rowLayout);
    }

    QChar toSquare(0x00B2);
    QString secSquared = "[s" + QString(toSquare) + "]";

    QStringList freqparams = {"TP " + secSquared, "HF " + secSquared, "LF " + secSquared, "VLF " + secSquared, "ULF " + secSquared, "LFHF"};
    QMap<QString, QLabel*> freqparamsLabels;

    QVBoxLayout *freqparamsLayout = new QVBoxLayout();
    for (const QString &param : freqparams) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        QLabel *paramLabel = new QLabel(param + ":");
        QLabel *valueLabel = new QLabel("-"); // Domyślna wartość to "-"
        freqparamsLabels[param] = valueLabel;

        rowLayout->addWidget(paramLabel);
        rowLayout->addWidget(valueLabel);
        rowLayout->addStretch();
        freqparamsLayout->addLayout(rowLayout);
    }

    paramsLayout->addLayout(timeparamsLayout);
    paramsLayout->addLayout(freqparamsLayout);

    layout->addLayout(paramsLayout);

    QPushButton *applyButton = new QPushButton("Analizuj");
    layout->addWidget(applyButton);

    connect(applyButton, &QPushButton::clicked, this, [timeparamsLabels, freqparamsLabels, timeparams, freqparams, widget]() {
        if (channelData.isEmpty() && filteredData.isEmpty()) {
            QMessageBox::warning(widget, "Błąd", "Nie wczytano danych. Wczytaj plik .dat \nzanim przystąpisz do analizy.");
            qDebug() << "Puste dane";
            return;
        }

        // QVector<double> data = !filteredData.isEmpty() ? filteredData : channelData;
        QVector<double> data = filteredData;
        // Utworzenie i konfiguracja QProgressDialog
        QProgressDialog progressDialog("Analiza w toku...", "Anuluj", 0, 100, widget);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setAutoClose(false);
        progressDialog.setAutoReset(false);
        progressDialog.setValue(0); // Początkowy postęp
        progressDialog.show();

        QCoreApplication::processEvents();

        if(method_counter==0){
            if (algorithmBox->currentIndex() == 0) {
                PanTompkins pantompkins;
                peaksResult = pantompkins.getPeaks(data);
                method_counter=1;
            } else if (algorithmBox->currentIndex() == 1) {
                HilbertTransform hilbert;
                peaksResult = hilbert.GetPeaks(data);
                method_counter=1;
            }
        }

        progressDialog.setValue(50); // Ustawienie postępu na 50%
        QCoreApplication::processEvents();

        HRV1 analizahrv1;

        if (peaksResult.isEmpty()) {
            qDebug() << "Nic nie wyznaczono.";
        } else {
            QVector<double> results_time = analizahrv1.analyzeTimeDomain(peaksResult, sampling_freq);
            QVector<double> results_freq = analizahrv1.analyzeFreqDomain(peaksResult, sampling_freq);

            for (int i = 0; i < results_time.size(); ++i) {
                timeparamsLabels[timeparams[i]]->setText(QString::number(results_time[i]));
            }

            for (int i = 0; i < results_freq.size(); ++i) {
                freqparamsLabels[freqparams[i]]->setText(QString::number(results_freq[i]));
            }

            // Tu wypisz freqparams !!!
            RaportInfo obj;
            obj.moduleName = "HRV1";


            // Dodaj wyniki freqparams do obj.parameters
            for (int i = 0; i < results_freq.size(); ++i) {
                obj.parameters.append(freqparams[i] + ": " + QString::number(results_freq[i]));
            }
            for (int i = 0; i < results_time.size(); ++i) {
                obj.parameters.append(timeparams[i] + ": " + QString::number(results_time[i]));
            }
            raport_vector.append(obj);
        }

        progressDialog.setValue(100); // Analiza zakończona
        QCoreApplication::processEvents();
    });

    widget->setLayout(layout);
    return widget;
}


QWidget* MainWindow::create_HRV2Page()
{
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *label = new QLabel("Parametry i wykresy wyznaczane podczas analizy HRV2");
    layout->addWidget(label);

    QHBoxLayout *chartsLayout = new QHBoxLayout();

    // Poincare plot
    QVBoxLayout *poincareLayout = new QVBoxLayout();
    QLabel *poincareLabel = new QLabel("Wykres Poincare");
    QCustomPlot *poincarePlot = new QCustomPlot();
    poincarePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    poincarePlot->setMinimumSize(300, 300);
    poincarePlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    poincareLayout->addWidget(poincareLabel);
    poincareLayout->addWidget(poincarePlot);

    // Histogram
    QVBoxLayout *histogramLayout = new QVBoxLayout();
    QLabel *histogramLabel = new QLabel("Histogram RR");
    QCustomPlot *histogramPlot = new QCustomPlot();
    //histogramPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    histogramPlot->setMinimumSize(300, 300);
    histogramPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    histogramLayout->addWidget(histogramLabel);
    histogramLayout->addWidget(histogramPlot);

    chartsLayout->addLayout(poincareLayout);
    chartsLayout->addLayout(histogramLayout);

    layout->addLayout(chartsLayout);

    QHBoxLayout *paramsLayout = new QHBoxLayout();

    QStringList params = {"TINN", "Triangular Index", "SD1", "SD2"};
    QMap<QString, QLabel*> paramsLabels;

    QVBoxLayout *paramsLabelsLayout = new QVBoxLayout();
    for (const QString &param : params) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        QLabel *paramLabel = new QLabel(param + ":");
        QLabel *valueLabel = new QLabel("-");
        paramsLabels[param] = valueLabel;

        rowLayout->addWidget(paramLabel);
        rowLayout->addWidget(valueLabel);
        rowLayout->addStretch();
        paramsLabelsLayout->addLayout(rowLayout);
    }

    paramsLayout->addLayout(paramsLabelsLayout);
    layout->addLayout(paramsLayout);

    QPushButton *applyButton = new QPushButton("Analizuj");
    layout->addWidget(applyButton);

    connect(applyButton, &QPushButton::clicked, this, [paramsLabels, params, poincarePlot, histogramPlot, widget]() {
        if (channelData.isEmpty() && filteredData.isEmpty()) {
            QMessageBox::warning(widget, "Błąd", "Nie wczytano danych. Wczytaj plik .dat \nzanim przystąpisz do analizy.");
            qDebug() << "Puste dane";
            return;
        }

        // QVector<double> data = !filteredData.isEmpty() ? filteredData : channelData;
        QVector<double> data = filteredData;

        QProgressDialog progressDialog("Analiza w toku...", "Anuluj", 0, 100, widget);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setAutoClose(false);
        progressDialog.setAutoReset(false);
        progressDialog.setValue(0); // Początkowy postęp
        progressDialog.show();

        QCoreApplication::processEvents();

        //  if (peaksResult.isEmpty()) {  tutaj zakomentowalem bo inaczej to przy drugim wczytywaniu pliku nie liczą sie wartosci z wczytanego kanalu tylko starego a teraz sie liczą z nowego
        if(method_counter==0){
            if (algorithmBox->currentIndex() == 0) {
                PanTompkins pantompkins;
                peaksResult = pantompkins.getPeaks(data);
                method_counter=1;
            } else if (algorithmBox->currentIndex() == 1) {
                HilbertTransform hilbert;
                peaksResult = hilbert.GetPeaks(data);
                method_counter=1;
            }
        }

        //   }
        progressDialog.setValue(50); // Ustawienie postępu na 50%
        QCoreApplication::processEvents();


        if (peaksResult.isEmpty()) {
            qDebug() << "Nie udało się wyznaczyć wyników. Dane są puste.";
            return;
        }

        HRV2 hrv2;

        hrv2.analyzeGeometry(peaksResult, 360);

        QMap<QString, double> geometry_results = hrv2.getGeometryResults();
        QList<double> geometry_values = geometry_results.values();

        for (int i = 0; i < params.size(); ++i) {
            paramsLabels[params[i]]->setText(QString::number(geometry_values[i]));
        }

        QPair<QVector<double>, QVector<double>> poincare = hrv2.getPoincarePlot();
        QPair<QVector<double>, QVector<double>> ellipse = hrv2.getEllipse(200);
        QPair<QVector<double>, QVector<double>> sd1_coords = hrv2.getSd1Sd2Coordinates(1);
        QPair<QVector<double>, QVector<double>> sd2_coords = hrv2.getSd1Sd2Coordinates(2);

        poincarePlot->clearGraphs();
        poincarePlot->clearItems();
        poincarePlot->legend->setVisible(true);
        poincarePlot->legend->setBrush(QBrush(QColor(255, 255, 255, 230)));

        poincarePlot->addGraph();
        poincarePlot->graph()->setData(poincare.first, poincare.second);
        poincarePlot->graph(0)->setLineStyle(QCPGraph::lsNone);
        poincarePlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::lightGray, Qt::lightGray, 2));
        poincarePlot->graph(0)->setName("Points");

        QCPCurve *ellipseCurve = new QCPCurve(poincarePlot->xAxis, poincarePlot->yAxis);
        QVector<QCPCurveData> ellipseData;
        for (int i = 0; i < ellipse.first.size(); ++i) {
            ellipseData.append(QCPCurveData(i, ellipse.first[i], ellipse.second[i]));
        }
        ellipseCurve->data()->set(ellipseData, true);
        ellipseCurve->setPen(QPen(Qt::black, 2));
        ellipseCurve->setName("Ellipse");

        poincarePlot->addGraph();
        poincarePlot->graph(1)->setData(sd1_coords.first, sd1_coords.second);
        poincarePlot->graph(1)->setPen(QPen(Qt::green, 2));
        poincarePlot->graph(1)->setName("SD1 Line");

        poincarePlot->addGraph();
        poincarePlot->graph(2)->setData(sd2_coords.first, sd2_coords.second);
        poincarePlot->graph(2)->setPen(QPen(Qt::red, 2));
        poincarePlot->graph(2)->setName("SD2 Line");

        poincarePlot->xAxis->setLabel("RR(n)");
        poincarePlot->yAxis->setLabel("RR(n+1)");
        poincarePlot->xAxis->setRange(*std::min_element(poincare.first.begin(), poincare.first.end()),
                                      *std::max_element(poincare.first.begin(), poincare.first.end()) );
        poincarePlot->yAxis->setRange(*std::min_element(poincare.second.begin(), poincare.second.end()),
                                      *std::max_element(poincare.second.begin(), poincare.second.end()));
        poincarePlot->replot();

        // RR Histogram
        QPair<QVector<double>, QVector<double>> rrHistogram = hrv2.getHistogram();

        histogramPlot->clearPlottables();

        QCPBars *bars = new QCPBars(histogramPlot->xAxis, histogramPlot->yAxis);
        bars->setData(rrHistogram.first, rrHistogram.second);
        bars->setWidth(0.03);
        bars->setBrush(QColor(100, 200, 255));
        bars->setPen(QPen(Qt::blue));

        histogramPlot->xAxis->setLabel("RR Interval");
        histogramPlot->yAxis->setLabel("Count");
        histogramPlot->xAxis->setRange(0, *std::max_element(rrHistogram.first.begin(), rrHistogram.first.end()) + 0.1);
        histogramPlot->yAxis->setRange(0, *std::max_element(rrHistogram.second.begin(), rrHistogram.second.end()) + 5);
        histogramPlot->replot();

        // Dodaj wyniki do obj.parameters
        RaportInfo obj;
        obj.moduleName = "HRV2";
        obj.chartPixmap = poincarePlot->toPixmap(800, 600);
        obj.chartPixmap1 = histogramPlot->toPixmap(800, 600);

        // Dodaj wyniki geometryczne do obj.parameters
        for (int i = 0; i < params.size(); ++i) {
            obj.parameters.append(params[i] + ": " + QString::number(geometry_values[i]));
        }

        raport_vector.append(obj);
        progressDialog.setValue(100); // Analiza zakończona
        QCoreApplication::processEvents();
    });
    widget->setLayout(layout);
    return widget;
}


QWidget* MainWindow::create_HRVDFAPage()
{
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *label = new QLabel("Parametry i wykresy wyznaczane podczas analizy HRV DFA");

    layout->addWidget(label);

    QVBoxLayout *chartsLayout = new QVBoxLayout();

    // HRV DFA plot
    QLabel *dfaChartLabel = new QLabel("Wykres dla HRV DFA");
    QCustomPlot *dfaPlot = new QCustomPlot();
    dfaPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    dfaPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    chartsLayout->addWidget(dfaChartLabel);
    chartsLayout->addWidget(dfaPlot);

    layout->addLayout(chartsLayout);

    QHBoxLayout *paramsLayout = new QHBoxLayout();

    QStringList params = {QChar{0xb1, 0x03} + '1', QChar{0xb1, 0x03} + '2'};
    QMap<QString, QLabel*> paramsLabels;

    QVBoxLayout *paramsLabelsLayout = new QVBoxLayout();
    for (const QString &param : params) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        QLabel *paramLabel = new QLabel(param + ":");
        QLabel *valueLabel = new QLabel("-"); // Default value is "-"
        paramsLabels[param] = valueLabel;

        rowLayout->addWidget(paramLabel);
        rowLayout->addWidget(valueLabel);
        rowLayout->addStretch();
        paramsLabelsLayout->addLayout(rowLayout);
    }

    paramsLayout->addLayout(paramsLabelsLayout);
    layout->addLayout(paramsLayout);

    QPushButton *applyButton = new QPushButton("Analizuj");
    layout->addWidget(applyButton);

    connect(applyButton, &QPushButton::clicked, this, [paramsLabels, params, dfaPlot, widget]() {
        if (channelData.isEmpty() && filteredData.isEmpty()) {
            QMessageBox::warning(widget, "Błąd", "Nie wczytano danych. Wczytaj plik .dat \nzanim przystąpisz do analizy.");
            qDebug() << "Puste dane";
            return;
        }

        QVector<double> data = filteredData;
        // Utworzenie i konfiguracja QProgressDialog
        QProgressDialog progressDialog("Analiza w toku...", "Anuluj", 0, 100, widget);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setAutoClose(false);
        progressDialog.setAutoReset(false);
        progressDialog.setValue(0); // Początkowy postęp
        progressDialog.show();

        QCoreApplication::processEvents();
        //   if (peaksResult.isEmpty()) {
        if(method_counter==0){
            if (algorithmBox->currentIndex() == 0) {
                PanTompkins pantompkins;
                peaksResult = pantompkins.getPeaks(data);
                method_counter=1;
            } else if (algorithmBox->currentIndex() == 1) {
                HilbertTransform hilbert;
                peaksResult = hilbert.GetPeaks(data);
                method_counter=1;
            }
        }
        //   }

        progressDialog.setValue(50); // Ustawienie postępu na 50%
        QCoreApplication::processEvents();

        if (peaksResult.isEmpty()) {
            qDebug() << "Nie udało się wyznaczyć wyników. Dane są puste.";
            return;
        }

        HRV_DFA hrv_dfa;

        QVector<double> rr_intervals;
        for (int i = 1; i < peaksResult.size(); ++i) {
            double rr_interval = static_cast<double>(peaksResult[i] - peaksResult[i - 1]) / sampling_freq;
            rr_intervals.append(rr_interval);
        }

        hrv_dfa.analyze(rr_intervals);

        auto [alpha1, alpha2, intercept1, intercept2] = hrv_dfa.getParams();

        paramsLabels[params[0]]->setText(QString::number(alpha1));
        paramsLabels[params[1]]->setText(QString::number(alpha2));

        dfaPlot->clearGraphs();
        dfaPlot->clearItems();

        QVector<double> log_n1, log_F_n1;
        for (const auto &point : hrv_dfa.getDfaData1()) {
            log_n1.append(point.first);
            log_F_n1.append(point.second);
        }
        dfaPlot->addGraph();
        dfaPlot->graph(0)->setData(log_n1, log_F_n1);
        dfaPlot->graph(0)->setPen(Qt::NoPen);
        dfaPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle,  QPen(Qt::blue), QBrush(Qt::NoBrush), 3));
        dfaPlot->graph(0)->setName("Alpha 1");

        QVector<double> log_n2, log_F_n2;
        for (const auto &point : hrv_dfa.getDfaData2()) {
            log_n2.append(point.first);
            log_F_n2.append(point.second);
        }
        dfaPlot->addGraph();
        dfaPlot->graph(1)->setData(log_n2, log_F_n2);
        dfaPlot->graph(1)->setPen(Qt::NoPen);
        dfaPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::red), QBrush(Qt::NoBrush), 3));

        dfaPlot->graph(1)->setName("Alpha 2");

        // Linie regresji
        auto params = hrv_dfa.getParams();

        QVector<double> fit_x1 = {log_n1.first(), log_n1.last()};
        QVector<double> fit_y1 = {std::get<0>(params) * fit_x1[0] + std::get<2>(params),
                                  std::get<0>(params) * fit_x1[1] + std::get<2>(params)};
        dfaPlot->addGraph();
        dfaPlot->graph()->setData(fit_x1, fit_y1);
        dfaPlot->graph()->setPen(QPen(Qt::blue));
        dfaPlot->graph()->setName("Linia regresji Alpha 1");

        QVector<double> fit_x2 = {log_n2.first(), log_n2.last()};
        QVector<double> fit_y2 = {std::get<1>(params) * fit_x2[0] + std::get<3>(params),
                                  std::get<1>(params) * fit_x2[1] + std::get<3>(params)};
        dfaPlot->addGraph();
        dfaPlot->graph()->setData(fit_x2, fit_y2);
        dfaPlot->graph()->setPen(QPen(Qt::red));
        dfaPlot->graph()->setName("Linia regresji Alpha 2");

        dfaPlot->legend->setVisible(true);
        dfaPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 230)));
        dfaPlot->legend->setBorderPen(QPen(Qt::black));
        dfaPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);

        dfaPlot->xAxis->setLabel("log(n)");
        dfaPlot->yAxis->setLabel("log(F(n))");
        dfaPlot->xAxis->setRange(*std::min_element(log_n1.begin(), log_n1.end()),
                                 *std::max_element(log_n2.begin(), log_n2.end()));
        dfaPlot->yAxis->setRange(*std::min_element(log_F_n1.begin(), log_F_n1.end()),
                                 *std::max_element(log_F_n2.begin(), log_F_n2.end()));
        dfaPlot->replot();


        // Dodaj wyniki do obj.parameters
        RaportInfo obj;
        obj.moduleName = "HRV_DFA";
        obj.chartPixmap = dfaPlot->grab();
        // obj.parameters.append(params[0] + ": " + QString::number(alpha1));
        // obj.parameters.append(params[1] + ": " + QString::number(alpha2));
        obj.parameters.append(QString("Alpha 1: %1").arg(QString::number(std::get<0>(params))));
        obj.parameters.append(QString("Alpha 2: %1").arg(QString::number(std::get<1>(params))));
        raport_vector.append(obj);
        progressDialog.setValue(100); // Analiza zakończona
        QCoreApplication::processEvents();
    });

    widget->setLayout(layout);
    return widget;
}


QWidget* MainWindow::create_HeartClassPage() {
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *label = new QLabel("Parametry i wykresy wyznaczane podczas analizy Heart Class i Waves");
    layout->addWidget(label);

    QVBoxLayout *chartsLayout = new QVBoxLayout();

    // Wykres dla Heart Class
    QLabel *heart_classChartLabel = new QLabel("Wykres sygnału EKG z zaznaczonymi punktami QRS");
    QChartView *heart_classChartView = new QChartView();
    heart_classChartView->setRenderHint(QPainter::Antialiasing);
    chartsLayout->addWidget(heart_classChartLabel);
    chartsLayout->addWidget(heart_classChartView);

    layout->addLayout(chartsLayout);

    QHBoxLayout *paramsLayout = new QHBoxLayout();

    QStringList params = {"Zespoły QRS", "Anomalie", "SVT (Częstoskurcz nadkomorowy)", "VT (Częstoskurcz komorowy)"};
    QMap<QString, QLabel*> paramsLabels;

    QVBoxLayout *paramsLabelsLayout = new QVBoxLayout();
    for (const QString &param : params) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        QLabel *paramLabel = new QLabel(param + ":");
        QLabel *valueLabel = new QLabel("-"); // Domyślna wartość to "-"
        paramsLabels[param] = valueLabel;

        rowLayout->addWidget(paramLabel);
        rowLayout->addWidget(valueLabel);
        rowLayout->addStretch();
        paramsLabelsLayout->addLayout(rowLayout);
    }

    paramsLayout->addLayout(paramsLabelsLayout);
    layout->addLayout(paramsLayout);

    QPushButton *applyButton = new QPushButton("Analizuj");
    layout->addWidget(applyButton);

    connect(applyButton, &QPushButton::clicked, this, [paramsLabels, params, heart_classChartView, widget]() {
        if (channelData.isEmpty() && filteredData.isEmpty()) {
            QMessageBox::warning(widget, "Błąd", "Nie wczytano danych. Wczytaj plik .dat \nzanim przystąpisz do analizy.");
            qDebug() << "Puste dane";
            return;
        }

        QVector<double> data = filteredData;

        QProgressDialog progressDialog("Analiza w toku...", "Anuluj", 0, 100, widget);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setAutoClose(false);
        progressDialog.setAutoReset(false);
        progressDialog.setValue(0);
        progressDialog.show();

        QCoreApplication::processEvents();
        //  if (peaksResult.isEmpty()) {
        if(method_counter==0){
            if (algorithmBox->currentIndex() == 0) {
                PanTompkins pantompkins;
                peaksResult = pantompkins.getPeaks(data);
                method_counter=1;
            } else if (algorithmBox->currentIndex() == 1) {
                HilbertTransform hilbert;
                peaksResult = hilbert.GetPeaks(data);
                method_counter=1;
            }
        }
        // }

        progressDialog.setValue(50); // Ustawienie postępu na 50%
        QCoreApplication::processEvents();
        if (peaksResult.isEmpty()) {
            qDebug() << "Nie udało się wyznaczyć wyników. Dane są puste.";
            return;
        }



        QChart *chart = new QChart();

        QLineSeries *ecgSeries = new QLineSeries();
        int numSamples = qMin(5000, data.size());
        for (int i = 0; i < data.size(); ++i) {
            ecgSeries->append(i, data[i]);
        }
        chart->addSeries(ecgSeries);
        ecgSeries->setName("Sygnał EKG");
        QPen pen(Qt::blue);
        pen.setWidth(1);
        ecgSeries->setPen(pen);

        QScatterSeries *qrsSeries = new QScatterSeries();
        qrsSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        qrsSeries->setMarkerSize(8.0);
        qrsSeries->setColor(Qt::green);

        for (int peak : peaksResult) {
            if (peak < data.size()) {
                qrsSeries->append(peak, data[peak]);
            }
        }
        chart->addSeries(qrsSeries);
        qrsSeries->setName("Punkty QRS");

        const int chunkSize = 2000;
        const int totalChunks = data.size() / chunkSize + (data.size() % chunkSize != 0);

        QVector<QString> allHeartResults;
        QVector<int> allQRSonsets, allQRSoffsets;

        for (int chunkIndex = 0; chunkIndex < totalChunks; ++chunkIndex) {

            int start = chunkIndex * chunkSize;
            int end = qMin(start + chunkSize, data.size());

            QVector<double> chunk = data.mid(start, end - start);

            QVector<int> peaksInChunk;
            for (int peak : peaksResult) {
                if (peak >= start && peak < end) {
                    peaksInChunk.append(peak - start);
                }
            }

            if (peaksInChunk.isEmpty()) continue; // Pomiń pusty fragment

            Waves waves_analysis;
            QVector<int> QRSonsets = waves_analysis.getQRSOnsets(chunk, peaksInChunk);
            QVector<int> QRSoffsets = waves_analysis.getQRSOffsets(chunk, peaksInChunk);

            HEART_CLASS heart_class;
            heart_class.classifyQRS(chunk, QRSonsets, QRSoffsets);
            QVector<QString> heartResults = heart_class.getClassifications();

            allQRSonsets.append(QRSonsets);
            allQRSoffsets.append(QRSoffsets);
            allHeartResults.append(heartResults);

            // qDebug() << "Heart class, loop: " << chunkIndex + 1 << "/" << totalChunks;
        }

        QMap<QString, int> classificationCounts;

        for(const QString& classification: allHeartResults){
            classificationCounts[classification]++;
        }

        QScatterSeries *anomalySeries = new QScatterSeries();
        anomalySeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        anomalySeries->setMarkerSize(8.0);
        anomalySeries->setColor(Qt::red);

        for (int i = 0; i < peaksResult.size(); ++i) {
            if (allHeartResults[i] == "Anomaly") {
                anomalySeries->append(peaksResult[i], data[peaksResult[i]]);
            }
        }
        chart->addSeries(anomalySeries);
        anomalySeries->setName("Anomalie");

        QScatterSeries *svtSeries = new QScatterSeries();
        svtSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        svtSeries->setMarkerSize(8.0);
        svtSeries->setColor(Qt::yellow);

        for (int i = 0; i < peaksResult.size(); ++i) {
            if (allHeartResults[i] == "SVT") {
                svtSeries->append(peaksResult[i], data[peaksResult[i]]);
            }
        }
        chart->addSeries(svtSeries);
        svtSeries->setName("SVT");

        QScatterSeries *vtSeries = new QScatterSeries();
        vtSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        vtSeries->setMarkerSize(8.0);
        vtSeries->setColor(Qt::magenta);

        for (int i = 0; i < peaksResult.size(); ++i) {
            if (allHeartResults[i] == "VT") {
                vtSeries->append(peaksResult[i], data[peaksResult[i]]);
            }
        }
        chart->addSeries(vtSeries);
        vtSeries->setName("VT");


        // Oś X i Y
        chart->createDefaultAxes();
        auto xAxis = dynamic_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
        auto yAxis = dynamic_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());

        if (xAxis) {
            xAxis->setTitleText("Czas");
            xAxis->setRange(0, 1000);
        }
        if (yAxis) {
            yAxis->setTitleText("Amplituda");
            yAxis->setRange(
                *std::min_element(data.begin(), data.begin() + numSamples) - 1.0,
                *std::max_element(data.begin(), data.begin() + numSamples) + 1.0
                );
        }

        heart_classChartView->setChart(chart);
        chart->setAnimationOptions(QChart::NoAnimation);
        heart_classChartView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        heart_classChartView->setRubberBand(QChartView::HorizontalRubberBand);

        if (xAxis) {
            connect(xAxis, &QValueAxis::rangeChanged, xAxis, [xAxis]() {
                if (xAxis->min() < 0) {
                    xAxis->setMin(0);
                }
            });
        }

        // Aktualizacja parametrów
        paramsLabels[params[0]]->setText(QString::number(allQRSonsets.size()));
        paramsLabels[params[1]]->setText(QString::number(classificationCounts["Anomaly"]));
        paramsLabels[params[2]]->setText(QString::number(classificationCounts["SVT"]));
        paramsLabels[params[3]]->setText(QString::number(classificationCounts["VT"]));
        // Dodaj wyniki do obj.parameters
        RaportInfo obj;
        obj.moduleName = "HeartClass";
        obj.chartPixmap = heart_classChartView->grab();
        obj.parameters.append(params[0] + ": " + QString::number(allQRSonsets.size()));
        obj.parameters.append(params[1] + ": " + QString::number(classificationCounts["Anomaly"]));
        obj.parameters.append(params[2] + ": " + QString::number(classificationCounts["SVT"]));
        obj.parameters.append(params[3] + ": " + QString::number(classificationCounts["VT"]));
        raport_vector.append(obj);
        progressDialog.setValue(100); // Analiza zakończona
        QCoreApplication::processEvents();
    });

    widget->setLayout(layout);
    return widget;
}


void MainWindow::on_actionRaport_triggered()
{
    // Ustawienie widgetu do wyświetlenia wykresów w formie PDF
    QString pdfPath = QFileDialog::getSaveFileName(this, "Zapisz Raport", "", "Dokumenty PDF (*.pdf)");
    if (pdfPath.isEmpty())
        return;

    QPdfWriter writer(pdfPath);
    writer.setPageSize(QPageSize::A4);
    writer.setPageMargins(QMargins(10, 10, 10, 10)); // Marginesy 10px
    QPainter painter(&writer);

    if (!painter.isActive()) {
        QMessageBox::warning(this, "Błąd", "Nie udało się otworzyć pliku PDF.");
        return;
    }

    int yOffset = 50; // Odstęp od góry strony
    const int lineSpacing = 200; // Odstęp między liniami tekstu
    const int pageWidth = writer.width();
    const int pageHeight = writer.height();
    const int marginBottom = 50; // Dolny margines

    QFont font = painter.font();
    font.setPointSize(20);
    font.setBold(true);
    painter.setFont(font);

    QSet<QString> includedModules;
    // Dodaj tytuł i datę generowania na górze każdej strony
    QDateTime currentTime = QDateTime::currentDateTime();
    QRect titleRect(0, yOffset, pageWidth, 600); // Prostokąt tytułu na całej szerokości strony
    painter.drawText(titleRect, Qt::AlignCenter, "Raport analizy zapisu EKG pacjenta"); // Wyśrodkowany tytuł
    yOffset += 700; // Dodatkowy odstęp dla daty
    font.setPointSize(9);
    font.setBold(false);
    painter.setFont(font);
    QString dateText = QString("Data generowania raportu: %1").arg(currentTime.toString("dd.MM.yyyy hh:mm"));
    painter.drawText(3000, yOffset, dateText);
    yOffset += 600; // Zwiększ odstęp przed rozpoczęciem listy modułów
    font.setPointSize(12);
    painter.setFont(font);

    // Zdefiniowanie zbioru zawierającego unikalne identyfikatory modułów
    QSet<QString> modułyDoZachowania = {"Rodzaj filtracji: ", "Surowe", "HRV1", "HRV2", "HRV_DFA", "HeartClass"};

    // Słownik do śledzenia, czy pierwsze wystąpienie modułu zostało już dodane
    QMap<QString, bool> flagi;

    // Przygotowanie mapy - początkowo wszystkie flagi są false
    for (const auto& moduł : modułyDoZachowania) {
        flagi[moduł] = false;
    }

    // Iterowanie od końca wektora
    for (int i = raport_vector.size() - 1; i >= 0; --i) {
        for (const auto& moduł : modułyDoZachowania) {
            // Sprawdzanie, czy moduleName zawiera jeden z zdefiniowanych ciągów
            if (raport_vector[i].moduleName.contains(moduł)) {
                if (!flagi[moduł]) {
                    // Pierwsze wystąpienie - ustawienie flagi na true, element nie jest usuwany
                    flagi[moduł] = true;
                } else {
                    // Kolejne wystąpienia są usuwane
                    raport_vector.remove(i);
                }
                break; // Zakończenie wewnętrznej pętli, ponieważ moduł już został przetworzony
            }
        }
    }

    for (const RaportInfo& r_v : raport_vector) {
        if (!includedModules.contains(r_v.moduleName)){
            // Wyświetlanie tytułu 'moduleName' nad wykresem
            if (!r_v.moduleName.isEmpty()) {
                font.setPointSize(15);
                font.setBold(true);
                painter.setFont(font);
                if (yOffset + 6000 > pageHeight - marginBottom) {
                    writer.newPage();
                    yOffset = 50; // Reset pozycji do góry strony na nowej stronie
                }
                painter.drawText(100, yOffset, r_v.moduleName);
                yOffset += lineSpacing;
                qDebug() << "Naglowek:";
                qDebug() << yOffset;
                includedModules.insert(r_v.moduleName); // Dodaj nazwę modułu do zbioru uwzględnionych modułów
            }


            if (!r_v.chartPixmap.isNull()) {
                int availableHeight = pageHeight - yOffset - marginBottom;

                // Jeśli wykres nie mieści się na stronie, rozpocznij nową stronę
                if (availableHeight < 6000) { // Minimalna wysokość na wykres
                    writer.newPage();
                    yOffset = 50; // Reset pozycji do góry strony na nowej stronie
                }

                QRect chartRect(10, yOffset, pageWidth - 20, 6000);
                painter.drawPixmap(chartRect, r_v.chartPixmap);
                yOffset += chartRect.height() + lineSpacing;

                qDebug() << "Obraz:";
                qDebug() << yOffset;
            }

            if (!r_v.chartPixmap1.isNull()) {
                int availableHeight = pageHeight - yOffset - marginBottom;

                // Jeśli wykres nie mieści się na stronie, rozpocznij nową stronę
                if (availableHeight < 6000) { // Minimalna wysokość na wykres
                    writer.newPage();
                    yOffset = 50; // Reset pozycji do góry strony na nowej stronie
                }

                QRect chartRect(10, yOffset, pageWidth - 20, 6000);
                painter.drawPixmap(chartRect, r_v.chartPixmap1);
                yOffset += chartRect.height() + lineSpacing;

                qDebug() << "Obraz:";
                qDebug() << yOffset;
            }

            // Wyświetlanie parametrów poniżej wykresu
            if (!r_v.parameters.isEmpty()) {
                if (yOffset + lineSpacing > pageHeight - marginBottom) {
                    writer.newPage();
                    yOffset = 50;
                }
                yOffset += 200;
                font.setPointSize(12);
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(100, yOffset, "Parametry:");
                yOffset += 100+lineSpacing;
                font.setBold(false);
                painter.setFont(font);

                for (int i = 0; i < r_v.parameters.size(); ++i) {
                    const QString& param = r_v.parameters[i];

                    if (yOffset + lineSpacing > pageHeight - marginBottom) {
                        writer.newPage();
                        yOffset = 50;
                        painter.drawText(100, yOffset, "Parametry (kontynuacja):");
                        yOffset += lineSpacing;
                    }

                    painter.drawText(120, yOffset, param);
                    yOffset += lineSpacing;
                }

                yOffset += 200 +lineSpacing; // Większy odstęp po parametrach
            }

            if (yOffset > pageHeight - marginBottom) {
                writer.newPage();
                yOffset = 50;
            }
        }
    }

    QMessageBox::information(this, "Raport PDF", "Raport PDF został zapisany.");
}
void MainWindow::on_actionWyjd_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionJak_korzysta_z_aplikacji_triggered()
{
    QMessageBox::information(this, "Instrukcja",
                             "Aby przeprowadzić analizę sygnału wykonaj następujące kroki:\n"
                             "1. Naciśnij przycisk \"Wczytaj plik\" i wybierz z eksploratora odpowiedni plik z rozszerzeniem .DAT. "
                             "Po poprawnym wczytaniu pliku wybierz kanał, z którego zostaną pobrane dane do dalszej analizy.\n"
                             "2. Przejdź do sekcji \"Filtracja\" i wybierz z listy rozwijalnej metodę filtracji. "
                             "Możliwa jest również analiza sygnału nieprzefiltrowanego (opcja: \"Brak filtracji\"). "
                             "Po naciśnięciu przycisku \"Zastosuj\" na wykresie pojawi się sygnał EKG przefiltrowany wybraną metodą "
                             "lub nieprzefiltrowany, jeśli wybrano opcję \"Brak filtracji\".\n"
                             "3. Następnie przejdź do sekcji \"Analiza\". Wybierz algorytm do wykrywania załamków R z listy rozwijalnej "
                             "w górnej części okna aplikacji, a następnie wykonaj wybraną analizę, przechodząc do odpowiedniej zakładki "
                             "i naciskając przycisk \"Analizuj\".\n"
                             "4. Za pomocą przycisku \"Wyświetl EKG\" można wyświetlić nieprzefiltrowany sygnał, który został wczytany do programu.\n"
                             "5. Aby wygenerować raport z analizy, naciśnij przycisk \"Raport\", a następnie w eksploratorze plików nadaj nazwę pliku "
                             "oraz wybierz odpowiedni folder. Raport jest generowany w formacie PDF.\n"
                             "6. Aby zakończyć pracę z aplikacją, wybierz w górnym panelu opcję \"Menu\", a następnie opcję \"Wyjdź\"."
                             );
}

void MainWindow::ekranPowitalny()
{
    QWidget *welcomeWidget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(welcomeWidget);


    QLabel *imageLabel = new QLabel(welcomeWidget);
    QPixmap pixmap(":/images/logo2.png");
    QPixmap scaledPixmap = pixmap.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel->setPixmap(scaledPixmap);
    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel, 1, 0, Qt::AlignCenter);

    QLabel *welcomeLabel = new QLabel(
        "Witaj w aplikacji! Wgraj swój plik EKG, aby następnie dokonać jego analizy :)", welcomeWidget);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setWordWrap(true);
    QFont font = welcomeLabel->font();
    font.setPointSize(20);
    welcomeLabel->setFont(font);
    layout->addWidget(welcomeLabel, 0, 1, 2, 1, Qt::AlignCenter);

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addItem(verticalSpacer, 1, 0);


    welcomeWidget->setLayout(layout);
    setCentralWidget(welcomeWidget);
}

void MainWindow::applyStyle() {
    QFile file(":/styles/style1.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = file.readAll();
    qApp->setStyleSheet(styleSheet);

}

void MainWindow::toggleSidebar() {
    QToolBar *toolBar_2 = ui->toolBar_2;
    QPropertyAnimation *animation = new QPropertyAnimation(toolBar_2, "pos");
    animation->setDuration(500);

    if (toolBar_2->isVisible()) {
        animation->setEndValue(QPoint(-toolBar_2->width(), toolBar_2->pos().y()));
        connect(animation, &QPropertyAnimation::finished, [toolBar_2]() {
            toolBar_2->setVisible(false);
        });
    } else {
        toolBar_2->setVisible(true);
        animation->setStartValue(QPoint(-toolBar_2->width(), toolBar_2->pos().y()));
        animation->setEndValue(QPoint(0, toolBar_2->pos().y()));
    }

    animation->start();
}



