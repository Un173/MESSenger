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

    connect(m_ptxtInput, SIGNAL(returnPressed()),
            this,        SLOT(slotSendToServer())
           );
    m_ptxtInfo->setReadOnly(true);

    QPushButton* pcmd = new QPushButton("&Send");
    connect(pcmd, SIGNAL(clicked()), SLOT(slotSendToServer()));

     listWidget = new QListWidget(this);
    /*new QListWidgetItem(tr("Oak"), listWidget);
        new QListWidgetItem(tr("Fir"), listWidget);
        new QListWidgetItem(tr("Pine"), listWidget);*/
    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Client</H1>"));
    pvbxLayout->addWidget(listWidget);
    pvbxLayout->addWidget(m_ptxtInfo);
    pvbxLayout->addWidget(m_ptxtInput);
    pvbxLayout->addWidget(pcmd);
    setLayout(pvbxLayout);
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
        case 0://Получение сообщения
        {
            QTime   time;
            QString str;
            in >> time >> str;

            m_ptxtInfo->append(time.toString() + " " + str);

            break;
        }
        case 2://Получение контактов
        {
            QList<QString> list;
            in>>list;
            listWidget->clear();
            foreach (QString str, list) {

                  new QListWidgetItem(str, listWidget);
            }


            break;
        }
        }
        m_nNextBlockSize = 0;
    }
}
void MyClient::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );
    m_ptxtInfo->append(strError);
}
void MyClient::slotSendToServer()// обычное сообщение
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) <<0<< QTime::currentTime()<<name << m_ptxtInput->text();

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
    bool mode=true;
    out << quint16(0) <<mode<<name;// true- значит запрос списка контактов

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);

}
void MyClient::slotConnected()
{
    m_ptxtInfo->append("Подключение к серверу прошло успешно");
}
bool MyClient::slotRecieveData(QList<QString> list)
{
    //проверка на доступность и отправка обратно

   // emit getListOfUsers();
    name=list[2];
    m_pTcpSocket->connectToHost(list[0], list[1].toInt());
   sendNameToServer();

this->show();
    return true;
}
