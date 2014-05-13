#include "imapgui.h"
#include "ui_imapgui.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

   // testing();

    startImap();
}

MainWindow::~MainWindow()
{
    delete ui;
}

 bool MainWindow::startImap()
 {
     Imap imap;
     if (!imap.connectToHost(IMAP_HOST, IMAP_PORT, IMAP_USE_SSL))
         IMAP_MAIN_ABORT("connectToHost()", imap.errorString());

     if (!imap.login(IMAP_USERNAME, IMAP_PASSWORD, IMAP_LOGIN_TYPE))
         IMAP_MAIN_ABORT("login()", imap.errorString());

    // qDebug() << imap.list();
     ;

     ImapMailbox *mailbox = imap.select("MyFoldr");
     if (mailbox == NULL)
         IMAP_MAIN_ABORT("select()", imap.errorString());

     qDebug() << "INBOX";
     qDebug() << " - Exists:" << mailbox->exists();
     qDebug() << " - Recent:" << mailbox->recent();
     qDebug() << " - Unseen:" << mailbox->unseen();
     qDebug() << " - Is RW:" << mailbox->isReadWrite();
     qDebug() << " - Flags:" << mailbox->flags();

     //QList<int> messageList = imap.searchText("\"Test mail attachment binary\"");
    // QList<int> messageList = imap.searchRecentUnseen();

     QList<int> messageList = imap.searchALL();
     qDebug() << "messageList:" << messageList;

     /*if (imap.fetchHeaders(1) == NULL)
         IMAP_MAIN_ABORT("fetchHeaders(1)", imap.errorString());
         */

     if (imap.fetch(mailbox, messageList) == NULL)
         IMAP_MAIN_ABORT("fetch()", imap.errorString());


    foreach (int msgId, messageList) {
         ImapMessage *message = mailbox->findById(msgId);
         if (message == NULL) {
             qDebug() << "Message" << msgId << "Not Found.";
             continue;
         }

         if (!imap.fetchBodyStructure(message))
             IMAP_MAIN_ABORT("fetchBodyStructure()", imap.errorString());

         qDebug() << "ID" << message->id()
                  << "UID" << message->uid()
                  << "REF" << message->reference();
         qDebug() << "FROM" << message->fromAddress().toString();
         foreach (ImapAddress address, message->toAddresses())
             qDebug() << " - TO" << address.toString();
         foreach (ImapAddress address, message->ccAddresses())
             qDebug() << " - CC" << address.toString();
         foreach (ImapAddress address, message->bccAddresses())
             qDebug() << " - BCC" << address.toString();
         qDebug() << "SUBJECT" << message->subject();
         qDebug() << "RECIEVED" << message->received();
         qDebug() << "SENT" << message->sent();
         qDebug() << "TIMEZONE" << message->timeZone();


        //ui->textBrowser->setText();




         for (int i = 0; i < message->bodyPartCount(); ++i) {
             ImapMessageBodyPart *bodyPart = message->bodyPartAt(i);

             qDebug() << bodyPart->isAttachment() << bodyPart->bodyPart()
                      << bodyPart->fileName() << bodyPart->encoding() << bodyPart->contentType();

             if (!imap.fetchBodyPart(message, i))
                 IMAP_MAIN_ABORT("fetchBodyPart()", imap.errorString());

             //imap.setSeen(message->id(), true);
             qDebug() << bodyPart->data();
             qDebug() << "=======================================================";

             ui->lineEditSubject->setText(bodyPart->data());
         }
     }


     // Detroy Mailbox.
     delete mailbox;

     if (!imap.logout())
         IMAP_MAIN_ABORT("logout()", imap.errorString());

     if (!imap.disconnectFromHost())
         IMAP_MAIN_ABORT("disconnectFromHost()", imap.errorString());

     return true;
 }

void MainWindow::on_pushButton_clicked()
{
    startImap();
}

void MainWindow::testing()
{

    QDateTime startDate(QDate(2012, 7, 6), QTime(8, 30, 0));

    QString string = "2008:09:23 14:18:03";
    QString format = "dddd, d MMMM yyyy hh:mm:ss";

       QString string1 = "M1d1";
      QString format1 = "'M'M'd'd";

      QDateTime dateTime2 = QDateTime::fromString("M1d1y9800:01:02",
                                                  "'M'M'd'd'y'yyhh:mm:ss");

      QDateTime dateTime1 =  QDateTime::fromString(string, format);

      QDateTime dateTime3 = QDateTime::fromString("01 May 2011 10:00", "dd'MMM'yyyy'hh:mm");

      qDebug() << dateTime3.toString("dd");



      QDateTime date;
      std::string val_string = "2008:09:23 14:18:03";
      QString d = QString::fromStdString(val_string.substr(0, 19));
      date = QDateTime::fromString(d,"yyyy:MM:dd HH:mm:ss");
      qDebug()<<d;
      qDebug()<<d.length()<<date.toString();

      //-------------------------------//





}
