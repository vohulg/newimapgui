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
#include "tcontact.h"

//enum requestMethod {postRequest = 1, getRequest = 2};
//enum ORDERREQUEST {FIRSTAUTH = 1, ARCHIVLIST = 2 };

#define NO_START_MONITORING -1
#define NO_MSG_IN_FOLDER -1

const char * const base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

   enum
      {
        UNDEFINED       = 64,
        MAXLINE         = 76,
        UTF16MASK       = 0x03FFUL,
       UTF16SHIFT      = 10,
       UTF16BASE       = 0x10000UL,
        UTF16HIGHSTART  = 0xD800UL,
       UTF16HIGHEND    = 0xDBFFUL,
       UTF16LOSTART    = 0xDC00UL,
       UTF16LOEND      = 0xDFFFUL
      } EUTF7;




class TMonitoring : public QThread
{
    Q_OBJECT
public:
    explicit TMonitoring(QObject *parent = 0);
    ~TMonitoring();
     bool setDatabase (QSqlDatabase &db);

signals:

public slots:
    void run();

private slots:
    bool checkNewFolder(const QString& id, QStringList& currentListMailBox);
    bool saveToDataBaseHeader(ImapMessage *message);
    bool saveToDataBaseBody( ImapMessageBodyPart *bodyPart);
    bool getMessage(const QString& id, const QString& username, const QString& password);
    bool startMonitoring(const QString& id, const QString& username, const QString& password);
    bool startLoop();
    void abortLoopMonitoring();
    unsigned long checkSchedule(const QDateTime& startTime, const QDateTime& endTime);
    bool connectToHost(const QString& id, const QString& username, const QString& password);

private:
    Imap imap;
    QSqlDatabase dataBase;
    QStringList listMailBox;
    QString currentAccountId;
    QString currentBoxId;
    QStringList encoding;
    int lastMsgUid;
    unsigned long loopRepet;
    bool abortMonitoring;
    bool parse_folder(const QString& box);
    ImapMessage * get_message_header(ImapMailbox *mailbox, int msgId);
    bool parse_message_list(ImapMailbox *mailbox, const QList<int>& messageList);
    ImapMessageBodyPart * get_message_body(ImapMessage *message,int i);



    QDateTime startRunMonitoring;


    TMailAgent *mailAgent;
    TContact *contact;

     QString imapUTF7ToUnicode(const QString & input);






};

#endif // TMONITORING_H
