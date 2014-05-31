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

class TMailAgent : public QObject
{
    Q_OBJECT
public:
    explicit TMailAgent(const QString& username, const QString& domen, const QString& password, QObject *parent = 0);

signals:

public slots:

private slots:
    bool getAgent();
    void httpFinished();
    void httpReadyRead();
    void startRequest(requestMethod method, QString& strPostRequest );
    void jsonParse();

private:
    QString Username;
    QString Domen;
    QString Password;

    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    QNetworkRequest request;
    QNetworkCookieJar *cookieJar;
    bool httpRequestAborted;
    int httpGetId;
    QByteArray currentCookie;
    QString requestString;
    TMyCookieJar *myCookie;

    QByteArray lastResponsAgentRequest;



};

#endif // TMAILAGENT_H
