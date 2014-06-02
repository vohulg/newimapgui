#include "tmailagent.h"

TMailAgent::TMailAgent(const QString& username, const QString& domen, const QString& password, QObject *parent) :
    QObject(parent), Username(username), Domen(domen), Password(password)
{
  getAgent();

}

bool TMailAgent::getNewAgentMessage(const QString& contactEmail)
{

        // получаем содержание переписки
        QString hash = getHash();
        url = "https://webarchive.mail.ru/ajax/dialog?opponent_email=" + contactEmail + "&message_id=&sort=desc&hash=" + hash;
        request.setUrl(url);
        request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
        startRequest(getRequest, requestString );

        QString messageParseStr = lastResponsAgentRequest;
        qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
        qDebug() << "url:" << url << "\n";
        qDebug() << "lastResponsAgentRequest:" << lastResponsAgentRequest;
        qDebug() << "****************************************";

        //----------парсинг переписки в json формате---------------------//
        // создаем контейнеры для хранения данных сообщения агента
        QStringList msgAgentId;
        QStringList msgAgentText;
        QStringList msgAgentInbox;
        QStringList msgAgentDate;


        QJsonDocument jsonResponse = QJsonDocument::fromJson(messageParseStr.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
        QJsonArray jsonArray = jsonObject["list"].toArray();

        foreach (const QJsonValue & value, jsonArray)
                {
                    QJsonObject obj = value.toObject();
                    msgAgentId.append(obj["id"].toString());
                    msgAgentText.append(obj["text"].toString());
                    msgAgentInbox.append(obj["inbox"].toString());
                    msgAgentDate.append(obj["date"].toString());

                }

        // запись в базу. Предварительно из базы вытаскиваем самый большой Id сообщения
        // и проверяем, если текущий ID меньше полученного, то в баз не пишем

        for (int i = 0; i < msgAgentId.size(); i++)
            // проверка и запись в базу




    return true;


}



bool TMailAgent::authenAgent()
{
    myCookie = new TMyCookieJar();
    qnam.setCookieJar(myCookie);
    url = "https://auth.mail.ru/cgi-bin/auth?from=splash";

    requestString = "Domain=mail.ru&Login=testov-79&Password=testtest&new_auth_form=1&saveauth=1";
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    startRequest(postRequest, requestString );

    // проверить успешно ли прошла аутентификация
    return true;
}

QList<QStringList> TMailAgent::getAgentContactList()
{
    url = "https://webarchive.mail.ru/iframe?history_enabled=1";
    request.setUrl(url);
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    startRequest(getRequest, requestString );

    QString parseStr = lastResponsAgentRequest;

    QString contactParseStr;

    QRegExp regexContactAgent("contacts: (.)+\\}\\;");
    if (regexContactAgent.indexIn(parseStr) != -1)
           contactParseStr = regexContactAgent.cap(0).remove("contacts: ");
    else
        qDebug() << "RegexContact not match";

    // создаем контейнер для хранения контактов агента
    QList<QStringList> agentContactList;
    agentContactList.clear();
    QStringList agentName;
    QStringList agentEmail;
    agentContactList.append(agentName);
    agentContactList.append(agentEmail);

    QJsonDocument jsonResponse = QJsonDocument::fromJson(contactParseStr.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArray = jsonObject["list"].toArray();

    foreach (const QJsonValue & value, jsonArray)
            {
                QJsonObject obj = value.toObject();
                agentContactList[0].append(obj["name"].toString());
                agentContactList[1].append(obj["email"].toString());

            }

    return agentContactList;

}

QString TMailAgent::getHash()
{

    // запрос для получения списка контактов агента и хэша.
    //Хэш используется для просмотра архива в следующем запросе

    url = "https://webarchive.mail.ru/iframe?history_enabled=1";
    request.setUrl(url);
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    startRequest(getRequest, requestString );

    // -----------------получаем hash и список контактов агента------------------------//
    QString parseStr = lastResponsAgentRequest;
    QString hashStr = "nohash";

    QRegExp regexHash("hash: \"(\\w)+\"");

    if (regexHash.indexIn(parseStr) != -1)
    {
           hashStr = regexHash.cap(0).remove("hash: ");
           hashStr = hashStr.remove("\"");
    }
    else
        qDebug() << "RegexHash not match";

   return hashStr;

}

bool TMailAgent::checkAndGetNewAgentListContacts()
{
    //получаем из базы список контактов агента
    //........................code........................


    // получаем список контактов с сервера и хэш для следующего запроса на получение содержания переписки

    QList<QStringList> contactList = getAgentContactList();


    // сверяем список контактов на сервере и в базе. Если в базе нет контакта,
    //а на сервере есть то добавляем в базу
    // если в базе есть а на сервере нет, ничего не делаем
    //..............code..........

    return true;
}

bool TMailAgent::getAgent()
{

    if (!authenAgent())
    {
        qDebug() << "Authentification not secuess";
        return false;
    }

    if (!checkAndGetNewAgentListContacts())
    {
        qDebug() << "Getting agent contact list not secuess";
        return false;
    }

    // -------- получаем из базы список контактов и запускаем скачивание новых писем для данных контактов
    //...........code...............
    QStringList contactEmailList;
    contactEmailList << "testtestov101@mail.ru" << "night_post@mail.ru";
    foreach (QString contactEmail, contactEmailList)
          getNewAgentMessage(contactEmail);



/*
    // запрос необходим для становления куков. Эксперимент показал что для получения переписки агента этот запрос не нужен, но на всякий слчай код оставлю
   url = "https://e.mail.ru/agent/archive";
   request.setUrl(url);
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    startRequest(getRequest, requestString );

 */





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
