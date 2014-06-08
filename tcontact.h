#ifndef TCONTACT_H
#define TCONTACT_H

#include <QObject>

class TContact : public QObject
{
    Q_OBJECT
public:
    explicit TContact(const QString& accountId, QObject *parent = 0);

signals:

public slots:

private:
    QString AccountId;

};

#endif // TCONTACT_H
