#ifndef ADDACOUNT_H
#define ADDACOUNT_H

#include <QDialog>
#include <QtSql>
#include <QTableWidget>
#include <QString>


namespace Ui {
class AddAcount;
}

class AddAcount : public QDialog
{
    Q_OBJECT

public:
    explicit AddAcount(QWidget *parent = 0 );
    ~AddAcount();
     bool setDatabase (QSqlDatabase &db);
     void setItemForChange(QString itemForChangeIn);

signals:
    bool sigRefreshTable();


private slots:
    bool on_buttonBox_accepted();
    void showItemForChange();


private:
    Ui::AddAcount *uiAdd;
    QSqlDatabase dataBase;
    QString ItemForChange;

};

#endif // ADDACOUNT_H
