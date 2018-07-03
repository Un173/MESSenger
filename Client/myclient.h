#ifndef _MyClient_h_
#define _MyClient_h_

#include <QWidget>
#include <QTcpSocket>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include<QTextEdit>
#include<QLineEdit>
#include<QPushButton>
#include <QMessageBox>
#include <QLayout>
#include <QLabel>
#include <QTime>
#include <QListWidget>
#include <QPair>
class MyClient : public QWidget {
Q_OBJECT
private:
    QTcpSocket* m_pTcpSocket;
    QTextEdit*  m_ptxtInfo;
    QLineEdit*  m_ptxtInput;
    quint16     m_nNextBlockSize;
    QString     name;
    QLabel *nameLabel;
    QList<QString> userList;
    QListWidget *listWidget;
    QList<QPair<QString,int>> users;
    QString reciever;
    bool allowed=false;

public:
    MyClient(QWidget* pwgt = 0);
    void sendNameToServer();
signals:
    void sendAllowanceResult(int);
    void reopenConnectionWidget();
private:
      void getListOfUsers();
void getHistory(QString user);
private slots:
    void slotReadyRead   (                            );
    void slotError       (QAbstractSocket::SocketError);
    void slotSendToServer(                            );
    void slotConnected   (                            );
    void slotRecieveData(QList<QString>);
    void onListItemClick(QListWidgetItem*);
    void slotChangeUser();

};

#endif  //_MyClient_h_
