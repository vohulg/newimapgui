#include "tmonitoring.h"

TMonitoring::TMonitoring(QObject *parent) :
    QThread(parent)
{

    encoding << "NoEncoding" << "UnknownEncoding" << "Utf7Encoding" << "Utf8Encoding" << "Base64Encoding" << "QuotedPrintableEncoding";
    lastMsgUid = -1;
    startRunMonitoring = QDateTime::currentDateTime();
    loopRepet = 10; // количество секунд через которые идет повторение

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
   //for debug
    startLoop();


    abortMonitoring = false;
    // запускаем цикл через определенные промежутки времени
    /*
    while(true)
    {
        if (abortMonitoring)
            break;

            startLoop();
        sleep(loopRepet);
    }
*/
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
        bool isActive = query.value(5).toBool();
        currentAccountId = id;

        if (isActive == false)
            continue;


        // проверка расписания
       unsigned long sleepValue = checkSchedule(startTime, endTime);

       if (sleepValue == NO_START_MONITORING)
           continue;
       else
       {
           qDebug() << "Sleeping staus.Monitoring is in waiting mode............................" ;
           sleep(sleepValue);

           qDebug() << "Monitoring is started............................" ;
           startMonitoring(id, username, password);
           qDebug() << "Monitoring is ended..............................." ;
           continue;
       }

    }

    return true;
}

