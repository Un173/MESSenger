#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QtWidgets>
class ConnectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectionWidget(QWidget *parent = nullptr);
private:

    QLineEdit *addressLineEdit= new QLineEdit("localhost");
    QLineEdit *portLineEdit= new QLineEdit("2323");
    QLineEdit *nameLineEdit= new QLineEdit("Алиса");
signals:
 void SendDataToMainWindow(QList<QString>);
public slots:
      void OKButtonClicked();
};

#endif // CONNECTIONWIDGET_H
