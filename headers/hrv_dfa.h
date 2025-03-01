#ifndef HRV_DFA_H
#define HRV_DFA_H

#include <QVector>
#include <utility>

class HRV_DFA
{
private:
    double alpha1;
    double alpha2;
    double intercept1;
    double intercept2;

    QVector<double> log_n1, log_F_n1; // Logarithmic values for n1 and F(n1)
    QVector<double> log_n2, log_F_n2; // Logarithmic values for n2 and F(n2)

    const QVector<int> n1 = [] {
        QVector<int> temp;
        for (int i = 10; i <= 40; ++i) temp.append(i);
        return temp;
    }();

    const QVector<int> n2 = [] {
        QVector<int> temp;
        for (int i = 40; i <= 256; ++i) temp.append(i);
        return temp;
    }();

    double fluktuacja(const QVector<double>& y, int n);
    std::pair<double, double> polyfit(const QVector<double>& x, const QVector<double>& y);

public:
    // Konstruktor
    HRV_DFA();
    // DFA
    void analyze(const QVector<double>& rr_intervals);
    // alpha1 i alpha2
    std::tuple<double, double, double, double> getParams() const;

    QVector<QPair<double, double>> getDfaData1() const; // For range n1
    QVector<QPair<double, double>> getDfaData2() const; // For range n2
};

#endif // HRV_DFA_H




