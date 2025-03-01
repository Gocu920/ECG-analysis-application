#include "butterworth.h"

#include <QVector>
#include <cmath>

QVector<double> Butterworth::applyFilter(const QVector<double>& data){

    QVector<double> a, b, processed(data.size());
    std::tie(a, b) = this->coefficients();

    for (int i = order; i < data.size(); ++i) {
        processed[i] = b[0] * data[i];
        for (int j = 1; j <= order; ++j) {
            processed[i] += b[j] * data[i - j] - a[j] * processed[i - j];
        }
    }

    // zmiany wprowadzone przez Paline, zmiany czasowy , trzeba będzie poprawić
    for (int i = 0; i < processed.size(); ++i) {
        processed[i] *= 0.1;
    }

    return processed;
}

std::pair<QVector<double>, QVector<double>> Butterworth::coefficients() {

    double wc = 2.0 * sampling_rate * tan(M_PI * cutoff / sampling_rate);
    QVector<double> a, b;

    for (int k = 0; k <= order; ++k) {
        double theta = M_PI * (2.0 * k + 1) / (2.0 * order);
        double re = wc * cos(theta);
        double im = wc * sin(theta);

        b.append(pow(-1, k));
        a.append(2.0 * re / (re * re + im * im));
    }

    return {a, b};
}
