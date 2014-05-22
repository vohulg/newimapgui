#include "tmonitoring.h"

TMonitoring::TMonitoring(QObject *parent) :
    QThread(parent)
{
}

bool TMonitoring::setDatabase(QSqlDatabase &db)
 {
     dataBase = db;
     return true;
 }

void TMonitoring::run()
{

   //=========initializ server value=========//
    QString host = "imap.mail.ru";
    quint16 port = 993;
    bool useSsl = true;
    Imap::LoginType loginType = Imap::LoginPlain;
    //============================================//

    QSqlQuery query;
    query.exec("SELECT id, account, password, startMonitor, endMonitor, status FROM accounts;");

    while (query.next())
    {
        QString id = query.value(0).toString();
        QString username = query.value(1).toString();
        QString password = query.value(2).toString();

        if (!imap.connectToHost(host, port, useSsl))
            qDebug() << "not connected to server";
            //IMAP_MAIN_ABORT("connectToHost()", imap.errorString());

        if (!imap.login(username, password, loginType ))
            qDebug() << "not connected to mailbox";
            //IMAP_MAIN_ABORT("login()", imap.errorString());

        //-------получаем список ящиков для аккаунта---------//
        listMailBox = imap.list();

        //listMailBox << "INPUT" << "MyFoldr";

        if (!checkNewFolder(id, listMailBox))
            qDebug() << "folder not checked";


        //=====get message from
         foreach (QString box, listMailBox)
         {
             ImapMailbox *mailbox = imap.select(box);
             if (mailbox == NULL)
                  qDebug() << box <<" not selected";

             QList<int> messageList = imap.searchALL();
             qDebug() << box << " messageList:" << messageList;

         }





}
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

