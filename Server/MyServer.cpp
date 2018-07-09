#include "MyServer.h"

MyServer::MyServer(int nPort, QWidget* pwgt) : QWidget(pwgt)
                                                    , m_nNextBlockSize(0)
{
    server = new QTcpServer(this);
    if (!server->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(0,
                              "Ошибка сервера",
                              "Невозможно создать сервер:"
                              + server->errorString()
                             );
        server->close();
        return;
    }
    connect(server, SIGNAL(newConnection()),this,SLOT(slotNewConnection()));
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
    QTcpSocket* pClientSocket = server->nextPendingConnection();
    connect(pClientSocket, SIGNAL(disconnected()),pClientSocket, SLOT(deleteLater()));
    connect(pClientSocket, SIGNAL(readyRead()),this,SLOT(slotReadClient()));
   User user;
   user.socket=pClientSocket;
   connect(pClientSocket, SIGNAL(disconnected()),this,SLOT(handleDisconnect()));
   users<<user;
   sendToClient(user, users);
}
void MyServer::handleDisconnect()
{
    auto sender=QObject::sender();

     for(int i=0;i<users.length();i++)
     {
         if(sender==users[i].socket)
         {
             users.removeAt(i);
             break;
         }
     }
    usersTextEdit->clear();
    for (User &user: users)
    {
        usersTextEdit->append(user.name+",");
        sendToClient(user, users);
    }
}
void MyServer::slotReadClient()
{
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    QDataStream in(clientSocket);

    in.setVersion(QDataStream::Qt_4_2);
       if (m_nNextBlockSize == 0) {
               if (clientSocket->bytesAvailable() < sizeof(quint64))
               return;
               in >> m_nNextBlockSize;
           }
           if (clientSocket->bytesAvailable() < m_nNextBlockSize)
               return;
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
            sendFileToClient(clientSocket, id);
            break;
        }
        case 3: //Загружен файл
        {
            QString reciever;
            QString fileName;
            int size;
            in>>time>>sender>>reciever>>fileName>>size;
           QByteArray q = clientSocket->read(size);
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
            User u=findUserBySocket(clientSocket);
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
                     sendToClient(clientSocket, false);
                     m_nNextBlockSize = 0;
                     return;
                 }
                 sendToClient(clientSocket, true);
            for (User &user: users) {
                if(user.socket==clientSocket)  user.name=sender;

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
            User u=findUserByName(reciever);
            history.addToHistory(time,sender,reciever,str);
            sendToClient(u,sender,str);
            break;
        }

        }
  m_nNextBlockSize = 0;
    }

void MyServer::sendIdToClient(User user, QString sender, QString fileName, int id)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint64(0) <<3<< QTime::currentTime() <<sender<<fileName<< id;
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));

    user.socket->write(arrBlock);
}
void MyServer::sendFileToClient(QTcpSocket* pClientSocket, int id)
{Message m=history.findMessageById(id);
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint64(0)<<4<<m.fileName<<m.file.size();
    arrBlock.append(m.file);
    qDebug()<<m.file.size();
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));
    pClientSocket->write(arrBlock);
}
void MyServer::sendToClient(QTcpSocket* pSocket, const QString& str)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint64(0) << QTime::currentTime() << str;

    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));

    pSocket->write(arrBlock);
}
void MyServer::sendToClient(User user,const QString&sender, const QString& str)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint64(0) <<0<< QTime::currentTime() <<sender<< str;

    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));

    user.socket->write(arrBlock);
}
void MyServer::sendToClient(User user, QList<User> list)
{
    if(list.length()==0) return;
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint64(0)<<2;//режим отправки контактов
QList<QString> names;
    foreach (User u, list)
    {
        if(u.name!=user.name&&u.name!="")
        names<<u.name;
    }
     out<<names;
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));

    user.socket->write(arrBlock);

}
void MyServer::sendToClient(User user, QList<Message> list)
{
    if(list.length()==0) return;

    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint64(0)<<1;//режим отправки истории
   QList<QList<QString>> result;
   foreach (Message m, list) {
       QList<QString> l;
       l<<QString::number(m.id)<<m.time.toString()<<m.sender<<m.reciever<<m.text<<m.fileName;
       result<<l;
   }
    out<<result;
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));
    user.socket->write(arrBlock);

}
void MyServer::sendToClient(QTcpSocket* pSocket, bool mode)
{

    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint64(0)<<-1<<mode;//режим подтверждения логина

    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));

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
