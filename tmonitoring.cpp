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
    query.exec("SELECT account, password, startMonitor, endMonitor, status FROM accounts;");

    while (query.next())
    {
        query.value(0).toString();
        query.value(1).toString();


    }



}
