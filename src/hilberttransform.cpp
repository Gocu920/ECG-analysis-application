#include "HilbertTransform.h"

#include <iostream>
#include <QDebug>
#include <QVector>
#include <algorithm>
#include <gsl/gsl_fft_complex.h>
#include <cmath>
#include <gsl/gsl_complex_math.h>
#include <numeric>
#include <boost/math/special_functions.hpp>
#define _USE_MATH_DEFINES
#include <cmath>
using namespace std;


void HilbertTransform::Normalize(QVector<double>& v) const
{
    double MAX = *std::max_element(v.begin(), v.end());
    double MIN = *std::min_element(v.begin(), v.end());
    double MAX_ABS = (abs(MAX) >= abs(MIN)) ? abs(MAX) : abs(MIN);

    std::for_each(v.begin(), v.end(), [MAX_ABS](double& element) {
        element = element / MAX_ABS;
    });
}

// https://stackoverflow.com/questions/24518989/how-to-perform-1-dimensional-valid-convolution
template <typename T>
QVector<T> HilbertTransform::Conv(const QVector<T>& f, const QVector<T>& g) const
{
    int const nf = f.size();
    int const ng = g.size();
    int const n = nf + ng - 1;
    QVector<T> out(n, T());
    for (int i = 0; i < n; ++i)
    {
        int const jmn = (i >= ng - 1) ? i - (ng - 1) : 0;
        int const jmx = (i < nf - 1) ? i : nf - 1;
        for (int j = jmn; j <= jmx; ++j)
        {
            out[i] += (f[j] * g[i - j]);
        }
    }
    return out;
}
float HilbertTransform::Factorial(int n) const
{
    if ((n == 0) || (n == 1))
        return 1;
    else
        return n * Factorial(n - 1);
}
QVector<double> HilbertTransform::Filter(const QVector<double>& signal, double fc1, double fc2) const
{
    const float M = 5.0;
    const float N = 2 * M + 1;

    QVector<float> n1;
    for (int i = -M; i <= M; i++)
        n1.push_back(i);

    float fc = fc1 / (m_fs / 2);

    // Impulse response of a discrete low-pass filter
    QVector<double> y1;
    for (float value : n1)
    {
        float result;
        if ((M_PI * value) != 0)
            result = sin(2 * M_PI * fc * value) / (M_PI * value);
        else
            result = 2 * fc;
        y1.push_back(result);
    }

    // Impulse response of a discrete high-pass filter
    QVector<double> y2;
    fc = fc2 / (m_fs / 2);
    for (float value : n1)
    {
        float result;
        if ((M_PI * value) != 0)
            result = -sin(2 * M_PI * fc * value) / (M_PI * value);
        else
            result = 1 - (2 * fc);
        y2.push_back(result);
    }

    QVector<float> n(N); // n = 0:N-1
    std::iota(n.begin(), n.end(), 0);

    const float alpha = 3; // parametr Kaisera, najczęściej 3
    QVector<float> window;

    for (float n_value : n)
    {
        float ans = pow(2 * n_value / N - 1, 2);
        ans = M_PI * alpha * sqrt(1 - ans);
        ans = boost::math::cyl_bessel_i(0, ans) / boost::math::cyl_bessel_i(0, M_PI * alpha);
        window.push_back(ans);
    }

    for (int i = 0; i < y1.size(); i++)
    {
        y1[i] = window[i] * y1[i];
        y2[i] = window[i] * y2[i];
    }

    QVector<double> c1 = Conv(y1, signal);
    Normalize(c1);
    QVector<double> c2 = Conv(y2, c1);
    Normalize(c2);
    return c2;
}



float HilbertTransform::CalcRMSValue(QVector<double>& signal) const
{
    float rms = 0.f;
    std::for_each(signal.begin(), signal.end(), [&rms](float x) {
        rms += x * x;
    });
    if (signal.size() == 0)
        return 0;
    return sqrt(rms / signal.size());
}



QVector<double> HilbertTransform::Derivative(QVector<double>& signal) const
{
    QVector<double> deriv;
    for (int i = 1; i < signal.size() - 1; i++)
    { // skip boundaries
        float acc = 1. / (2 * m_fs) * (signal[i + 1] - signal[i]);
        deriv.push_back(acc);
    }
    return deriv;
}


