#include "tmonitoring.h"

TMonitoring::TMonitoring(QObject *parent) :
    QThread(parent)
{

    encoding << "NoEncoding" << "UnknownEncoding" << "Utf7Encoding" << "Utf8Encoding" << "Base64Encoding" << "QuotedPrintableEncoding";
    lastMsgUid = -1;
    startRunMonitoring = QDateTime::currentDateTime();
    loopRepet = 120; // количество секунд через которые идет повторение

}

TMonitoring::~TMonitoring()
{


}

bool TMonitoring::setDatabase(QSqlDatabase &db)
 {
     dataBase = db;
     return true;
 }

void TMonitoring::run()
{
    abortMonitoring = false;
    // запускаем цикл через определенные промежутки времени
    while(true)
    {
        if (abortMonitoring)
            break;

        startLoop();
        sleep(loopRepet);
    }

}

void TMonitoring::abortLoopMonitoring()
{
  abortMonitoring = true;

}

// ------------- запск цикла мониторинга------- //

bool TMonitoring::startLoop()
{

    QSqlQuery query;
    query.exec("SELECT id, account, password, startMonitor, endMonitor, status FROM accounts ORDER BY startMonitor DESC;");

   //================обрабатываем каждый ящик =========================//

    while (query.next())
    {
        QString id = query.value(0).toString();
        QString username = query.value(1).toString();
        QString password = query.value(2).toString();
        QDateTime startTime = query.value(3).toDateTime();
        QDateTime endTime = query.value(4).toDateTime();
        currentAccountId = id;

        qint64 secStartTimeMinusStartRunMonitoring =  startTime.secsTo(startRunMonitoring);
        qint64 secEndTimeMinusStartRunMonitoring =  endTime.secsTo(startRunMonitoring);
        QDateTime currentDateTime = QDateTime::currentDateTime();

        // 1 вариант. Время запуска потока меньше начала назначенного старта мониторинга для данного аккаунта
        if (secStartTimeMinusStartRunMonitoring > 0)
        {
            qint64 secBetweenCurTimeAndStartTime = startTime.secsTo(currentDateTime);
            QTimer::singleShot(secBetweenCurTimeAndStartTime, this, SLOT(startMonitoring(id, username, password)));
        }

        // 2 вариант время запуска потока больше старта но меньше окончания назначенного времени
        else if (secEndTimeMinusStartRunMonitoring > 0)
            QTimer::singleShot(1, this, SLOT(startMonitoring(id, username, password)));

        // 3 вариант время запуска потока больше и начала и окончания назначенного времени
        // пропскаем этот аккаунт

        else if (secEndTimeMinusStartRunMonitoring < 0)
            continue;
    }

    return true;
}

bool TMonitoring::startMonitoring(const QString& id, const QString& username, const QString& password)
{
    // запуск на скачивание почты
    getMessage(id, username, password); // скачивание новой почты

    // запуск на скачивание агента и контактов почтового ящика
    mailAgent = new TMailAgent (currentAccountId, dataBase);
    mailAgent->startFetchAgentAndContact();

    return true;

}



bool TMonitoring::getMessage(const QString& id, const QString& username, const QString& password)
{
     qDebug() << "Monitoring starting....................";

    //=========initializ server value=========//
     QString host = "imap.mail.ru";
     quint16 port = 993;
     bool useSsl = true;
     Imap::LoginType loginType = Imap::LoginPlain;
     //============================================//

    if (!imap.connectToHost(host, port, useSsl))
        qDebug() << "not connected to server";


    if (!imap.login(username, password, loginType ))
        qDebug() << "not connected to mailbox";


    //-------получаем список ящиков для аккаунта---------//
    listMailBox = imap.list();

   //----------проверяем на наличие новых папок в ящике. Если появились новые папки добавляем их в базу-----------//
    if (!checkNewFolder(id, listMailBox))
        qDebug() << "folder not checked";

    // получаем последний uid сообщения. Если такого нет, значит в базе нет ни одного сообщения с аккаунта
    QSqlQuery query;
    QString cmd = "SELECT uid FROM headers WHERE accountId = " +  currentAccountId + " ORDER BY uid DESC";
    bool res = query.exec(cmd);


    if (res)
    {
        query.next();
        lastMsgUid = query.value(0).toInt();
    }

    /*
    else
         lastMsgUid = -1;

         */






    //================проходим по каждой папке, получаем сообщения и записываем их в базу =================================================================================//

     foreach (QString box, listMailBox)
     {
         ImapMailbox *mailbox = imap.select(box);
         if (mailbox == NULL)
              qDebug() << box <<" not selected";



         QList<int> messageList;
         if (lastMsgUid == -1)
         {
             messageList = imap.searchALL();
             qDebug() << box << " messageList:" << messageList;
         }
         else
         {
             messageList = imap.searchNew(QString("%1").arg(lastMsgUid));
             qDebug() << box << " messageList:" << messageList;
         }






        QSqlQuery query;

          QString cmd = "SELECT id FROM folderMap WHERE accountId = " +  currentAccountId + " AND folderName = \'" + box + "\'";
          bool res = query.exec(cmd);
         if (res)
         {
             query.next();
             currentBoxId = query.value(0).toString();

         }



         if (imap.fetch(mailbox, messageList) == NULL)
             qDebug() << box <<" not fetched";

         if (!saveToDataBaseHeader(mailbox,messageList))
              qDebug() << "Don't saved new messagу in mailbox";


         // Destroy Mailbox.
         delete mailbox;


    //return true;
   }
}


