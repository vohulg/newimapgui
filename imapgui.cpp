#include "imapgui.h"
#include "ui_imapgui.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    res = false;

    if (!connectAndCreateDataBase())
        qDebug() << "DataBase not connected";

   RefreshAccountsList();

   showFolders();
   //startMonitoring();

   QObject::connect(ui->treeWidgetFolders, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(showMessage(QTreeWidgetItem*,int)));


}

void MainWindow::showMessage(QTreeWidgetItem* selectedItem,int column)
{

    QMessageBox msgBox;
    msgBox.setWindowTitle("title");
    //msgBox.setText(selectedItem->text(column));
     msgBox.setText(QString::number(selectedItem->type()));
    msgBox.exec();

    //info = QMessageBox::about(this, "Test", selectedItem->text(column));

    //QMessageBox msgBox(QMessageBox::Warning, tr("QMessageBox::warning()"), "Warrning", 0, this);
   // msgBox.show();

}

bool MainWindow::connectAndCreateDataBase()
{
    dataBaseName = "dbImap.sqlite";

    //===== check if databas not exists create database and all tables======//
    QFile file(dataBaseName);
    if(!file.exists())
        createTableDataBase();

    //createTableDataBase();

   if (!connectDatabase(dataBaseName))
       FUNC_ABORT("database not connected");

   dialog = new AddAcount();
   QObject::connect(dialog , SIGNAL(sigRefreshTable()),this, SLOT(RefreshAccountsList()));
   QObject::connect(this , SIGNAL(sigShowItemForChange()),dialog, SLOT(showItemForChange()));
   return true;
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

    ui->treeWidgetFolders->setHeaderLabel("Объекты");
    QSqlQuery query;
    query.exec("SELECT  account, id FROM accounts ORDER BY id;");
    while(query.next())
    {
        QString account = query.value(0).toString();
        QString id = query.value(1).toString();
        QTreeWidgetItem *topLevelAccountItem=new QTreeWidgetItem(ui->treeWidgetFolders);
        topLevelAccountItem->setText(0,account);

        QSqlQuery queryChild;
        QString cmd = "SELECT  folderName FROM folderMap WHERE accountId =" + id;
        queryChild.exec(cmd);
        while(queryChild.next())
        {
           QString folderName = queryChild.value(0).toString();
           QTreeWidgetItem *item=new QTreeWidgetItem(topLevelAccountItem);

           item->setText(0,imapUTF7ToUnicode(folderName));
        }

    }

}

QString MainWindow::imapUTF7ToUnicode(const QString & input)
      {

       unsigned char c, i, bitcount;
       unsigned long ucs4, utf16, bitbuf;
        unsigned char base64[256],utf8[6];
       unsigned long srcPtr = 0;

        QByteArray output;
        QByteArray src = input.toLatin1();

        // Initialize modified base64 decoding table.

        memset(base64, UNDEFINED, sizeof(base64));

      // for (i = 0; i < sizeof(base64chars); ++i)
      for (i = 0; i < 64; ++i)
        {
          base64[(int)base64chars[i]] = i;
        }


        // Loop until end of string.
       while (srcPtr < src.length())
        {
          c = src[(int)srcPtr++];

          // Deal with literal characters and "&-".

          if (c != '&' || src[(int)srcPtr] == '-')
          {
            // Encode literally.

            output += c;

            // Skip over the '-' if this is an "&-" sequence.

           if (c == '&')
             srcPtr++;
         }
         else
         {
           // Convert modified UTF-7 -> UTF-16 -> UCS-4 -> UTF-8 -> HEX.
           bitbuf = 0;
           bitcount = 0;
           ucs4 = 0;

           while ((c = base64[(unsigned char) src[(int)srcPtr]]) != UNDEFINED)
          {

             ++srcPtr;
            bitbuf = (bitbuf << 6) | c;
            bitcount += 6;

             // Enough bits for an UTF-16 character ?

             if (bitcount >= 16)
           {
             bitcount -= 16;
              utf16 = (bitcount ? bitbuf >> bitcount : bitbuf) & 0xffff;

              // Convert UTF16 to UCS4.

               if (utf16 >= UTF16HIGHSTART && utf16 <= UTF16HIGHEND)
               {
                 ucs4 = (utf16 - UTF16HIGHSTART) << UTF16SHIFT;
                continue;
             }
              else if (utf16 >= UTF16LOSTART && utf16 <= UTF16LOEND)
              {
                 ucs4 += utf16 - UTF16LOSTART + UTF16BASE;
               }
             else
               {
                ucs4 = utf16;
              }

              // Convert UTF-16 range of UCS4 to UTF-8.

               if (ucs4 <= 0x7fUL)
               {
                utf8[0] = ucs4;
               i = 1;
               }
              else if (ucs4 <= 0x7ffUL)
               {
                 utf8[0] = 0xc0 | (ucs4 >> 6);
                utf8[1] = 0x80 | (ucs4 & 0x3f);
                 i = 2;
               }
              else if (ucs4 <= 0xffffUL)
               {
                utf8[0] = 0xe0 | (ucs4 >> 12);
                utf8[1] = 0x80 | ((ucs4 >> 6) & 0x3f);
                 utf8[2] = 0x80 | (ucs4 & 0x3f);
                 i = 3;
              }
               else
              {
                 utf8[0] = 0xf0 | (ucs4 >> 18);
                 utf8[1] = 0x80 | ((ucs4 >> 12) & 0x3f);
                utf8[2] = 0x80 | ((ucs4 >> 6) & 0x3f);
                 i = 4;
               }

               // Copy it.
               for (c = 0; c < i; ++c)
               {
                 output += utf8[c];
               }
             }
           }

           // Skip over trailing '-' in modified UTF-7 encoding.

           if (src[(int)srcPtr] == '-')
            ++srcPtr;
         }
      }


       return QString::fromUtf8(output.data());

     }


