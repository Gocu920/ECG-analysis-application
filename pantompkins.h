#ifndef PANTOMPKINS_H
#define PANTOMPKINS_H
#include <QObject>
#include <QDebug>
#include <QVector>
#include <QSharedPointer>
class PanTompkins
{
public:
    PanTompkins()=default;
    /** @brief Funkcja do wykrywania szczytów R metodą Pana Tompkinsa.
     *  @param electrocardiogram_signal - sygnał EKG
     *  @param fs - częstotliwość próbkowania
     *  @return Indeksy szczytów R
     */
    QVector<int> getPeaks(QVector<double> electrocardiogram_signal, float fs = 360.0);
    QVector<double> normalize(QVector<double>& v) const;

private:
    float m_fs; // Częstotliwość próbkowania

    QVector<double> filter(const QVector<double>& signal, double fc1, double fc2) const;


    template<typename T>
    QVector<T> conv(const QVector<T>& f, const QVector<T>& g) const;
};

#endif // PANTOMPKINS_H
