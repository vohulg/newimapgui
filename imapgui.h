#ifndef IMAPGUI_H
#define IMAPGUI_H

#include <QMainWindow>
#include <QStringList>
#include <QtGlobal>
#include <QDebug>
#include <QDateTime>
#include <QtSql>
#include <QTableWidget>
#include <QMessageBox>
#include <QTreeWidgetItem>

#include "imapmailbox.h"
#include "imapmessage.h"
#include "imapaddress.h"
#include "imap.h"
#include "addacount.h"
#include "tmonitoring.h"



#define IMAP_MAIN_ABORT(func, message)     \
    { qDebug() << func << message; return(1); }


const char * const base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

   enum
      {
        UNDEFINED       = 64,
        MAXLINE         = 76,
        UTF16MASK       = 0x03FFUL,
       UTF16SHIFT      = 10,
       UTF16BASE       = 0x10000UL,
        UTF16HIGHSTART  = 0xD800UL,
       UTF16HIGHEND    = 0xDBFFUL,
       UTF16LOSTART    = 0xDC00UL,
       UTF16LOEND      = 0xDFFFUL
      };

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
    void on_butChange_clicked();
    void on_buttonAdd_clicked();
    void testing();
    void createTableDataBase();
    bool connectAndCreateDataBase();

    void startMonitoring();
    void showFolders();

    void on_pushButton_clicked();
    QString imapUTF7ToUnicode(const QString & input);

    void showMessage(QTreeWidgetItem*,int);

    //void finishMonitoring();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlQuery query;
    QTableWidget *tableWidget;
    AddAcount *dialog;
    QString dataBaseName;
    bool res;

   //QList<QTreeWidgetItem> listFolder;


};

#endif // IMAPGUI_H
