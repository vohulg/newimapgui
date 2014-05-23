#ifndef TMONITORING_H
#define TMONITORING_H

#include <QThread>
#include <QtSql>



#include "imapmailbox.h"
#include "imapmessage.h"
#include "imapaddress.h"
#include "imap.h"


class TMonitoring : public QThread
{
    Q_OBJECT
public:
    explicit TMonitoring(QObject *parent = 0);
     bool setDatabase (QSqlDatabase &db);

signals:

public slots:
    void run();

private slots:
    bool checkNewFolder(const QString& id, QStringList& currentListMailBox);
    bool saveToDataBase(ImapMailbox *mailbox, const QList<int>& messageList);

private:
    Imap imap;
    QSqlDatabase dataBase;
    QStringList listMailBox;




};

#endif // TMONITORING_H
