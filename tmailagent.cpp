#include "tmailagent.h"

TMailAgent::TMailAgent(const QString& accountId, QSqlDatabase & database, QObject *parent) :
    QObject(parent), AccountId(accountId), DataBase(database)
{
    authIndicator = false;
}

bool TMailAgent::startFetchAgentAndContact()
{
    getAgent();
    getMailContact();

}

//------------------------get contact mail----------------------------//

bool TMailAgent::getMailContact()
{

   if (!authIndicator)
   {
    if (!authenAgent())
        FUNC_ABORT ("Authentification not sucuess");

   }

   url = "https://e.mail.ru/addressbook";
    request.setUrl(url);
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    startRequest(getRequest, requestString );

   QRegExp regexjsonContact("\\{\"body\"\\:\\{\"contacts\"\\:(.)+\"status\"\\:\\d+\\}");

   QString jsonParseString;
   QStringList listEmailContacts;
   QStringList listNick;
   QStringList listDisplayName;
   QStringList listId;
   QStringList listPhones;
   QStringList listFio;

    if (regexjsonContact.indexIn(lastResponsAgentRequest) == -1)
          FUNC_ABORT("regex for contact not work");

    jsonParseString = regexjsonContact.cap(0);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonParseString.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QJsonObject jsonObjectContacts = jsonObject["body"].toObject();

    QJsonArray jsonArray = jsonObjectContacts["contacts"].toArray();
    qDebug() << jsonArray;

    foreach (const QJsonValue & value, jsonArray)
            {
                QJsonObject obj = value.toObject();

                //---------получение данных поля emails---------------------------------//
                QJsonArray jsonArrayEmail = obj["emails"].toArray();
                QString emails;
                    foreach (const QJsonValue & valueEmail, jsonArrayEmail)
                            emails.append(valueEmail.toString() + " ; ");
                listEmailContacts << emails;

                //---------получение данных поля name---------------------------------------//
               QString fio;
                QJsonObject jsonObjName = obj["name"].toObject();
               fio.append(jsonObjName.value("first").toString() + " ");
                fio.append(jsonObjName.value("last").toString());
                listFio << fio;


                //---------получение данных поля phone---------------------------------------//
                QString phone;
                QJsonArray jsonArrayPhones = obj["phones"].toArray();
                 foreach (const QJsonValue & valuePhone,  jsonArrayPhones)
                 {
                     QJsonObject obj = valuePhone.toObject();
                      phone.append(obj.value("phone").toString() + " ; ");

                 }
                 listPhones << phone;


                //---------получение данных поля nick---------------------------------------//
               listNick.append(obj["nick"].toString());

               //---------получение данных поля display_name------------------------------//
               listDisplayName.append(obj["display_name"].toString());

               //---------получение данных поля id------------------------------//
               listId.append(obj["id"].toString());

            }



    QList<QStringList> contactEmailContainer;
    contactEmailContainer.clear();

    contactEmailContainer.append( listEmailContacts);
    contactEmailContainer.append( listNick);
    contactEmailContainer.append( listDisplayName);
    contactEmailContainer.append( listId);
    contactEmailContainer.append( listPhones);
    contactEmailContainer.append( listFio);

   // сохранение контактов в базу данных
    saveContactEmailToDataBase(contactEmailContainer);


    return true;
}

//---------- сохранение контактов почты в базу данных ----------------..

bool TMailAgent::saveContactEmailToDataBase(QList<QStringList> &contactEmailContainer)
{

    // получаем список контактов с базы данных

    QStringList contactUidFromDatabase;
    contactUidFromDatabase.clear();

    QSqlQuery lquery;

    QString lcmd = "SELECT uid FROM contactsEmail WHERE accountId = " + AccountId;
    if (!lquery.exec(lcmd))
        FUNC_ALERT_ERROR("uid of contacts  select not got from database");

    while(lquery.next()) // если записи есть записываем их в contactFromDatabase
        contactUidFromDatabase << lquery.value(0).toString();


    // получаем список контактов с сервера
    QStringList listEmailContacts = contactEmailContainer[0];
    QStringList listNick = contactEmailContainer[1];
    QStringList listDisplayName = contactEmailContainer[2];
    QStringList listId = contactEmailContainer[3];
    QStringList listPhones = contactEmailContainer[4];
    QStringList listFio = contactEmailContainer[5];


    for (int i =0; i < listEmailContacts.size(); i++ )
    {
       // сверяем имеющиеся в базе контакты с полученными с сервера
        if (contactUidFromDatabase.contains(listId[i]))
           continue;

      lcmd = "INSERT INTO contactsEmail(email, fio, phones, uid, nick, displayName, accountId )"
              " VALUES(:email, :fio, :phones, :uid, :nick, :displayName, :accountId)";
      res = lquery.prepare(lcmd);

       lquery.bindValue(":email", listEmailContacts[i]);
       lquery.bindValue(":fio", listFio[i]);
       lquery.bindValue(":phones", listPhones[i]);
       lquery.bindValue(":uid", listId[i]);
       lquery.bindValue(":nick", listNick[i]);
        lquery.bindValue(":displayName", listDisplayName[i]);
       lquery.bindValue(":accountId", AccountId);

       if (!lquery.exec())
           FUNC_ALERT_ERROR("Email_Contact from server not writed to database");

    }



return true;

}



