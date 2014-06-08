#include "imapgui.h"
#include "ui_imapgui.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

     testing();

    dataBaseName = "dbImap.sqlite";

    //===== check if databas not exists create database and all tables======//
    QFile file(dataBaseName);
    if(!file.exists())
        createTableDataBase();

    createTableDataBase();

   if (!connectDatabase(dataBaseName))
      qDebug() << "database not connected";



   dialog = new AddAcount();
   QObject::connect(dialog , SIGNAL(sigRefreshTable()),this, SLOT(RefreshAccountsList()));
   QObject::connect(this , SIGNAL(sigShowItemForChange()),dialog, SLOT(showItemForChange()));

   RefreshAccountsList();

   showFolders();

   //startMonitoring();





    //startImap(host, port, useSsl, username, password, loginType);
}

MainWindow::~MainWindow()
{
    db.close();
    delete ui;
    delete dialog;
    query.clear();
}

void MainWindow::createTableDataBase()
{
    bool res = false;

    if (!connectDatabase(dataBaseName))
       qDebug() << "database not connected";

    QSqlQuery query;
   // query.exec("CREATE TABLE folderMap (id INTEGER PRIMARY KEY AUTOINCREMENT, accountId INTEGER, folderName TEXT)");
    res = query.exec("CREATE TABLE accountstmp (id INTEGER PRIMARY KEY AUTOINCREMENT, account TEXT, password TEXT, startMonitor TIMESTAMP, endMonitor TIMESTAMP, status BOOL )");


   res =  query.exec("CREATE TABLE headers (id INTEGER PRIMARY KEY AUTOINCREMENT, accountId INTEGER, bcc TEXT, cc TEXT, flags TEXT, htmlpart TEXT, folderId INTEGER, from TEXT, subject TEXT, copyTo TEXT, recieved TIMESTAMP, created TIMESTAMP, UID TEXT, REF TEXT, TIMEZONE TEXT )");

   int tmp = 0;

    }

bool MainWindow::connectDatabase(const QString& database)
{

     db =  QSqlDatabase::addDatabase("QSQLITE");
     db.setDatabaseName(database);
     bool res = true;

     res = db.open();
     if (!res)
         return false;
     return true;
}

 bool MainWindow::RefreshAccountsList() //refresh
 {
    bool res = false;

    ui->tableWidgetListAccounts->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetListAccounts->setSelectionBehavior(QAbstractItemView::SelectRows);
     ui->tableWidgetListAccounts->setColumnCount(5);
     ui->tableWidgetListAccounts->setColumnWidth(0,50);
     ui->tableWidgetListAccounts->setColumnWidth(1,200);
     ui->tableWidgetListAccounts->setColumnWidth(2,150);
     ui->tableWidgetListAccounts->setColumnWidth(3,150);
     ui->tableWidgetListAccounts->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("ID")));
     ui->tableWidgetListAccounts->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Account")));
     ui->tableWidgetListAccounts->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Start Monitor")));
     ui->tableWidgetListAccounts->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("End Monitor")));
     ui->tableWidgetListAccounts->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Activ")));


     int n = ui->tableWidgetListAccounts->rowCount();
          for( int i = 0; i < n; i++ ) ui->tableWidgetListAccounts->removeRow( 0 );

          QSqlQuery query(db);
          res = query.exec("SELECT id, account, startMonitor, endMonitor, status FROM accounts;");

          while (query.next())
          {
               ui->tableWidgetListAccounts->insertRow(0);
               ui->tableWidgetListAccounts->setItem(0, 0, new QTableWidgetItem(query.value(0).toString()));
               ui->tableWidgetListAccounts->setItem(0, 1, new QTableWidgetItem(query.value(1).toString()));
               ui->tableWidgetListAccounts->setItem(0, 2, new QTableWidgetItem(query.value(2).toDateTime().toString()));
               ui->tableWidgetListAccounts->setItem(0, 3, new QTableWidgetItem(query.value(3).toDateTime().toString()));
               ui->tableWidgetListAccounts->setItem(0, 4, new QTableWidgetItem(query.value(4).toString()));
               ui->tableWidgetListAccounts->setRowHeight(0, 20);
          }

     return true;
 }


