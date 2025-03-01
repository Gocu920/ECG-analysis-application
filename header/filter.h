#ifndef FILTER_H
#define FILTER_H
#include <QObject>
#include <QMainWindow>

class Filter:public QMainWindow
{
    Q_OBJECT
public:
    Filter();

    virtual ~Filter() {}

    virtual QVector<double> applyFilter(const QVector<double>& data);
};

#endif // FILTER_H
