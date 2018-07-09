#include <QApplication>
#include <Qwidget>
#include "MyClient.h"
#include "connectionwidget.h"


int main(int argc, char** argv)
{

    QApplication app(argc, argv);
    ConnectionWidget w;
     w.show();
     QWidget* pwgt=0;
     MyClient     client(pwgt);
  QObject::connect(&w, SIGNAL(sendDataToMainWindow(QList<QString>)), &client, SLOT(slotRecieveData(QList<QString>)));
  QObject::connect(&client, SIGNAL(sendAllowanceResult(int)), &w, SLOT(dataReturned(int)));
  QObject::connect(&client, SIGNAL(reopenConnectionWidget()), &w, SLOT(slotForOpening()));

    return app.exec();
}