// проверка расписания
unsigned long  TMonitoring::checkSchedule(const QDateTime& startTime, const QDateTime& endTime)
{
    unsigned long sleepValue = 0;

    qint64 secStartTimeMinusStartRunMonitoring = startRunMonitoring.secsTo(startTime);
    qint64 secEndTimeMinusStartRunMonitoring = startRunMonitoring.secsTo(endTime);
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 1 вариант. Время запуска потока меньше начала назначенного старта мониторинга для данного аккаунта
    if (secStartTimeMinusStartRunMonitoring > 0)
            sleepValue = currentDateTime.secsTo(startTime);

    // 2 вариант время запуска потока больше старта но меньше окончания назначенного времени
    else if (secEndTimeMinusStartRunMonitoring > 0)
        sleepValue = 0;

    // 3 вариант время запуска потока больше и начала и окончания назначенного времени
    // пропускаем этот аккаунт

    else if (secEndTimeMinusStartRunMonitoring < 0)
        sleepValue = NO_START_MONITORING;

    return sleepValue;

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


// скачивание почты
bool TMonitoring::getMessage(const QString& id, const QString& username, const QString& password)
{
     if (!connectToHost(id, username, password))
            return false;


    //-------получаем список ящиков для аккаунта---------//
    listMailBox = imap.list();

   //----------проверяем на наличие новых папок в ящике. Если появились новые папки добавляем их в базу-----------//
    if (!checkNewFolder(id, listMailBox))
        qDebug() << "folder not checked";

    // получаем последний uid сообщения. Если такого нет, значит в базе нет ни одного сообщения с аккаунта
    QSqlQuery query;
    QString cmd = "SELECT uid FROM headers WHERE accountId = " +  currentAccountId + " ORDER BY uid DESC";
    bool res = query.exec(cmd);

    if (!query.next())
        lastMsgUid = NO_MSG_IN_FOLDER; // в базе нет почтовых сообщений для этого ящика

    else
    {
      // получаем uid последнего скачанного сообщения в базе для текущего ящика
        query.previous();
      lastMsgUid = query.value(0).toInt();
    }


    //================проходим по каждой папке, получаем сообщения и записываем их в базу =================================================================================//

     foreach (QString box, listMailBox)
        {
                parse_folder(box);
        }

}
//--------------парсим ящик----------------------------//
 bool TMonitoring::parse_folder(const QString& box )
 {
     ImapMailbox *mailbox = imap.select(box);
     if (mailbox == NULL)
     {
          qDebug() << box <<" not selected";
          return false;
     }


     QList<int> messageList;
     if (lastMsgUid == NO_MSG_IN_FOLDER)
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


     // извлечение почты
     if (imap.fetch(mailbox, messageList) == NULL)
     {
         qDebug() << box <<" not fetched";
         return false;
     }

    // get_message(mailbox,messageList);
    // ImapMessage * get_message(ImapMailbox *mailbox, const QList<int>& messageList);

     if (!parse_message_list(mailbox,messageList))
          qDebug() << "Don't saved new messagу in mailbox";


     // Destroy Mailbox.
     delete mailbox;

     return true;

 }

 //--------------parse message list--------------------------------------
bool TMonitoring::parse_message_list(ImapMailbox *mailbox, const QList<int>& messageList)
 {
    int cnt = messageList.count();

    foreach (int msgId, messageList)
    {
          ImapMessage *message = get_message_header(mailbox,msgId); // получение заголовков сообщения
           if (message == NULL)
               continue;
           else
             saveToDataBaseHeader(message); // запись в базу полей заголовков сообщения

           // получение body. Body может сомтоять из нескольких частей
           // Поэтому в цикле перебираем все части body и каждую отдельно записываем в базу
           for (int i = 0; i < message->bodyPartCount(); ++i)
             {
                ImapMessageBodyPart *bodyPart = get_message_body(message, i);
                if (bodyPart)
                    saveToDataBaseBody(bodyPart);
             }


    }

    return true;
 }

 //----------извлекаем сообщение------------------
ImapMessage * TMonitoring::get_message_header(ImapMailbox *mailbox, int msgId)
 {
     ImapMessage *message = NULL;
     message = mailbox->findByUid(msgId);
     if (message == NULL) {
         qDebug() << "Message" << msgId << "Not Found.";

     }

     if (!imap.fetchBodyStructure(message))
          qDebug() << "fetchBodyStructure() not worked";

     return message;

 }

//----------------- сохраняем в базу заголовки писем-------------------//
bool TMonitoring::saveToDataBaseHeader(ImapMessage *message)
{

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


         bool res = query.prepare(cmd);
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

    return true;

}

//-------------получение body сообщения-----------------
ImapMessageBodyPart * TMonitoring::get_message_body(ImapMessage *message,int i)
{
    ImapMessageBodyPart *bodyPart = NULL;
    bodyPart = message->bodyPartAt(i);

    qDebug() << bodyPart->isAttachment() << bodyPart->bodyPart() << bodyPart->fileName() << bodyPart->encoding() << bodyPart->contentType();


    if (!imap.fetchBodyPart(message, i))
       qDebug() << "fetchBodyPart not worked";

    //imap.setSeen(message->id(), true);

    // проверяем кодировку на utf7 и utf-8
    QString data;
    data.clear();
    if (bodyPart->encoding() == ImapMessageBodyPart::Utf7Encoding || bodyPart->encoding() == ImapMessageBodyPart::Utf8Encoding )
    {
        QTextCodec *codec = QTextCodec::codecForName(bodyPart->charset().toLatin1());
        data = codec->toUnicode(bodyPart->data());
        bodyPart->setDataStr(data);

    }
    else
        data.append(bodyPart->data());

    // проверяем закодировано ли имя файла, если да то разкодируем
    QString fname = bodyPart->fileName() ;
    if (fname.startsWith("=?"))
    {
       fname = imap.decode(fname);
       bodyPart->setFileName(fname);
    }

    return bodyPart;
}


//====================== сохраняем в базу тела писем=========================//
bool TMonitoring::saveToDataBaseBody( ImapMessageBodyPart *bodyPart)
 {

        QSqlQuery query;
        QString cmd = "SELECT id FROM headers ORDER BY id DESC";
        bool res = query.exec(cmd);
        QString currentHeaderId;
        if (res)
        {
            query.next();
            currentHeaderId = query.value(0).toString();

        }

         cmd = "INSERT INTO body(headersId, accountId, isAttach, part, filename, encoding, contentType, data, charset)"
                " VALUES (:headersId, :accountId, :isAttach, :part, :filename, :encoding, :contentType, :data, :charset)";

        res = query.prepare(cmd);
        query.bindValue(":headersId", currentHeaderId);
        query.bindValue(":accountId", currentAccountId);
        query.bindValue(":isAttach",  bodyPart->isAttachment());
        query.bindValue(":part", bodyPart->bodyPart());
        query.bindValue(":filename", bodyPart->fileName());
        query.bindValue(":encoding", encoding[bodyPart->encoding()]);
        query.bindValue(":contentType", bodyPart->contentType());
        query.bindValue(":data", bodyPart->dataStr());
        query.bindValue(":charset", bodyPart->charset());
        res =  query.exec();

     return true;
 }


 // подключаемся к хосту и аккаунту
  bool  TMonitoring::connectToHost(const QString& id, const QString& username, const QString& password)
  {

      //=========initializ server value=========//
      QString host = "imap.mail.ru";
      quint16 port = 993;
      bool useSsl = true;
      Imap::LoginType loginType = Imap::LoginPlain;
      //============================================//

     if (!imap.connectToHost(host, port, useSsl))
     {
         qDebug() << "not connected to server";
         return false;
     }
     else
         qDebug() << "Connected to server seccuessfull";


     if (!imap.login(username, password, loginType ))
     {
         qDebug() << "not connected to account " << username ;
         return false;
     }
     else
         qDebug() << "Connected to account " << username << "seccuessfull";

     return true;

  }



 // проверка на наличие новой почты
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

 //====================================

QString  TMonitoring::imapUTF7ToUnicode(const QString & input)
      {

       unsigned char c, i, bitcount;
       unsigned long ucs4, utf16, bitbuf;
        unsigned char base64[256],utf8[6];
       unsigned long srcPtr = 0;

        QByteArray output;
        QByteArray src = input.toLatin1();

        // Initialize modified base64 decoding table.

        memset(base64, UNDEFINED, sizeof(base64));

      // for (i = 0; i < sizeof(base64chars); ++i)
      for (i = 0; i < 64; ++i)
        {
          base64[(int)base64chars[i]] = i;
        }


        // Loop until end of string.
       while (srcPtr < src.length())
        {
          c = src[(int)srcPtr++];

          // Deal with literal characters and "&-".

          if (c != '&' || src[(int)srcPtr] == '-')
          {
            // Encode literally.

            output += c;

            // Skip over the '-' if this is an "&-" sequence.

           if (c == '&')
             srcPtr++;
         }
         else
         {
           // Convert modified UTF-7 -> UTF-16 -> UCS-4 -> UTF-8 -> HEX.
           bitbuf = 0;
           bitcount = 0;
           ucs4 = 0;

           while ((c = base64[(unsigned char) src[(int)srcPtr]]) != UNDEFINED)
          {

             ++srcPtr;
            bitbuf = (bitbuf << 6) | c;
            bitcount += 6;

             // Enough bits for an UTF-16 character ?

             if (bitcount >= 16)
           {
             bitcount -= 16;
              utf16 = (bitcount ? bitbuf >> bitcount : bitbuf) & 0xffff;

              // Convert UTF16 to UCS4.

               if (utf16 >= UTF16HIGHSTART && utf16 <= UTF16HIGHEND)
               {
                 ucs4 = (utf16 - UTF16HIGHSTART) << UTF16SHIFT;
                continue;
             }
              else if (utf16 >= UTF16LOSTART && utf16 <= UTF16LOEND)
              {
                 ucs4 += utf16 - UTF16LOSTART + UTF16BASE;
               }
             else
               {
                ucs4 = utf16;
              }

              // Convert UTF-16 range of UCS4 to UTF-8.

               if (ucs4 <= 0x7fUL)
               {
                utf8[0] = ucs4;
               i = 1;
               }
              else if (ucs4 <= 0x7ffUL)
               {
                 utf8[0] = 0xc0 | (ucs4 >> 6);
                utf8[1] = 0x80 | (ucs4 & 0x3f);
                 i = 2;
               }
              else if (ucs4 <= 0xffffUL)
               {
                utf8[0] = 0xe0 | (ucs4 >> 12);
                utf8[1] = 0x80 | ((ucs4 >> 6) & 0x3f);
                 utf8[2] = 0x80 | (ucs4 & 0x3f);
                 i = 3;
              }
               else
              {
                 utf8[0] = 0xf0 | (ucs4 >> 18);
                 utf8[1] = 0x80 | ((ucs4 >> 12) & 0x3f);
                utf8[2] = 0x80 | ((ucs4 >> 6) & 0x3f);
                 i = 4;
               }

               // Copy it.
               for (c = 0; c < i; ++c)
               {
                 output += utf8[c];
               }
             }
           }

           // Skip over trailing '-' in modified UTF-7 encoding.

           if (src[(int)srcPtr] == '-')
            ++srcPtr;
         }
      }


       return QString::fromUtf8(output.data());

     }

