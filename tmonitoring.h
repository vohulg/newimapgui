#ifndef TMONITORING_H
#define TMONITORING_H

#include <QThread>

class TMonitoring : public QThread
{
    Q_OBJECT
public:
    explicit TMonitoring(QObject *parent = 0);

signals:

public slots:

};

#endif // TMONITORING_H
