#include"myclient.h"

MyClient::MyClient(

                   QWidget*       pwgt
                  ) : QWidget(pwgt)
                    , m_nNextBlockSize(0)
{
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this,         SLOT(slotError(QAbstractSocket::SocketError))
           );
    chatTextBrowser  = new QTextBrowser;
    chatTextBrowser->setOpenLinks(false);
    connect(chatTextBrowser, QTextBrowser::anchorClicked, this, onAnchorClicked);

    inputLineEdit = new QLineEdit;
    nameLabel=new QLabel("<H2>Список контактов:<\/H2>");
    inputLineEdit->setReadOnly(true);
    connect(inputLineEdit, SIGNAL(returnPressed()),this,SLOT(slotSendToServer()));

    sendMessageButton = new QPushButton("Отправить");
    connect(sendMessageButton, SIGNAL(clicked()), SLOT(slotSendToServer()));

    fileTransferButton = new QPushButton();
    fileTransferButton->setIcon(this->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(fileTransferButton, SIGNAL(clicked()), SLOT(addFile()));

    fileTransferButton->setDisabled(1);
    sendMessageButton->setDisabled(1);
    listWidget = new QListWidget(this);

QMenuBar *menuBar=new QMenuBar(this);

   menuBar->addAction("Смена пользователя", this, SLOT(slotChangeUser()) );

   QVBoxLayout* vLayout1 = new QVBoxLayout;
   vLayout1->addWidget(nameLabel);
   vLayout1->addWidget(listWidget);

   QHBoxLayout* buttonHLayout = new QHBoxLayout;
   buttonHLayout->addWidget(sendMessageButton,10);
 buttonHLayout->addWidget(fileTransferButton,1);

   QVBoxLayout* vLayout2 = new QVBoxLayout;
   vLayout2->addWidget(new QLabel("<H2>Сообщения:<\/H2>"));
   vLayout2->addWidget(chatTextBrowser);
   vLayout2->addWidget(inputLineEdit);
   vLayout2->addLayout(buttonHLayout);

   QFrame *line = new QFrame(this);
   line->setFrameShape(QFrame::VLine); // Horizontal line
   line->setFrameShadow(QFrame::Sunken);
   line->setLineWidth(1);

    QHBoxLayout* hLayout = new QHBoxLayout;
   hLayout->setMenuBar(menuBar);
   hLayout->addLayout(vLayout1,1);
   hLayout->addWidget(line);
   hLayout->addLayout(vLayout2,4);
   setLayout(hLayout);

   connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(onListItemClick(QListWidgetItem*)));
}
void MyClient::addFile()
{ 
  QString fileName = QFileDialog::getOpenFileName(this,
        tr("Прикрепить файл"));
  if(fileName=="") return;
  QFile *file=new QFile(fileName);
  if (!file->open(QIODevice::ReadOnly)) {
      qDebug() << "Невозможно открыть файл";
      return;
  }
  QByteArray q = file->readAll();
  QByteArray  arrBlock;
  QDataStream out(&arrBlock, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_2);
  out << quint64(0) <<3<< QTime::currentTime()<<name<<reciever<<file->fileName()<<q.size();
arrBlock.append(q);
 file->close();
  chatTextBrowser->append("<span>Отправлен файл "+file->fileName().section("/",-1)+"</span>");
setAllignment(1);
  out.device()->seek(0);
  out << quint64(arrBlock.size() - sizeof(quint64));

  socket->write(arrBlock);

  qDebug()<<arrBlock.size();
}
void MyClient::slotChangeUser()
{
    reciever="";
    chatTextBrowser->clear();
    fileTransferButton->setDisabled(1);
    sendMessageButton->setDisabled(1);
    inputLineEdit->setReadOnly(true);
    socket->disconnectFromHost();
    this->close();
    emit reopenConnectionWidget();
}
void MyClient::onListItemClick(QListWidgetItem *item)
{
chatTextBrowser->clear();
QString str=item->text();
int a=str.lastIndexOf("(");
QString result=str.left(a);
 for(int i=0;i<users.length();i++)
 {
    if(users[i].first==result) users[i].second=0;
 }
item->setText(result);
reciever=item->text();
getHistory(reciever);
inputLineEdit->setReadOnly(false);
inputLineEdit->setReadOnly(false);
fileTransferButton->setDisabled(0);
sendMessageButton->setDisabled(0);
}
void MyClient::slotReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_2);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (socket->bytesAvailable() < sizeof(quint64)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (socket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        int mode;
        in>>mode;
        switch(mode)
        {
        default:
        {break;}
        case 0://Получение сообщения
        {
            QTime   time;
            QString str;
            QString sender;
            in >> time >>sender>> str;
            if(sender==reciever)
            {
                        chatTextBrowser->append("<span>"+time.toString() + " " +sender+": "+ str+"</span>");

                        setAllignment(0);
            }
else
{
    for(int i=0;i<users.length();i++)
    {
        if(sender==users[i].first)
        {
           users[i].second++;
           QString number= QString::number(users[i].second);
            if(users[i].second==1)
            {

            listWidget->item(i)->setText(listWidget->item(i)->text()+"("+number+")");
            }
            else if(users[i].second>1)
            {
                QString str=listWidget->item(i)->text();
                int a=str.lastIndexOf("(");
                QString result=str.left(a);
                listWidget->item(i)->setText(result+"("+number+")");
            }
        }
    }
}
            break;
        }
        case 1://Получение истории
        {
        chatTextBrowser->clear();
        QList<QList<QString>> messages;
        in>>messages;
        foreach (QList<QString> l, messages) {
            QString id=l[0];
            QString time=l[1];
            QString sender=l[2];
            QString reciever=l[3];
            QString text=l[4];
            QString fileName=l[5];
            if(reciever==name)
            {
                 if(fileName=="")
                 {
                  chatTextBrowser->append("<span>"+time + " " +sender+": "+ text+"</span>");
                  setAllignment(0);
                 }
                 else
                 {
                     chatTextBrowser->append("<span>"+time + " " +sender+" отправил файл\""+"<a href = \""+id+"\" >"+ fileName.section("/",-1)+"</a>"+"\".</span>");
                     setAllignment(0);
                 }
             }
             else
             {
                if(fileName=="")
                {
                 chatTextBrowser->append("<span>"+text+"</span>");
                 setAllignment(1);
                }
                else
                {
                  chatTextBrowser->append("<span>Отправлен файл "+fileName.section("/",-1)+"</span>");
                  setAllignment(1);
                }
             }
        }

        break;
        }
        case 2://Получение контактов
        {
            QList<QString> list;
            users.clear();
            in>>list;

            listWidget->clear();

            foreach (QString str, list) {

                  new QListWidgetItem(str, listWidget);
                users.append(qMakePair(str,0));
            }
            bool ka=0;
            foreach (auto user, users) {
                if(user.first==reciever) ka=1;
            }
          if(ka==0)
          {
              reciever="";
              chatTextBrowser->clear();
              fileTransferButton->setDisabled(1);
              sendMessageButton->setDisabled(1);
              inputLineEdit->setReadOnly(true);
          }
            break;
        }
        case 3://Получение ссылки на файл
        {
             QTime   time;
            QString sender;
            QString fileName;
            int id;
            in>>time>>sender>>fileName>>id;
             if(sender==reciever)
             {
                 chatTextBrowser->append("<span>"+time.toString() + " " +sender+" отправил файл\""+"<a href = \""+QString::number(id)+"\" >"+ fileName.section("/",-1)+"</a>"+"\".</span>");
                 setAllignment(0);
             }
             else
             {
                 for(int i=0;i<users.length();i++)
                 {
                     if(sender==users[i].first)
                     {
                        users[i].second++;
                        QString number= QString::number(users[i].second);
                         if(users[i].second==1)
                         {

                         listWidget->item(i)->setText(listWidget->item(i)->text()+"("+number+")");
                         }
                         else if(users[i].second>1)
                         {
                             QString str=listWidget->item(i)->text();
                             int a=str.lastIndexOf("(");
                             QString result=str.left(a);
                             listWidget->item(i)->setText(result+"("+number+")");
                         }
                     }
                 }
             }

            break;
        }
        case 4:// Получение файлов
        {

            getHistory(reciever);
            QString fileName;
            int size;
            in>>fileName>>size;
            QByteArray q = socket->read(size);
            qDebug()<<q.size();

           QString filePath = QFileDialog::getSaveFileName(this, "Сохранить файл", fileName);
            if(filePath=="") break;
               QFile target(filePath);

               if (!target.open(QIODevice::WriteOnly)) {
                   qDebug() << "Невозможно открыть файл";
                   break;
               }
               target.write(q);

               target.close();


            break;
        }
        case -1:
        {
            bool alowance;
            in>>alowance;
            if(alowance==true)
            {
                emit sendAllowanceResult(0);
                this->show();
                 m_nNextBlockSize = 0;
                 return;
            }

                emit sendAllowanceResult(1);
            socket->disconnectFromHost();
            break;
        }
        }
        m_nNextBlockSize = 0;
    }
}
void MyClient::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Ошибка: " + (err == QAbstractSocket::HostNotFoundError ?
                     "Хост не был найден." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "Сервер закрыл соединение." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "Сервер отказал в соединении." :
                     QString(socket->errorString())
                    );
    chatTextBrowser->append("<span>"+strError+"</span>");
}
void MyClient::slotSendToServer()// обычное сообщение
{if(inputLineEdit->text()=="") return;
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint64(0) <<0<< QTime::currentTime()<<reciever<<name<< inputLineEdit->text();
    chatTextBrowser->append("<span>"+inputLineEdit->text()+"</span>");
    setAllignment(1);
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));

    socket->write(arrBlock);
    inputLineEdit->setText("");
}
void MyClient::sendNameToServer()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint64(0) <<1<<name;

    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));

    socket->write(arrBlock);

}
void MyClient::getListOfUsers()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);

    out << quint64(0) <<1<<name;// 1- запрос списка контактов

    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));

    socket->write(arrBlock);

}
void MyClient::getHistory(QString user)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);

    out << quint64(0) <<2<<user<<name;// 2- запрос истории


    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));

    socket->write(arrBlock);

}
void MyClient::slotConnected()
{chatTextBrowser->clear();
    chatTextBrowser->append("<span>Подключение к серверу прошло успешно</span>");
}
void MyClient::slotRecieveData(QList<QString> list)
{

    name=list[2];
    socket->connectToHost(list[0], list[1].toInt());

   sendNameToServer();
   setWindowTitle("MESSenger - "+name);

}
void MyClient::onAnchorClicked(const QUrl &url)
{
QString str=url.toEncoded();
str=str.section("/",-1);
int id=str.toInt();
QByteArray  arrBlock;
QDataStream out(&arrBlock, QIODevice::WriteOnly);
out.setVersion(QDataStream::Qt_4_2);
out << quint64(0)<<4<<id;

out.device()->seek(0);
out << quint64(arrBlock.size() - sizeof(quint64));

socket->write(arrBlock);

}
void MyClient::setAllignment(bool allignment)
{ QTextCursor cursor = chatTextBrowser->textCursor();
    QTextBlockFormat textBlockFormat = cursor.blockFormat();
    if(allignment==1)
    {
         textBlockFormat.setAlignment(Qt::AlignRight);
    }
   else
    {
        textBlockFormat.setAlignment(Qt::AlignLeft);
    }

    cursor.mergeBlockFormat(textBlockFormat);
    chatTextBrowser->setTextCursor(cursor);
}
