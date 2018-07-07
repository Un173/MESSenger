#include"myclient.h"

MyClient::MyClient(

                   QWidget*       pwgt /*=0*/
                  ) : QWidget(pwgt)
                    , m_nNextBlockSize(0)
{
    m_pTcpSocket = new QTcpSocket(this);


    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this,         SLOT(slotError(QAbstractSocket::SocketError))
           );

    m_ptxtInfo  = new QTextBrowser;
    m_ptxtInfo->setOpenLinks(false);
    m_ptxtInfo->setOpenExternalLinks(false);
    connect(m_ptxtInfo, &QTextBrowser::anchorClicked, this, onAnchorClicked);
    m_ptxtInput = new QLineEdit;
    nameLabel=new QLabel("<H2>Список контактов:<\/H2>");
m_ptxtInput->setReadOnly(true);
    connect(m_ptxtInput, SIGNAL(returnPressed()),
            this,        SLOT(slotSendToServer())
           );
    m_ptxtInfo->setReadOnly(true);

    pcmd = new QPushButton("Отправить");
    connect(pcmd, SIGNAL(clicked()), SLOT(slotSendToServer()));
 // fileTransferButton = new QPushButton("Прикрепить файл");
    fileTransferButton = new QPushButton();
    fileTransferButton->setIcon(this->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(fileTransferButton, SIGNAL(clicked()), SLOT(addFile()));
fileTransferButton->setDisabled(1);
pcmd->setDisabled(1);
     listWidget = new QListWidget(this);

QMenuBar *menuBar=new QMenuBar(this);


   menuBar->addAction("Смена пользователя", this, SLOT(slotChangeUser()) );
//menuBar->show();
   QVBoxLayout* vLayout1 = new QVBoxLayout;

   vLayout1->addWidget(nameLabel);
   vLayout1->addWidget(listWidget);
   QHBoxLayout* buttonHLayout = new QHBoxLayout;
   buttonHLayout->addWidget(pcmd,10);
 buttonHLayout->addWidget(fileTransferButton,1);

   QVBoxLayout* vLayout2 = new QVBoxLayout;
   vLayout2->addWidget(new QLabel("<H2>Сообщения:<\/H2>"));
   vLayout2->addWidget(m_ptxtInfo);
   vLayout2->addWidget(m_ptxtInput);
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


    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)),
               this, SLOT(onListItemClick(QListWidgetItem*)));
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
  out << quint16(0) <<3<< QTime::currentTime()<<name<<reciever<<file->fileName();
  //
arrBlock.append(q);
 file->close();

  m_ptxtInfo->append("Отправлен файл "+file->fileName().section("/",-1));
  m_ptxtInfo->setAlignment(Qt::AlignRight);

  //
  out.device()->seek(0);
  out << quint16(arrBlock.size() - sizeof(quint16));

  m_pTcpSocket->write(arrBlock);

  qDebug()<<q.size();
}
void MyClient::slotChangeUser()
{
    m_pTcpSocket->disconnectFromHost();
    this->close();
    emit reopenConnectionWidget();
}
void MyClient::onListItemClick(QListWidgetItem *item)
{
m_ptxtInfo->clear();
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
m_ptxtInput->setReadOnly(false);
m_ptxtInput->setReadOnly(false);
fileTransferButton->setDisabled(0);
pcmd->setDisabled(0);
}
void MyClient::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_2);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
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
                        m_ptxtInfo->append(time.toString() + " " +sender+": "+ str);
                        m_ptxtInfo->setAlignment(Qt::AlignLeft);
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

        QList<QList<QString>> messages;
        in>>messages;
        foreach (QList<QString> l, messages) {
           //   l<<QString::number(m.id)<<m.time.toString()<<m.sender<<m.reciever<<m.text<<m.fileName;

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
                  m_ptxtInfo->append(time + " " +sender+": "+ text);
                  m_ptxtInfo->setAlignment(Qt::AlignLeft);
                 }
                 else
                 {
                     m_ptxtInfo->append(time + " " +sender+" отправил файл\""+"<a href = \""+id+"\" >"+ fileName.section("/",-1)+"</a>"+"\".");
                     m_ptxtInfo->setAlignment(Qt::AlignLeft);
                 }
             }
             else
             {
                if(fileName=="")
                {
                 m_ptxtInfo->append(text);
                 m_ptxtInfo->setAlignment(Qt::AlignRight);
                }
                else
                {
                  m_ptxtInfo->append("Отправлен файл "+fileName.section("/",-1));
                  m_ptxtInfo->setAlignment(Qt::AlignRight);
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

            break;
        }
        case 3://Получение ссылки на файл
        {
            //QTime::currentTime() <<sender<< id;
             QTime   time;
            QString sender;
            QString fileName;
            int id;
            in>>time>>sender>>fileName>>id;
             if(sender==reciever)
             {
                 m_ptxtInfo->append(time.toString() + " " +sender+" отправил файл\""+"<a href = \""+QString::number(id)+"\" >"+ fileName.section("/",-1)+"</a>"+"\".");
                 m_ptxtInfo->setAlignment(Qt::AlignLeft);
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
            QString fileName;
            in>>fileName;
            QByteArray q = m_pTcpSocket->readAll();
            qDebug()<<q.size();

           QString filePath = QFileDialog::getSaveFileName(this, "Сохранить файл", fileName);
            if(filePath=="") return;
               QFile target(filePath);

               if (!target.open(QIODevice::WriteOnly)) {
                   qDebug() << "Невозможно открыть файл";
                   return;
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
            m_pTcpSocket->disconnectFromHost();
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
                     QString(m_pTcpSocket->errorString())
                    );
    m_ptxtInfo->append(strError);
}
void MyClient::slotSendToServer()// обычное сообщение
{if(m_ptxtInput->text()=="") return;
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) <<0<< QTime::currentTime()<<reciever<<name<< m_ptxtInput->text();
    //
    m_ptxtInfo->append(m_ptxtInput->text());
    m_ptxtInfo->setAlignment(Qt::AlignRight);

    //
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
    m_ptxtInput->setText("");
}
void MyClient::sendNameToServer()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) <<1<<name;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);

}
void MyClient::getListOfUsers()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);

    out << quint16(0) <<1<<name;// 1- запрос списка контактов


    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);

}
void MyClient::getHistory(QString user)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);

    out << quint16(0) <<2<<user<<name;// 2- запрос истории


    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);

}
void MyClient::slotConnected()
{m_ptxtInfo->clear();
    m_ptxtInfo->append("Подключение к серверу прошло успешно");
}
void MyClient::slotRecieveData(QList<QString> list)
{

    name=list[2];
    m_pTcpSocket->connectToHost(list[0], list[1].toInt());

   sendNameToServer();
   //nameLabel->setText("<H1>"+name+"<\/H1>");
   setWindowTitle("MESSenger - "+name);

//this->show();

}
void MyClient::onAnchorClicked(const QUrl &url)
{
QString str=url.toEncoded();
str=str.section("/",-1);
int id=str.toInt();
QByteArray  arrBlock;
QDataStream out(&arrBlock, QIODevice::WriteOnly);
out.setVersion(QDataStream::Qt_4_2);
out << quint16(0) <<4<<id;

out.device()->seek(0);
out << quint16(arrBlock.size() - sizeof(quint16));

m_pTcpSocket->write(arrBlock);

}
