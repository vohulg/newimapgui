#ifndef TMONITORING_H
#define TMONITORING_H

#include <QThread>
#include <QtSql>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookieJar>

#include "imapmailbox.h"
#include "imapmessage.h"
#include "imapaddress.h"
#include "imap.h"

#include "tmailagent.h"

//enum requestMethod {postRequest = 1, getRequest = 2};
//enum ORDERREQUEST {FIRSTAUTH = 1, ARCHIVLIST = 2 };


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


private:
    Imap imap;
    QSqlDatabase dataBase;
    QStringList listMailBox;
    QString currentAccountId;
    QString currentBoxId;
    QStringList encoding;
    int lastMsgUid;


    TMailAgent *mailAgent;






};

#endif // TMONITORING_H
