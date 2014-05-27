#ifndef TMONITORING_H
#define TMONITORING_H

#include <QThread>
#include <QtSql>
#include <QNetworkAccessManager>



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
    bool saveToDataBaseHeader(ImapMailbox *mailbox, const QList<int>& messageList);
    bool saveToDataBaseBody(ImapMessage *message);
    bool getMessage(const QString& id, const QString& username, const QString& password);
    bool getAgent();
    void replyFinish(QNetworkReply* replay);

private:
    Imap imap;
    QSqlDatabase dataBase;
    QStringList listMailBox;
    QString currentAccountId;
    QString currentBoxId;
    QStringList encoding;
    int lastMsgUid;

};

#endif // TMONITORING_H
