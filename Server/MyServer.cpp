#include "MyServer.h"
#include <QMessageBox>
#include <QLayout>
#include <QLabel>
#include <QTime>
MyServer::MyServer(int nPort, QWidget* pwgt /*=0*/) : QWidget(pwgt)
                                                    , m_nNextBlockSize(0)
{
    m_ptcpServer = new QTcpServer(this);
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(0,
                              "Server Error",
                              "Unable to start the server:"
                              + m_ptcpServer->errorString()
                             );
        m_ptcpServer->close();
        return;
    }
    connect(m_ptcpServer, SIGNAL(newConnection()),
            this,         SLOT(slotNewConnection())
           );

    m_ptxt = new QTextEdit;
    m_ptxt->setReadOnly(true);

    //Layout setup
    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Server</H1>"));
    pvbxLayout->addWidget(m_ptxt);
    setLayout(pvbxLayout);
}
/*virtual*/ void MyServer::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();

    connect(pClientSocket, SIGNAL(disconnected()),
            pClientSocket, SLOT(deleteLater())
           );//Тут еще подключить слот, который удалял бы этого человека из списка сокетов
    connect(pClientSocket, SIGNAL(readyRead()),
            this,          SLOT(slotReadClient())
           );
//todo: Послать сообщение с контактами в сети
    //sendToClient(pClientSocket, "Server Response: Connected!");

   User user;
   user.socket=pClientSocket;

    //sendToClient(user, users);
 users<<user;

}
void MyServer::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_4_2);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (pClientSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (pClientSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }

         int   mode;
        QTime   time;
        QString str;
        QString name;
        in>>mode;


        switch (mode)
        {
        case 1:
        {
            in>>name;
            for (User &user: users) {
                if(user.socket==pClientSocket)  user.name=name;

            }
              for (User &user: users) {
       sendToClient(user, users);
            }
            break;
        }
        case 0:
        {
            in >> time >>name>> str;
            QString strMessage =
                time.toString() + " " + name+" has sended - " + str;
            m_ptxt->append(strMessage);

            /*sendToClient(pClientSocket,
                         "Server Response: Received \"" + str + "\""
                        );*/
            sendToClient(users,
                         name+": "+ str
                        );
            break;
        }
        }

         m_nNextBlockSize = 0;
    }
}
void MyServer::sendToClient(QTcpSocket* pSocket, const QString& str)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) << QTime::currentTime() << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}
void MyServer::sendToClient(QList<User> list, const QString& str)
{
    foreach(User user, list)
    {
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) << QTime::currentTime() << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    user.socket->write(arrBlock);
    }
}
void MyServer::sendToClient(User user, QList<User> list)
{
    if(list.length()==0) return;
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0)<<2;//режим отправки контактов
QList<QString> names;
    foreach (User u, list)
    {
        if(u.name!=user.name&&u.name!="")
        names<<u.name;
    }
     out<<names;
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    user.socket->write(arrBlock);

}
QTcpSocket* findSocket(QString str)
{

}

