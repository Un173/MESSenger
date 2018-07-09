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
       if (m_nNextBlockSize == 0) {
               if (pClientSocket->bytesAvailable() < sizeof(quint64))
               return;
               in >> m_nNextBlockSize;
           }
           if (pClientSocket->bytesAvailable() < m_nNextBlockSize)
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
            sendFileToClient(pClientSocket, id);
            break;
        }
        case 3: //Загружен файл
        {
            //3<< QTime::currentTime()<<reciever<<name<<file->fileName()<<q;
            QString reciever;
            QString fileName;
            int size;
            in>>time>>sender>>reciever>>fileName>>size;



           /* QString path = QFileDialog::getSaveFileName(this, "Сохранить файл", fileName);

              QFile *file = new QFile(path);                            //Create the file
               file->open(QIODevice::WriteOnly);                  //Open it

           quint64 sizeread=0;
           quint64 length=0;
               //Download it in chunks, so the memory won't crash
               char *data = new char[size];
               length = pClientSocket->read(data, size);                //Buffer 1MB first, max.
               file->write(data,length);
               sizeread += length;
               delete [] data;

               if ((int)sizeread == size)                 //File size reached, closing file
               {
                   file->close();
                   delete file;

                   length=0;
                   sizeread=0;
               }*/
           QByteArray q = pClientSocket->read(size);
qDebug() <<q.size();




         /*   QByteArray q = pClientSocket->readAll();
             qDebug() <<q.size();*/


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
