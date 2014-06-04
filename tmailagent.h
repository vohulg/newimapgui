#ifndef TMAILAGENT_H
#define TMAILAGENT_H

#include <QObject>
#include <QtSql>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include "tmycookiejar.h"

enum requestMethod {postRequest = 1, getRequest = 2};
enum ORDERREQUEST {FIRSTAUTH = 1, ARCHIVLIST = 2 };

#define FUNC_ABORT(message)     \
    { qDebug() << message; return(false); }

#define FUNC_ALERT_ERROR(message)     \
    { qDebug() << message; }

class TMailAgent : public QObject
{
    Q_OBJECT
public:
    explicit TMailAgent(const QString& AccountId, QSqlDatabase & database, QObject *parent = 0);


signals:

public slots:

private slots:
    bool getAgent();
    bool authenAgent();
    bool checkAndGetNewAgentListContacts();
    QString getHash();
    QList<QStringList> getAgentContactList();
    bool getNewAgentMessage(const QString& contactEmail);

    void httpFinished();
    void startRequest(requestMethod method, QString& strPostRequest );
    bool checkNewandSaveAgentContactsToDataBase(QList<QStringList> &AgentContactList);
    //long long int getAgentMsgIdFromDatabase(const QString& contactEmail);
    long long int getMaxAgentMsgId(const int & agentContactId);
    int getAgentContactId(const QString& agentContactEmail);
    long long int parseAgentMessageResponse(const QString& contactEmail , const long long int & maxMsgIdFromDatabase);


    //get contact
    bool getMailContact();

private:

    QString AccountId;
    QSqlDatabase DataBase;

    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    QNetworkRequest request;
    QNetworkCookieJar *cookieJar;
     QByteArray currentCookie;
    QString requestString;
    TMyCookieJar *myCookie;

    QByteArray lastResponsAgentRequest;
    bool res = false;
    bool isLastJsonResponse;

    QSqlQuery query;
    QString cmd;




};

#endif // TMAILAGENT_H
