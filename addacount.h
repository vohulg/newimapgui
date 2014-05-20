#ifndef ADDACOUNT_H
#define ADDACOUNT_H

#include <QDialog>
#include <QtSql>


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

signals:
    bool sigRefreshTable();


private slots:
    bool on_buttonBox_accepted();

private:
    Ui::AddAcount *uiAdd;
    QSqlDatabase dataBase;

};

#endif // ADDACOUNT_H
