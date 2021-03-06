#include "connectionwidget.h"

ConnectionWidget::ConnectionWidget(QWidget *parent) : QWidget(parent)
{


QPushButton *btnOK = new QPushButton("OK", this);
QVBoxLayout* pvbxLayout = new QVBoxLayout;
pvbxLayout->addWidget(new QLabel("<H2>Введите адрес сервера:</H2>"));
pvbxLayout->addWidget(addressLineEdit);
pvbxLayout->addWidget(new QLabel("<H2>Введите порт сервера:</H2>"));
pvbxLayout->addWidget(portLineEdit);
pvbxLayout->addWidget(new QLabel("<H2>Введите ваше имя:</H2>"));
pvbxLayout->addWidget(nameLineEdit);
pvbxLayout->addWidget(btnOK);
setLayout(pvbxLayout);
 connect(btnOK, SIGNAL(clicked()), this, SLOT(okButtonClicked()));
}
void ConnectionWidget::slotForOpening()
{
    this->show();
}
void ConnectionWidget::okButtonClicked()
{
    QList<QString> list;
    list<<addressLineEdit->text()<<portLineEdit->text()<<nameLineEdit->text();
   emit sendDataToMainWindow(list);
    //this->close();
}
void ConnectionWidget::dataReturned(int result)
{
    switch(result)
    {
    case 0:
    {
        this->close();
        break;
    }
    case 1:
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Ошибка","Выбранное имя уже занято");
        messageBox.setFixedSize(500,200);
        break;
    }
    }
}
