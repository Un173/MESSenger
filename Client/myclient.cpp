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

    m_ptxtInfo  = new QTextEdit;
    m_ptxtInput = new QLineEdit;
    nameLabel=new QLabel("<H2>Список контактов:<\/H2>");
m_ptxtInput->setReadOnly(true);
    connect(m_ptxtInput, SIGNAL(returnPressed()),
            this,        SLOT(slotSendToServer())
           );
    m_ptxtInfo->setReadOnly(true);

    QPushButton* pcmd = new QPushButton("Отправить");
    connect(pcmd, SIGNAL(clicked()), SLOT(slotSendToServer()));

     listWidget = new QListWidget(this);
    /*new QListWidgetItem(tr("Oak"), listWidget);
        new QListWidgetItem(tr("Fir"), listWidget);
        new QListWidgetItem(tr("Pine"), listWidget);*/
QMenuBar *menuBar=new QMenuBar(this);


   menuBar->addAction("Смена пользователя", this, SLOT(slotChangeUser()) );
//menuBar->show();
   QVBoxLayout* vLayout1 = new QVBoxLayout;

   vLayout1->addWidget(nameLabel);
   vLayout1->addWidget(listWidget);
   QVBoxLayout* vLayout2 = new QVBoxLayout;
   vLayout2->addWidget(new QLabel("<H2>Сообщения:<\/H2>"));
   vLayout2->addWidget(m_ptxtInfo);
   vLayout2->addWidget(m_ptxtInput);
   vLayout2->addWidget(pcmd);

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
void MyClient::slotChangeUser()
{
    m_pTcpSocket->disconnectFromHost();
    this->close();
    emit reopenConnectionWidget();
}
void MyClient::onListItemClick(QListWidgetItem *item)
{
m_ptxtInfo->clear();
reciever=item->text();
getHistory(reciever);
m_ptxtInput->setReadOnly(false);
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

}
            break;
        }
        case 1:
        {

        QList<QList<QString>> messages;//Получение истории
        in>>messages;
        foreach (QList<QString> l, messages) {
           //   l<<m.time.toString()<<m.sender<<m.reciever<<m.text;

             if(reciever==l[1])
             {
                  m_ptxtInfo->append(l[0] + " " +l[1]+": "+ l[3]);
                  m_ptxtInfo->setAlignment(Qt::AlignLeft);
             }
             else
             {
                 m_ptxtInfo->append(l[3]);
                 m_ptxtInfo->setAlignment(Qt::AlignRight);
             }
        }

        break;
        }
        case 2://Получение контактов
        {

            users.clear();
            in>>users;

            listWidget->clear();
            foreach (QString str, users) {

                  new QListWidgetItem(str, listWidget);
            }

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
{
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
