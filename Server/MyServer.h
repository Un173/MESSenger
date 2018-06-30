

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
    QTime time;
    QString sender;
    QString reciever;
    QString text;
};
class History
{private:
    QList<Message> list;
public:
    void addToHistory(QTime time, QString sender, QString reciever, QString str);
    QList<Message> getMessages(QString user1,QString user2);
};
class MyServer : public QWidget {
Q_OBJECT
public:
    QList<User> users;
    QTcpServer* m_ptcpServer;
    QTextEdit*  m_ptxt;
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
User findUserByName(QString name);
User findUserBySocket(QTcpSocket* socket);
public slots:
    virtual void slotNewConnection();
            void slotReadClient   ();
            void handleDisconnect();
};


