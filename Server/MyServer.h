#include <QFileDialog>

#include <QWidget>
#include <QTime>
#include <QTcpServer.h>
#include <QTextEdit.h>
#include <QTcpSocket.h>

class User
{
public:
    QString name;
    QTcpSocket* socket;

};
struct Message
{
    int id;
    QTime time;
    QString sender;
    QString reciever;
    QString text;
    QString fileName;
    QByteArray file;
};
class History
{private:
    int currentId=0;
    const int maxNumberOfMessages=10;
    QList<Message> list;
public:
    void addToHistory(QTime time, QString sender, QString reciever, QString str);
    int addToHistory(QTime time, QString sender, QString reciever,QString fileName, QByteArray file);
    Message findMessageById(int id);
    QList<Message> getMessages(QString user1,QString user2);
};
class MyServer : public QWidget {
Q_OBJECT
public:
    QList<User> users;
    QTcpServer* m_ptcpServer;

    QTextEdit*  usersTextEdit;
    QTextEdit*  chatLogTextEdit;
    quint16     m_nNextBlockSize;
    MyServer(int nPort, QWidget* pwgt = 0);


private:
    History history;

void sendToClient(QTcpSocket* pSocket, const QString& str);
void sendToClient(QList<User> list, const QString& str);
void sendToClient(User user, QList<User> list);
void sendToClient(QTcpSocket* pSocket, bool mode);
void sendToClient(User user,const QString&sender, const QString& str);
void sendToClient(User user, QList<Message> list);
void sendIdToClient(User user,QString sender, QString fileName, int id);
void sendFileToClient(QTcpSocket* pClientSocket, int id);
User findUserByName(QString name);
User findUserBySocket(QTcpSocket* socket);
public slots:
    virtual void slotNewConnection();
            void slotReadClient   ();
            void handleDisconnect();
};


