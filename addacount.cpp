#include "addacount.h"
#include "ui_addacount.h"

AddAcount::AddAcount(QWidget *parent) :
    QDialog(parent),
    uiAdd(new Ui::AddAcount)
{
    uiAdd->setupUi(this);
    uiAdd->dateTimeStartMonitor->setDateTime(QDateTime::currentDateTime());
    uiAdd->dateTimeEndMonitor->setDateTime(QDateTime::currentDateTime());
}

bool AddAcount::setDatabase(QSqlDatabase &db)
 {
     dataBase = db;
     return true;
 }

AddAcount::~AddAcount()
{
    delete uiAdd;
}

bool AddAcount::on_buttonBox_accepted()
{
        bool res = false;
        QSqlQuery query;
        QString cmd;


        if (!ItemForChange.isEmpty())
        {
             cmd = QString("UPDATE accounts SET account = :account, password = :password, startMonitor = :startMonitor, endMonitor = :endMonitor, status = :status, imapServer = :imapServer WHERE id = %1;").arg(ItemForChange);
             ItemForChange.clear();
        }
        else
        {
            cmd = "INSERT INTO accounts(account, password, startMonitor, endMonitor, status, imapServer)"
                               " VALUES (:account, :password, :startMonitor, :endMonitor, :status, :imapServer)";
            ItemForChange.clear();
        }

        query.prepare(cmd);
        query.bindValue(":account", uiAdd->lineAccountName->text());
        query.bindValue(":password", uiAdd->lineAccountPassword->text());
        query.bindValue(":startMonitor", uiAdd->dateTimeStartMonitor->dateTime());
        query.bindValue(":endMonitor", uiAdd->dateTimeEndMonitor->dateTime());
        query.bindValue(":status", uiAdd->checkBoxStatus->isChecked());
        query.bindValue(":imapServer", uiAdd->lineImapServer->text());
        res = query.exec();

        emit sigRefreshTable();
        return true;
}

 void  AddAcount::setItemForChange(QString itemForChangeIn)
 {
    ItemForChange=itemForChangeIn;
 }

 void AddAcount::showItemForChange()
 {
     QString cmd = QString("SELECT account, password, startMonitor, endMonitor, status, imapServer FROM accounts WHERE id = %1;").arg(ItemForChange);

     QSqlQuery query;
     query.exec(cmd);

     QString debug;

     while (query.next())
     {

         uiAdd->lineAccountName->setText(query.value(0).toString());
         uiAdd->lineAccountPassword->setText(query.value(1).toString());
         uiAdd->dateTimeStartMonitor->setDateTime(query.value(2).toDateTime());
         uiAdd->dateTimeEndMonitor->setDateTime(query.value(3).toDateTime());

         if (query.value(4).toString() == "1")
             uiAdd->checkBoxStatus->setChecked(true);

          uiAdd->lineImapServer->setText(query.value(5).toString());





     }


 }