//----------------- сохраняем в базу заголовки писем-------------------//

bool TMonitoring::saveToDataBaseHeader(ImapMailbox *mailbox, const QList<int>& messageList)
{

      bool res = false;
       int cnt = messageList.count();

    foreach (int msgId, messageList) {
         ImapMessage *message = mailbox->findByUid(msgId);
         if (message == NULL) {
             qDebug() << "Message" << msgId << "Not Found.";
             continue;
         }


         if (!imap.fetchBodyStructure(message))
              qDebug() << "fetchBodyStructure() not worked";

            //==============save headers to databasе=============//


         QString to;
         QString cc;
         QString bcc;
         foreach (ImapAddress address, message->toAddresses())
             to +=  address.toString();

         foreach (ImapAddress address, message->ccAddresses())
             cc +=  address.toString();

         foreach (ImapAddress address, message->bccAddresses())
             bcc +=  address.toString();



         QSqlQuery query;
         QString cmd = "INSERT INTO headers(accountId, folderName, bcc, sent, subject, ref, сс, recieved, timezone, fr, komu, uid)"
                 " VALUES (:accountId, :folderName, :bcc, :sent, :subject, :ref, :cc, :recieved, :timezone, :fr, :komu, :uid)";


         res = query.prepare(cmd);
         query.bindValue(":accountId", currentAccountId);
         query.bindValue(":folderName", currentBoxId);
         query.bindValue(":subject", message->subject());
         query.bindValue(":ref",  message->reference());
         query.bindValue(":fr",message->fromAddress().toString());
         query.bindValue(":komu", to);
         query.bindValue(":cc",cc);
         query.bindValue(":bcc", bcc);
         query.bindValue(":recieved", message->received());
         query.bindValue(":sent", message->sent());
         query.bindValue(":timezone",  message->timeZone());
         query.bindValue(":uid",  message->uid().toInt());
         res =  query.exec();


         //=================== запись полей body ===============================//
        saveToDataBaseBody(message);


     }

    return true;

}

//====================== сохраняем в базу тела писем=========================//

 bool TMonitoring::saveToDataBaseBody(ImapMessage *message)
 {

     for (int i = 0; i < message->bodyPartCount(); ++i)
       {
        ImapMessageBodyPart *bodyPart = message->bodyPartAt(i);

        qDebug() << bodyPart->isAttachment() << bodyPart->bodyPart()
                 << bodyPart->fileName() << bodyPart->encoding() << bodyPart->contentType();

        if (!imap.fetchBodyPart(message, i))
           qDebug() << "fetchBodyPart not worked";

        //imap.setSeen(message->id(), true);
        qDebug() << bodyPart->data();
        qDebug() << "=======================================================";



        QSqlQuery query;
        QString cmd = "SELECT id FROM headers ORDER BY id DESC";
        bool res = query.exec(cmd);
        QString currentHeaderId;
        if (res)
        {
            query.next();
            currentHeaderId = query.value(0).toString();

        }



         cmd = "INSERT INTO body(headersId, accountId, isAttach, part, filename, encoding, contentType, data)"
                " VALUES (:headersId, :accountId, :isAttach, :part, :filename, :encoding, :contentType, :data)";

        res = query.prepare(cmd);
        query.bindValue(":headersId", currentHeaderId);
        query.bindValue(":accountId", currentAccountId);
        query.bindValue(":isAttach",  bodyPart->isAttachment());
        query.bindValue(":part", bodyPart->bodyPart());
        query.bindValue(":filename", bodyPart->fileName());
        query.bindValue(":encoding", encoding[bodyPart->encoding()]);
        query.bindValue(":contentType", bodyPart->contentType());
        query.bindValue(":data", bodyPart->data());
        res =  query.exec();



    }

     return true;
 }



bool TMonitoring::checkNewFolder(const QString& accountId, QStringList& currentListMailBox)
    {

        bool res = false;
        QSqlQuery query;
        QString cmd = QString("SELECT folderName FROM folderMap WHERE accountId = %1;").arg(accountId);
        res = query.exec(cmd);

        if (!query.next()) // no folder in database for this account
              qDebug() << "no folder in database for this account";

        else
        {
            query.previous();
            while (query.next())
            {
                QString folderName = query.value(0).toString();
                if (currentListMailBox.contains(folderName))
                {
                    currentListMailBox.removeOne(folderName);
                    continue;
                }

                else
                {
                    currentListMailBox << folderName;

                }
            }

        }

        foreach (QString box, currentListMailBox)
        {
            QSqlQuery queryAddFolder;
            cmd = "INSERT INTO folderMap(accountId, folderName) VALUES (:accountId, :folderName );";
            queryAddFolder.prepare(cmd);
            queryAddFolder.bindValue(":accountId", accountId);
            queryAddFolder.bindValue(":folderName", box);
            res = queryAddFolder.exec();


        }


        //====== get list mailbox from database after writing new folder ===//

        cmd = QString("SELECT folderName FROM folderMap WHERE accountId = %1;").arg(accountId);
        res = query.exec(cmd);
        listMailBox.clear();
        while (query.next())
        {
            listMailBox << query.value(0).toString();
        }


    return true;

    }

