#ifndef _MyClient_h_
#define _MyClient_h_

#include <QWidget>
#include <QTcpSocket>

#include<QTextEdit>
#include<QLineEdit>
#include<QPushButton>
#include <QMessageBox>
#include <QLayout>
#include <QLabel>
#include <QTime>
#include <QListWidget>

class MyClient : public QWidget {
Q_OBJECT
private:
    QTcpSocket* m_pTcpSocket;
    QTextEdit*  m_ptxtInfo;
    QLineEdit*  m_ptxtInput;
    quint16     m_nNextBlockSize;
    QString     name;
    QList<QString> userList;
    QListWidget *listWidget;

public:
    MyClient(QWidget* pwgt = 0);
    void sendNameToServer();
signals:
private:
      void getListOfUsers();

private slots:
    void slotReadyRead   (                            );
    void slotError       (QAbstractSocket::SocketError);
    void slotSendToServer(                            );
    void slotConnected   (                            );
    bool slotRecieveData(QList<QString>);


};
#endif  //_MyClient_h_
