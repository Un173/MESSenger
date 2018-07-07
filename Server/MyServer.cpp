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
                              "Ошибка сервера",
                              "Невозможно создть сервер:"
                              + m_ptcpServer->errorString()
                             );
        m_ptcpServer->close();
        return;
    }
    connect(m_ptcpServer, SIGNAL(newConnection()),
            this,         SLOT(slotNewConnection())
           );


    usersTextEdit = new QTextEdit;
    usersTextEdit->setReadOnly(true);
    chatLogTextEdit=new QTextEdit;
     chatLogTextEdit->setReadOnly(true);


    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Server</H1>"));
    pvbxLayout->addWidget(new QLabel("<H2>Пользователи в сети:</H2>"));
    pvbxLayout->addWidget(usersTextEdit);
    pvbxLayout->addWidget(new QLabel("<H2>Лог чата:</H2>"));
    pvbxLayout->addWidget(chatLogTextEdit);

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
    usersTextEdit->clear();
      for (User &user: users) {

          usersTextEdit->append(user.name+",");
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
        case 4://Запрос на загрузку файла
        {
            int id;
        in>>id;
            sendFileToClient(pClientSocket, id);
            break;
        }
        case 3: //Загружен файл
        {
            //3<< QTime::currentTime()<<reciever<<name<<file->fileName()<<q;
            QString reciever;
            QString fileName;
            in>>time>>sender>>reciever>>fileName;
            QByteArray q = pClientSocket->readAll();
             qDebug() <<q.size();
            int id= history.addToHistory(time,sender,reciever,fileName.section("/",-1),q);
            User user=findUserByName(reciever);
        sendIdToClient(user, sender, fileName,id);

        QString strMessage =
            time.toString() + " " + sender+" отправил файл пользователю "+reciever+" :" + fileName.section("/",-1);
        chatLogTextEdit->append(strMessage);
            break;
        }
        case 2:// Запрос на историю
        {
            QString reciever;
            in>>sender;
            in>>reciever;
            User u=findUserBySocket(pClientSocket);
            QList<Message> messages=history.getMessages(sender,reciever);
            sendToClient(u,messages);
            break;
        }
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
            usersTextEdit->clear();
              for (User &user: users) {

                  usersTextEdit->append(user.name+",");
                sendToClient(user, users);
            }
            break;
        }
        case 0:// Обычное сообщение
        {QString reciever;
            in >> time >>reciever>>sender>>str;
            QString strMessage =
                time.toString() + " " + sender+" отправил пользователю "+reciever+" :" + str;
            chatLogTextEdit->append(strMessage);

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
void MyServer::sendIdToClient(User user, QString sender, QString fileName, int id)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) <<3<< QTime::currentTime() <<sender<<fileName<< id;
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    user.socket->write(arrBlock);
}
void MyServer::sendFileToClient(QTcpSocket* pClientSocket, int id)
{Message m=history.findMessageById(id);
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0)<<4<<m.fileName;
    arrBlock.append(m.file);
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));
    pClientSocket->write(arrBlock);
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
void MyServer::sendToClient(User user, QList<Message> list)
{
    if(list.length()==0) return;

    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0)<<1;//режим отправки истории
   QList<QList<QString>> result;
   foreach (Message m, list) {
       QList<QString> l;
       l<<QString::number(m.id)<<m.time.toString()<<m.sender<<m.reciever<<m.text<<m.fileName;
       result<<l;
   }
    out<<result;
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
User MyServer::findUserBySocket(QTcpSocket *socket)
{
    foreach (User user, users) {
        if(user.socket==socket) return user;
    }

}
int History::addToHistory(QTime time, QString sender, QString reciever,QString fileName, QByteArray file)
 {
     int count=0;
     foreach (auto a, list)
     {
         if((a.sender==sender&&a.reciever==reciever)||(a.sender==reciever&&a.reciever==sender))
             count++;
     }
     if(count>maxNumberOfMessages)
     {
         for(int i=0;i<list.length();i++)
         {
             if((list[i].sender==sender&&list[i].reciever==reciever)||(list[i].sender==reciever&&list[i].reciever==sender))
             {
                 count--;
                 list.removeAt(i);
             }
             if(count<=maxNumberOfMessages) break;
         }
     }
     Message m;
     m.id=currentId;
     m.time=time;
     m.sender=sender;
     m.reciever=reciever;
     m.text="";
     m.file=file;
     m.fileName=fileName;
     list<<m;
     currentId++;
     return m.id;
 }
void History::addToHistory(QTime time, QString sender, QString reciever, QString str)
{
    int count=0;
    foreach (auto a, list)
    {
        if((a.sender==sender&&a.reciever==reciever)||(a.sender==reciever&&a.reciever==sender))
            count++;
    }
    if(count>maxNumberOfMessages)
    {
        for(int i=0;i<list.length();i++)
        {
            if((list[i].sender==sender&&list[i].reciever==reciever)||(list[i].sender==reciever&&list[i].reciever==sender))
            {
                count--;
                list.removeAt(i);
            }
            if(count<=maxNumberOfMessages) break;
        }
    }
    Message m;
    m.id=currentId;
    m.time=time;
    m.sender=sender;
    m.reciever=reciever;
    m.text=str;
    m.fileName="";
    list<<m;
    currentId++;
}
QList<Message> History::getMessages(QString user1,QString user2)
{
    QList<Message> result;
    foreach (Message m, list) {
        if((m.sender==user1&&m.reciever==user2)||(m.sender==user2&&m.reciever==user1)) result<<m;
    }
    return result;
}
Message History::findMessageById(int id)
{
    foreach (Message m, list) {
        if(m.id==id) return m;
    }
}
