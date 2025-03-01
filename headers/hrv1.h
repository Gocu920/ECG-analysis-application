#ifndef HRV1_H
#define HRV1_H

#include <QVector>
#include <QMap>
#include <QString>
#include<map>

class HRV1 {
private:
    QVector <double> time_parameters; // Parametry HRV w dziedzinie czasu
    QVector <double> freq_parameters; // Parametry HRV w dziedzinie częstotliwości

public:
    HRV1();
    QVector <double> analyzeTimeDomain(QVector<int>& peaks, int fs);
    QVector <double> analyzeFreqDomain(QVector<int>& peaks, int fs);

    QVector <double> getTimeParameters() const;
    QVector <double> getFreqParameters() const;
};

#endif // HRV1_H