QVector<double> HilbertTransform::ComputeHilbertTransform(QVector<double> signal, int first) const
{
#define REAL(z, i) ((z)[2*(i)])
#define IMAG(z, i) ((z)[2*(i)+1])

    double data[2 * m_kSize];

    for (int i = 0; i < m_kSize; i++)
    {
        REAL(data, i) = signal[first + i];
        IMAG(data, i) = 0.0;
    }

    // fast fourier transform
    gsl_fft_complex_radix2_forward(data, 1, m_kSize);

    // set DC component to 0
    REAL(data, 0) = 0.0;
    IMAG(data, 0) = 0.0;

    for (int i = 0; i < m_kSize / 2; i++)
    {
        gsl_complex a, b;
        GSL_REAL(a) = REAL(data, i);
        GSL_IMAG(a) = IMAG(data, i);
        GSL_REAL(b) = 0.0;
        GSL_IMAG(b) = -1.0;

        gsl_complex c = gsl_complex_mul(a, b);
        REAL(data, i) = GSL_REAL(c);
        IMAG(data, i) = GSL_IMAG(c);
    }

    for (int i = m_kSize / 2; i < m_kSize; i++)
    {
        gsl_complex a, b;
        GSL_REAL(a) = REAL(data, i);
        GSL_IMAG(a) = IMAG(data, i);
        GSL_REAL(b) = 0.0;
        GSL_IMAG(b) = 1.0;

        gsl_complex c = gsl_complex_mul(a, b);
        REAL(data, i) = GSL_REAL(c);
        IMAG(data, i) = GSL_IMAG(c);
    }

    // inverse fast fourier transform
    gsl_fft_complex_radix2_inverse(data, 1, m_kSize);

    QVector<double> h;
    for (int i = 0; i < m_kSize; i++)
        h.push_back(REAL(data, i));

#undef REAL
#undef IMAG
    return h;
}



int HilbertTransform::CalcAverageDistance(QVector<int>& peaks) const
{
    float distance = 0.0;
    for (int i = 1; i < peaks.size(); i++)
        distance += (peaks[i] - peaks[i - 1]);
    return distance / (peaks.size() - 1);
}

QVector<int> HilbertTransform::GetPeaks(QVector<double> electrocardiogram_signal, int fs)
{
    m_fs = fs;

    QVector<double> signal = Filter(electrocardiogram_signal, 8, 20);
    signal.erase(signal.begin(), signal.begin() + 9); // offset caused by filters
    signal = Derivative(signal);

    int i = 0;
    float max_amplitude_old = 0.0;
    QVector<int> peaks;
    QVector<int> windows;

    while ((i + m_kSize) < signal.size())
    {
        windows.push_back(i);
        QVector<double> signal_hilb = ComputeHilbertTransform(signal, i);
        double rms_value = CalcRMSValue(signal_hilb);
        auto max_amplitude = std::max_element(signal_hilb.begin(), signal_hilb.end());
        double threshold;

        if (rms_value >= 0.18 * *max_amplitude)
        {
            threshold = 0.39 * *max_amplitude;
            if (*max_amplitude > 2 * max_amplitude_old)
                threshold = 0.0039 * max_amplitude_old;
        }
        else
        {
            threshold = 1.6 * rms_value;
        }

        if (*max_amplitude < threshold)
        {
            std::cout << "Peak was not found" << std::endl;
            i += m_kSize;
            continue;
        }

        float maximum = 0.0;
        float maximum_val = 0.0;

        for (int j = 0; j < signal_hilb.size(); j++)
        {
            if (signal_hilb[j] > threshold && signal_hilb[j] > maximum_val)
            {
                maximum = j;
                maximum_val = signal_hilb[j];
            }
            if (signal_hilb[j] < maximum_val && maximum_val > 0)
                break;
        }

        int peak_current = i + maximum;

        if (peaks.size() >= 3 && peak_current - peaks.back() < 0.2 * m_fs)
        {
            int peak_last = peaks.back();
            peaks.pop_back();

            QVector<int> peaksVector = peaks;
            int average_distance = CalcAverageDistance(peaksVector);
            int peak_second_to_last = peaks.back();

            int dist1 = abs(average_distance - (peak_second_to_last - peak_last));
            int dist2 = abs(average_distance - (peak_second_to_last - peak_current));

            if (dist1 <= dist2)
            {
                peaks.push_back(peak_last);
            }
            else
            {
                peaks.push_back(peak_current);
            }
        }
        else
        {
            peaks.push_back(peak_current);
        }

        i = peak_current + 1;
        max_amplitude_old = *max_amplitude;
    }
    return peaks;
}
