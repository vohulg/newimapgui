#ifndef IMAPGUI_H
#define IMAPGUI_H

#include <QMainWindow>
#include <QStringList>
#include <QtGlobal>
#include <QDebug>
#include <QDateTime>
#include <QtSql>
#include <QTableWidget>

#include "imapmailbox.h"
#include "imapmessage.h"
#include "imapaddress.h"
#include "imap.h"
#include "addacount.h"

#define IMAP_MAIN_ABORT(func, message)     \
    { qDebug() << func << message; return(1); }

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool startImap(const QString& host, quint16 port, bool useSsl, const QString& username, const QString& password, Imap::LoginType loginType );


signals:
   // sigRefreshTable();

private slots:


    void on_buttonAdd_clicked();

private:
    Ui::MainWindow *ui;
    void testing();
    QSqlDatabase db;
    QSqlQuery query;
    bool saveToDataBase(ImapMailbox *mailbox, const QList<int>& messages);
    Imap imap;

    bool connectDatabase(const QString& database);
    QTableWidget *tableWidget;
    AddAcount *dialog;

private slots:
    bool RefreshAccountsList();
};

#endif // IMAPGUI_H
