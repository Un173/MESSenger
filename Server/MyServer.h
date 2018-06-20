

#include <QWidget>

#include <QTcpServer.h>
#include <QTextEdit.h>
#include <QTcpSocket.h>


class MyServer : public QWidget {
Q_OBJECT
public:
    QTcpServer* m_ptcpServer;
    QTextEdit*  m_ptxt;
    quint16     m_nNextBlockSize;
    MyServer(int nPort, QWidget* pwgt = 0);


private:
    void sendToClient(QTcpSocket* pSocket, const QString& str);

public slots:
    virtual void slotNewConnection();
            void slotReadClient   ();
};