// ---------------- start download message from mailbox by imap protocol ---------------------------- //

 bool MainWindow::startImap(const QString& host, quint16 port, bool useSsl, const QString& username, const QString& password, Imap::LoginType loginType)
 {

     if (!imap.connectToHost(host, port, useSsl))
         IMAP_MAIN_ABORT("connectToHost()", imap.errorString());

     if (!imap.login(username, password, loginType ))
         IMAP_MAIN_ABORT("login()", imap.errorString());

     //qDebug() << imap.list();


     ImapMailbox *mailbox = imap.select("INBOX");
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


     if (!saveToDataBase(mailbox, messageList))
          qDebug() << "Don't saved new messagу in mailbox";


     // Detroy Mailbox.
     delete mailbox;

     if (!imap.logout())
         IMAP_MAIN_ABORT("logout()", imap.errorString());

     if (!imap.disconnectFromHost())
         IMAP_MAIN_ABORT("disconnectFromHost()", imap.errorString());

     return true;
 }

 //----------------- save to database-------------------//

 bool MainWindow::saveToDataBase(ImapMailbox *mailbox, const QList<int>& messageList)
 {


     foreach (int msgId, messageList) {
          ImapMessage *message = mailbox->findByUid(msgId);
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


                 for (int i = 0; i < message->bodyPartCount(); ++i) {
              ImapMessageBodyPart *bodyPart = message->bodyPartAt(i);

              qDebug() << bodyPart->isAttachment() << bodyPart->bodyPart()
                       << bodyPart->fileName() << bodyPart->encoding() << bodyPart->contentType();

              if (!imap.fetchBodyPart(message, i))
                  IMAP_MAIN_ABORT("fetchBodyPart()", imap.errorString());

              //imap.setSeen(message->id(), true);
              qDebug() << bodyPart->data();
              qDebug() << "=======================================================";


          }
      }

     return true;

 }


// ---------------- function for testing ---------------------------- //

void MainWindow::testing()
{

    QString string2 = "22:51:41";
    QString format2 = "hh:mm:ss";
    QDateTime valid2 = QDateTime::fromString(string2, format2);
    QString res = valid2.toString();

    qDebug() << "dattime:" << res ;
    return;


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

void MainWindow::on_buttonAdd_clicked()
{

    dialog->show();
    dialog->setDatabase(db);

}

void MainWindow::on_butChange_clicked()
{
    QList<QTableWidgetItem *> list;
    list = ui->tableWidgetListAccounts->selectedItems();
    QString item = list[0]->text();
    dialog->setItemForChange(item);
    emit sigShowItemForChange();
    dialog->show();

}

void MainWindow::startMonitoring()
{
    //======== start loop for monitoring mail, agent and contact================//

    TMonitoring *monitorLoop = new TMonitoring();
    //connect(monitorLoop, SIGNAL(finished()), this, SLOT(finishMonitoring());
    monitorLoop->setDatabase(db);
    monitorLoop->run();

     //==================================================//

}

// запуск мониторинга
void MainWindow::on_pushButton_clicked()
{
    startMonitoring();
}

void MainWindow::showFolders()
{


    // создаем новый итем (пусть сначала базовый)
    QTreeWidgetItem *topLevelAccountItem=new QTreeWidgetItem(ui->treeWidgetFolders);
    // вешаем его на наше дерево в качестве топ узла.
    ui->treeWidgetFolders->addTopLevelItem(topLevelAccountItem);
    // укажем текст итема
    topLevelAccountItem->setText(0,"info@mail.ru");
    // создаем новый итем и сразу вешаем его на наш базовый
    QTreeWidgetItem *item=new QTreeWidgetItem(topLevelAccountItem);
    // укажем текст итема
    item->setText(0,"INBOX");

    QTreeWidgetItem *topLevelAccountItem2=new QTreeWidgetItem(ui->treeWidgetFolders);
    // вешаем его на наше дерево в качестве топ узла.
    ui->treeWidgetFolders->addTopLevelItem(topLevelAccountItem2);
    // укажем текст итема
    topLevelAccountItem2->setText(0,"mail@mail.ru");


}


