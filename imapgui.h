#ifndef IMAPGUI_H
#define IMAPGUI_H

#include <QMainWindow>
#include <QStringList>
#include <QtGlobal>
#include <QDebug>
#include <QDateTime>

#include "imapmailbox.h"
#include "imapmessage.h"
#include "imapaddress.h"
#include "imap.h"

#define IMAP_HOST           "imap.mail.ru"
#define IMAP_PORT           (993)
#define IMAP_USE_SSL        (true)

#define IMAP_USERNAME       "testov-79@mail.ru"
#define IMAP_PASSWORD       "testtest"
#define IMAP_LOGIN_TYPE     (Imap::LoginPlain)

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
    bool startImap();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    void testing();
};

#endif // IMAPGUI_H
