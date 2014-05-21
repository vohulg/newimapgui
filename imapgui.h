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
#include "tmonitoring.h"



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

signals:
    void sigShowItemForChange();

private slots:
    bool RefreshAccountsList();

    bool connectDatabase(const QString& database);
    bool saveToDataBase(ImapMailbox *mailbox, const QList<int>& messages);
    bool startImap(const QString& host, quint16 port, bool useSsl, const QString& username, const QString& password, Imap::LoginType loginType );
    void on_butChange_clicked();
    void on_buttonAdd_clicked();
    void testing();
    void createTableDataBase();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlQuery query;
    Imap imap;
    QTableWidget *tableWidget;
    AddAcount *dialog;
    TMonitoring *monitorLoop;
    QString dataBaseName;

};

#endif // IMAPGUI_H
