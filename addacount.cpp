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
        QSqlQuery query;
        query.prepare("INSERT INTO accounts(account, password, startMonitor, endMonitor, status) VALUES (:account, :password, :startMonitor, :endMonitor, :status );");
        query.bindValue(":account", uiAdd->lineAccountName->text());
        query.bindValue(":password", uiAdd->lineAccountPassword->text());
        query.bindValue(":startMonitor", uiAdd->dateTimeStartMonitor->dateTime());
        query.bindValue(":endMonitor", uiAdd->dateTimeEndMonitor->dateTime());
        query.bindValue(":status", uiAdd->checkBoxStatus->isChecked());
        query.exec();

        emit sigRefreshTable();
        return true;
}
