#ifndef HEART_CLASS_H
#define HEART_CLASS_H

#include <QMainWindow>

class HEART_CLASS : public QMainWindow
{
    Q_OBJECT

public:

    HEART_CLASS(); // konstruktor

    void classifyQRS(const QVector<double>& ecgSignal, QVector<int> qrs_onset_idx, QVector<int> qrs_end_idx);
    QVector<QString> getClassifications();

private:

    struct Motif {
        double bitSave;
        QVector<double> center;
    };

    struct AnomalyDetectionResult {
        QVector<QVector<double>> anomalies;
        QVector<int> anomalyIndices;
        QVector<double> center;
    };

    QVector<double> ecgSignal;
    double samplingRate = 360.0;
    int motifLength;
    QVector<QString> classificationResults;

    AnomalyDetectionResult detectAnomalies(const QVector<double>& ecgSignal);
    Motif motifDiscoveryAlgorithm(const QVector<double>& signal, double samplingFrequency);
    unsigned int calculateStartingLength(double samplingFrequency);
    QVector<QVector<double>> extractSubsequences(const QVector<double>& signal, int m);
    QVector<QVector<double>> zNormalize(const QVector<QVector<double>>& subsequences);
    std::tuple<QVector<double>, QVector<double>, double> findMostSimilarPair(const QVector<QVector<double>>& subsequences);
    double calculateBitsave(const QVector<double>& A, const QVector<double>& B, double dist);
    QVector<double> findNextNeighbor(const QVector<double>& signal, const QVector<double>& center, int m);
    Motif addToGroup(const Motif& group, const QVector<double>& next);
};

#endif // HEART_CLASS_H
