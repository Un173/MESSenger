

#include <QWidget>

#include <QTcpServer.h>
#include <QTextEdit.h>
#include <QTcpSocket.h>

class User
{
public:
    QString name;
    QTcpSocket* socket;

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
    void sendToClient(QTcpSocket* pSocket, const QString& str);
void sendToClient(QList<User> list, const QString& str);
void sendToClient(User user, QList<User> list);
void sendToClient(QTcpSocket* pSocket, bool mode);
QTcpSocket* findSocket(QString str);
User findUser( QTcpSocket* socket);
public slots:
    virtual void slotNewConnection();
            void slotReadClient   ();
            void handleDisconnect();
};


