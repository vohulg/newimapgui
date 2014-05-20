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

    QSqlQuery query();
    query.exec("SELECT id, account, password, startMonitor, endMonitor, status FROM accounts;");

    while (query.next())
    {
        QString id = query.value(0).toString();
        QString username = query.value(1).toString();
        QString password = query.value(2).toString();

        if (!imap.connectToHost(host, port, useSsl))
            IMAP_MAIN_ABORT("connectToHost()", imap.errorString());

        if (!imap.login(username, password, loginType ))
            IMAP_MAIN_ABORT("login()", imap.errorString());

        //-------получаем список ящиков для аккаунта---------//
        QStringlist listMailBox = imap.list();

        if (!checkNewFolder(id, listMailBox))
            qDebug() << "folder not checked";

        // QStringlist boxlist = getFolderList();

        // foreach (QString box, boxlist) {
    //
    //}



}
 }

    bool TMonitoring::checkNewFolder(const QString& accountId, QStringList& currentListMailBox)
    {
        QSqlQuery query();
        QString cmd = QString("SELECT folderName FROM folderMap WHERE id = %1;").arg(accountId);
        query.exec(cmd);

        while (query.next())
        {
            QString folderName = query().value(0).toString();

            if (currentListMailBox.contains(folderName))
                    continue;

             QSqlQuery queryAddFolder();
             cmd = QString("INSERT INTO folderMap(accointId, folderName) VALUES (:accountId, :folderName ) ;";
             queryAddFolder().prepare(cmd);
             queryAddFolder().bindValue(":accountId", accountId);
             queryAddFolder().bindValue(":folderName", folderName);
             queryAddFolder().exec();


        }

    return true;

    }

