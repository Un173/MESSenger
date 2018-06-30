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
void MyServer::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();


    connect(pClientSocket, SIGNAL(disconnected()),
            pClientSocket, SLOT(deleteLater())
           );


    connect(pClientSocket, SIGNAL(readyRead()),
            this,          SLOT(slotReadClient())
           );

    //sendToClient(pClientSocket, "Server Response: Connected!");

   User user;
   user.socket=pClientSocket;

   connect(pClientSocket, SIGNAL(disconnected()),
           this, SLOT(handleDisconnect())
          );

 users<<user;
  sendToClient(user, users);

}
void MyServer::handleDisconnect()
{
    auto sender=QObject::sender();

     for(int i=0;i<users.length();i++)
     {
         if(sender==users[i].socket) {users.removeAt(i);break;}

     }
    for (User &user: users) {
      sendToClient(user, users);
}

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
        QString sender;
        in>>mode;


        switch (mode)
        {
        case 1://Подключился пользователь
        {
            in>>sender;
             for (User &user: users)
                 if(user.name==sender)
                 {
                     sendToClient(pClientSocket, false);
                     m_nNextBlockSize = 0;
                     return;
                 }
                 sendToClient(pClientSocket, true);
            for (User &user: users) {
                if(user.socket==pClientSocket)  user.name=sender;

            }
            m_ptxt->clear();
              for (User &user: users) {

                  m_ptxt->append(user.name+",");
                sendToClient(user, users);
            }
            break;
        }
        case 0:// Обычное сообщение
        {QString reciever;
            in >> time >>reciever>>sender>>str;
            QString strMessage =
                time.toString() + " " + sender+" has sended - " + str;
            m_ptxt->append(strMessage);

            /*sendToClient(pClientSocket,
                         "Server Response: Received \"" + str + "\""
                        );*/
            User u=findUserByName(reciever);
            history.addToHistory(time,sender,reciever,str);
            sendToClient(u,sender,str
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
void MyServer::sendToClient(User user,const QString&sender, const QString& str)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) <<0<< QTime::currentTime() <<sender<< str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    user.socket->write(arrBlock);
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
void MyServer::sendToClient(QTcpSocket* pSocket, bool mode)
{

    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0)<<-1<<mode;//режим подтверждения логина

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

   pSocket->write(arrBlock);

}
User MyServer::findUserByName(QString name)
{
    foreach (User user, users) {
        if(user.name==name) return user;
    }

}
void History::addToHistory(QTime time, QString sender, QString reciever, QString str)
{ Message m;
    m.time=time;
    m.sender=sender;
    m.reciever=reciever;
    m.text=str;
    list<<m;
}
QList<Message> History::getMessages(QString user1,QString user2)
{
    QList<Message> result;
    foreach (Message m, list) {
        if((m.sender==user1&&m.reciever==user2)||(m.sender==user2&&m.reciever==user1)) result<<m;
    }
    return result;
}
