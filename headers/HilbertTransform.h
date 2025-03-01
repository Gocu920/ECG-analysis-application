/*
 * Sources:
 * A New QRS Detection Algorithm Based on the Hilbert Transform DS Benitez', PA Gaydecki, A Zaidi, AP Fitzpatric
 */

#ifndef ECG_ANALYZER_HILBERTTRANSFORM_H
#define ECG_ANALYZER_HILBERTTRANSFORM_H

#include <QVector>

/*
 * Example usage:
 * HilbertTransform hilbert = HilbertTransform();
 * QVector<float> electrocardiogram_signal = { * dane sygnału EKG * };
 * QVector<int> peaks = hilbert.GetPeaks(electrocardiogram_signal, 360);
 */
class HilbertTransform
{
public:
    HilbertTransform() = default;

    /*
     * @brief Function for finding R peaks using Hilbert Transform
     * @param[in] electrocardiogram_signal: EKG signal as QVector<float>
     * @param[in] fs: Sampling frequency of the EKG signal (default = 360 Hz)
     * @return Indexes of R peaks for the given signal as QVector<int>
     */
    QVector<int> GetPeaks(QVector<double> electrocardiogram_signal, int fs = 360);

private:
    static constexpr int m_kSize = 1024; // size of window in Hilbert Transform
    int m_fs; // electrocardiogram_signal sampling frequency

    /*
     * @brief Compute Hilbert Transform on a window of size HilbertTransform::m_kSize
     * @param[in] signal: EKG signal as QVector<float>
     * @param[in] first: First index to start
     * @return The result of the Hilbert Transform as QVector<float>
     */
    QVector<double> ComputeHilbertTransform(QVector<double> signal, int first) const;

    /*
     * @brief Compute the derivative of the EKG signal
     * @param[in, out] signal: EKG signal as QVector<float>
     * @return The derivative of the signal as QVector<float>
     */
    QVector<double> Derivative(QVector<double>& signal) const;

    /*
     * @brief Apply a band-pass filter to the EKG signal
     * @param[in] signal: EKG signal as QVector<float>
     * @param[in] fc1: Low cutoff frequency
     * @param[in] fc2: High cutoff frequency
     * @return The filtered signal as QVector<float>
     */
    QVector<double> Filter(const QVector<double>& signal, double fc1, double fc2) const;

    /*
     * @brief Calculate the average distance between peaks
     * @param[in] peaks: Indices of the peaks as QVector<int>
     * @return The average distance between peaks
     */
    int CalcAverageDistance(QVector<int>& peaks) const;

    /*
     * @brief Calculate the RMS value of the EKG signal
     * @param[in] signal: EKG signal as QVector<float>
     * @return The RMS value of the signal
     */
    float CalcRMSValue(QVector<double>& signal) const;

    /*
     * @brief Normalize the EKG signal
     * @param[in, out] v: EKG signal as QVector<float>
     */
    void Normalize(QVector<double>& v) const;

    /*
     * @brief Perform a convolution operation on two signals
     * @param[in] f: First signal as QVector<T>
     * @param[in] g: Second signal as QVector<T>
     * @return The result of the convolution as QVector<T>
     */
    template <typename T>
    QVector<T> Conv(const QVector<T>& f, const QVector<T>& g) const;

    /*
     * @brief Calculate the factorial of a number
     * @param[in] n: Number to calculate the factorial for
     * @return The factorial of n
     */
    float Factorial(int n) const;
};

#endif // ECG_ANALYZER_HILBERTTRANSFORM_H