//-------------------------------------------------------------------//

long long int TMailAgent::parseAgentMessageResponse(const QString& contactEmail, const long long int & maxMsgIdFromDatabase)
{
    QString messageParseStr = lastResponsAgentRequest;

    //----------парсинг переписки в json формате---------------------//
    // создаем контейнеры для хранения данных сообщения агента
    long long int lastMsgId = 0;
    QStringList msgAgentId;
    QStringList msgAgentText;
    QByteArray msgAgentInbox;
    QStringList msgAgentDate;

    QJsonDocument jsonResponse = QJsonDocument::fromJson(messageParseStr.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArray = jsonObject["list"].toArray();
    foreach (const QJsonValue & value, jsonArray)
            {
                QJsonObject obj = value.toObject();
                msgAgentDate.append(obj["date"].toString());
                msgAgentId.append(obj["id"].toString());
                msgAgentInbox.append(obj["inbox"].toBool());
                msgAgentText.append(obj["text"].toString());
            }


    isLastJsonResponse = jsonObject.value("is_last").toBool();



    // запись в базу. Предварительно из базы вытаскиваем самый большой Id сообщения
    // и проверяем, если текущий ID меньше полученного, то в базe не пишем

   // long long int maxMsgIdFromDatabase = getMaxAgentMsgId(getAgentContactId(contactEmail)); // максимальный id сообщения в базе

    for (int i = 0; i < msgAgentId.size(); i++)
    {

        lastMsgId =  msgAgentId[i].toLongLong();
        // если id текущего сообщения меньше максимального, то сообщение
        //пропускаем и не записываем
       // long long int iMsg = msgAgentId[i].toLongLong();

        if (msgAgentId[i].toLongLong() <= maxMsgIdFromDatabase)
            continue;

        QSqlQuery lquery;
        QString localCmd = "INSERT INTO agentMessage(agentContactId, msgId, msgText, inbox, timestamp)"
                " VALUES(:agentContactId, :msgId, :msgText, :inbox, :timestamp)";

        res = lquery.prepare(localCmd);


         lquery.bindValue(":agentContactId", getAgentContactId(contactEmail) );
         lquery.bindValue(":msgId", msgAgentId[i].toLongLong());
         lquery.bindValue(":msgText",  msgAgentText[i] );
         lquery.bindValue(":inbox", (int)msgAgentInbox[i]);
         lquery.bindValue(":timestamp", msgAgentDate[i] );

         if (!lquery.exec())
             FUNC_ALERT_ERROR("Message from server not writed to database");



    }

    return lastMsgId;

}

bool TMailAgent::getNewAgentMessage(const QString& contactEmail)
{
   isLastJsonResponse = false;
   long long int maxMsgIdFromDatabase = getMaxAgentMsgId(getAgentContactId(contactEmail)); // максимальный id сообщения в базе

   // посылаем первый запрос на получение переписки
        QString hash = getHash();
        url = "https://webarchive.mail.ru/ajax/dialog?opponent_email=" + contactEmail + "&message_id=&sort=desc&hash=" + hash;
        request.setUrl(url);
        request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
        startRequest(getRequest, requestString );

        QString messageParseStr = lastResponsAgentRequest;


        long long int lastMsgId =  parseAgentMessageResponse (contactEmail, maxMsgIdFromDatabase);

       while (isLastJsonResponse == false)
       {
           // посылаем первый запрос на получение переписки
           QString hash = getHash();
           url = "https://webarchive.mail.ru/ajax/dialog?opponent_email=" + contactEmail + "&message_id=" + QString::number(lastMsgId) +"&sort=desc&hash=" + hash;
           request.setUrl(url);
           request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
           startRequest(getRequest, requestString );

           QString messageParseStr = lastResponsAgentRequest;

           lastMsgId =  parseAgentMessageResponse(contactEmail, maxMsgIdFromDatabase);

       }

   return true;


}

int TMailAgent::getAgentContactId(const QString& agentContactEmail)
{
    // получаем максимальный ID для сообщения агента - msgId
    QSqlQuery localQuery;
    cmd = "SELECT id FROM agentAccount WHERE agentContactEmail = \'" + agentContactEmail + "\'";
     if (!localQuery.exec(cmd))
         FUNC_ABORT("AgentContactId not got from database");

    // если id найден то возвращаем его
     if(localQuery.next())
        return localQuery.value(0).toInt();

}

long long int TMailAgent::getMaxAgentMsgId(const int & agentContactId)
{
    // получаем максимальный ID для сообщения агента - msgId
    QSqlQuery lquery;
    QString lcmd = "SELECT msgId FROM agentMessage WHERE agentContactId = " + QString::number(agentContactId) + " ORDER BY msgId DESC";
     if (!lquery.exec(lcmd))
         FUNC_ABORT("Max msgId not got from database");

    // если записи есть записываем их в maxMsgId
     if(!lquery.next())
         return 0;
     else
     {
        long long int x = lquery.value(0).toLongLong();
        return lquery.value(0).toLongLong();
     }


}


bool TMailAgent::authenAgent()
{
   // получение из базы username, password, domen,

    cmd = "SELECT account, password FROM accounts WHERE id = " + AccountId;
    if (!query.exec(cmd))
        FUNC_ABORT("Account select not got from database");

    if (!query.next())
        FUNC_ABORT("Account next not got from database");

    QString tmpUsername = query.value(0).toString();
    QString password = query.value(1).toString();
    int pos = tmpUsername.indexOf("@");

    QString domen = tmpUsername.remove(0,pos+1);
    QString username = query.value(0).toString().remove(pos,query.value(0).toString().size());


    myCookie = new TMyCookieJar();
    qnam.setCookieJar(myCookie);
    url = "https://auth.mail.ru/cgi-bin/auth?from=splash";

    requestString = "Domain=" + domen + "&Login=" + username + "&Password=" + password + "&new_auth_form=1&saveauth=1";
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.46 Safari/536.5");
    startRequest(postRequest, requestString );

    // проверить успешно ли прошла аутентификация
    // если успешно устанавливаем индикатор
    authIndicator = true;
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

    // сохраняем в базу данных контактный лист
    checkNewandSaveAgentContactsToDataBase(agentContactList);

    return agentContactList;

}


bool TMailAgent::checkNewandSaveAgentContactsToDataBase(QList<QStringList> &AgentContactList)
{
   //получаем из базы список имеющихся контактов агента
    QStringList contactFromDatabase;
    contactFromDatabase.clear();

    cmd = "SELECT agentContactEmail FROM agentAccount WHERE accountId = " + AccountId;
    if (!query.exec(cmd))
        FUNC_ABORT("Account select not got from database");

    while(query.next()) // если записи есть записываем их в contactFromDatabase
        contactFromDatabase << query.value(0).toString();

    // получаем списки контактов с сервера
    QStringList agentName = AgentContactList[0];
    QStringList agentEmail =  AgentContactList[1];


    for (int i =0; i < agentEmail.size(); i++ )
    {
       // сверяем имеющиеся в базе контакты с полученными с сервера
        if (contactFromDatabase.contains(agentEmail[i]))
           continue;

      cmd = "INSERT INTO agentAccount(agentContactName, agentContactEmail, accountId) VALUES(:agentContactName, :agentContactEmail, :accountId)";
      res = query.prepare(cmd);

       query.bindValue(":agentContactName", agentName[i]);
       query.bindValue(":agentContactEmail", agentEmail[i]);
       query.bindValue(":accountId", AccountId);

       if (!query.exec())
           FUNC_ALERT_ERROR("Contact from server not writed to database");

    }
    return true;

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

    QList<QStringList> contactList = getAgentContactList();
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
    QStringList contactEmailList;
    contactEmailList.clear();

     cmd = "SELECT agentContactEmail FROM agentAccount WHERE accountId = " + AccountId;
     if (!query.exec(cmd))
         FUNC_ABORT("Account select not got from database");

     while(query.next()) // если записи есть записываем их в  contactEmailList
          contactEmailList << query.value(0).toString();

    //contactEmailList << "testtestov101@mail.ru" << "night_post@mail.ru";

    foreach (QString contactEmail, contactEmailList)
          getNewAgentMessage(contactEmail);



/*
    // запрос необходим для становления куков. Эксперимент показал что для получения
    //переписки агента этот запрос не нужен, но на всякий слчай код оставлю
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


    qDebug() << "errorcode:" << errorcode;
  // qDebug() << "myCookie->getAllCookies():" << myCookie->getAllCookies();
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
