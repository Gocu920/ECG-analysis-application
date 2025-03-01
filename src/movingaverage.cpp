#include "movingaverage.h"
#include <QVector>
#include <numeric>

QVector<double> MovingAverage::applyFilter(const QVector<double>& data) {

    int N = 0;
    if (window_size % 2 == 0) {
        N = window_size / 2;
        window_size++;
    } else {
        N = (window_size - 1) / 2;
    }

    int L = data.length();
    QVector<double> smoothed;

    for (int i = 0; i < L; i++) {
        if (i >= N && i < (L - N)) {
            // Poprawne użycie std::accumulate na odpowiednim zakresie
            double temp = std::accumulate(data.begin() + (i - N), data.begin() + (i + N), 0.0) / window_size;
            smoothed.append(temp);
        } else {
            // Jeśli nie można zastosować okna, zachowujemy oryginalną wartość
            smoothed.append(data[i]);
        }
    }

    return smoothed;
}
