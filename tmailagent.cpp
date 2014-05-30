#include "tmailagent.h"

TMailAgent::TMailAgent(const QString& username, const QString& domen, const QString& password, QObject *parent) :
    QObject(parent), Username(username), Domen(domen), Password(password)
{
  getAgent();
}

// получение агента

bool TMailAgent::getAgent()
{
    url = "https://auth.mail.ru/cgi-bin/auth?from=splash";
    requestString = "Domain=mail.ru&Login=testov-79&Password=testtest&new_auth_form=1&saveauth=1";
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
   // cookieJar = new QNetworkCookieJar();
   // qnam.setCookieJar(cookieJar);
    myCookie = new TMyCookieJar();
    qnam.setCookieJar(myCookie);
    startRequest(postRequest, requestString );


    url = "https://e.mail.ru/agent/archive";
    request.setUrl(url);
    //request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    request.setRawHeader("Cookie", currentCookie);

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

   QString filename = url.host();
   //bool ret = QDir::setCurrent("C:/");
   //QString currentPath = QDir::currentPath();
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        qDebug() << "file not created";
    QTextStream out(&file);
   // out << "The magic number is: " << 49 << "\n";

    int errorcode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (!reply->rawHeader("Set-Cookie").isEmpty())
           currentCookie = currentCookie.insert(0,reply->rawHeader("Set-Cookie"));

    qDebug() << "errorcode:" << errorcode;
    qDebug() << "currentCookie:"<< currentCookie;
    qDebug() << "location:" << reply->rawHeader("Location");
    QByteArray respons =  reply->readAll();
    qDebug() << "allreplay:" << respons;
    out << respons;
    file.close();
    qDebug() << "================================================";

    if (!reply->rawHeader("Location").isEmpty())
    {
        url = reply->rawHeader("Location");
        request.setUrl(url);
        //request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
        request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
        request.setRawHeader("Cookie", currentCookie);

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
