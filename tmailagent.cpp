#include "tmailagent.h"

TMailAgent::TMailAgent(const QString& username, const QString& domen, const QString& password, QObject *parent) :
    QObject(parent), Username(username), Domen(domen), Password(password)
{
  getAgent();
  jsonParse();
}

void TMailAgent::jsonParse()
{
   //QFile file("webarchive.mail.ru");
   //if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
     //  qDebug() << "file not created";
   //QString parseStr = file.readAll();

    QString parseStr = lastResponsAgentRequest;
    QString contactParseStr;
    QString hashStr;

    // -----------------получаем hash и список контактов агента------------------------//
    QRegExp regexContactAgent("contacts: (.)+\\}\\;");
    QRegExp regexHash("hash: \"(\\w)+\"");

    if (regexContactAgent.indexIn(parseStr) != -1)
           contactParseStr = regexContactAgent.cap(0).remove("contacts: ");
    else
        qDebug() << "RegexContact not match";

    if (regexHash.indexIn(parseStr) != -1)
    {
           hashStr = regexHash.cap(0).remove("hash: ");
           hashStr = hashStr.remove("\"");
    }
    else
        qDebug() << "RegexHash not match";


    QStringList agentName;
    QStringList agentEmail;

    QJsonDocument jsonResponse = QJsonDocument::fromJson(contactParseStr.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArray = jsonObject["list"].toArray();

    foreach (const QJsonValue & value, jsonArray)
            {
                QJsonObject obj = value.toObject();

                agentName.append(obj["name"].toString());
                agentEmail.append(obj["email"].toString());
            }


    foreach (QString agentEmailItem, agentEmail)
    {
        url = "https://webarchive.mail.ru/ajax/dialog?opponent_email=" + agentEmailItem + "&message_id=&sort=desc&hash=" + hashStr;
        request.setUrl(url);
        request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
        startRequest(getRequest, requestString );

        qDebug() << "url:" << url << "\n";
        qDebug() << "lastResponsAgentRequest:" << lastResponsAgentRequest;
        qDebug() << "****************************************";

    }


}

// получение агента

bool TMailAgent::getAgent()
{
    myCookie = new TMyCookieJar();
    qnam.setCookieJar(myCookie);
    url = "https://auth.mail.ru/cgi-bin/auth?from=splash";

    requestString = "Domain=mail.ru&Login=testov-79&Password=testtest&new_auth_form=1&saveauth=1";
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    startRequest(postRequest, requestString );


    url = "https://e.mail.ru/agent/archive";
    request.setUrl(url);
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    startRequest(getRequest, requestString );

    url = "https://webarchive.mail.ru/iframe?history_enabled=1";
    request.setUrl(url);
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    startRequest(getRequest, requestString );


    return true;

}

//-----------------------------------------------------------------

void TMailAgent::startRequest(requestMethod method, QString& strPostRequest )
{
    QEventLoop loopGet;
    QTimer::singleShot(60000, &loopGet, SLOT(quit())); // таймаут минута на запрос


    if (method == postRequest)
        reply = qnam.post(request,strPostRequest.toUtf8() );
    else if (method == getRequest)
        reply = qnam.get(request);

    else
    {
        qDebug() << "HttpMethod not sended";
        return;
    }
    connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
    connect(reply, SIGNAL(finished()), &loopGet, SLOT(quit()));

    loopGet.exec();

}

void TMailAgent::httpFinished()
{

   lastResponsAgentRequest =  reply->readAll();


    QString filename = url.host();
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        qDebug() << "file not created";
    QTextStream out(&file);

    int errorcode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute).toInt();

   // if (!reply->rawHeader("Set-Cookie").isEmpty())
     //      currentCookie = currentCookie.insert(0,reply->rawHeader("Set-Cookie"));

    qDebug() << "errorcode:" << errorcode;
    //qDebug() << "currentCookie:"<< currentCookie;
    qDebug() << "location:" << reply->rawHeader("Location");
    //qDebug() << "allreplay:" << lastResponsAgentRequest;
    out << lastResponsAgentRequest;
    file.close();
    qDebug() << "================================================";

    if (!reply->rawHeader("Location").isEmpty())
    {
        url = reply->rawHeader("Location");
        request.setUrl(url);
        request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
        startRequest(getRequest, requestString );


    }

    //QList<QNetworkCookie> listCookie = myCookie->getAllCookies();
    //listCookie.append();
    //qDebug() << listCookie[0].name();



    reply->deleteLater();
    reply = 0;


}

void TMailAgent::httpReadyRead()
{

    //cookieJar->setAllCookies(reply->rawHeader("Set-Cookie"));

    //qDebug() << reply->rawHeader("Set-Cookie");
    //qDebug() << reply->readAll();
}


/*
void TMonitoring::replyFinish(QNetworkReply* reply)
{

    QString answer = QString::fromUtf8(reply->readAll());
        qDebug() << reply->rawHeader("Set-Cookie");
    //qDebug() << answer;
  //int errorcode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute).toInt();
   /* qDebug() << reply->rawHeader("Location");
*/
    /*
    QVariant varRedirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    QUrl urlRedirect = varRedirect.toUrl();
     if(!urlRedirect.isEmpty())
       {
          qDebug() << "Redirection url: " << urlRedirect.toString();
          //QUrl getUrl(QString("https://e.mail.ru/messages/inbox/?back=1"));
          QNetworkRequest getrequest(urlRedirect);
          pManager->get(getrequest);
          // redirect to urlRedirect ...
       }



            reply->deleteLater();
}

*/
