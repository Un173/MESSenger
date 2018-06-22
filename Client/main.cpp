#include <QApplication>
#include <Qwidget>
#include "MyClient.h"
#include "connectionwidget.h"


int main(int argc, char** argv)
{

    QApplication app(argc, argv);

 /*   QWidget *widget = new QWidget;
       Ui::ConnectionWidget ui;
       ui.setupUi(widget);

       widget->show();
*/
       ConnectionWidget w;

       w.show();
QWidget*       pwgt=0;
         MyClient     client(pwgt);
  QObject::connect(&w, SIGNAL(SendDataToMainWindow(QList<QString>)), &client, SLOT(slotRecieveData(QList<QString>)));
         //client.show();
    return app.exec();
}

