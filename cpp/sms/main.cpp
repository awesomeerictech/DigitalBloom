#include <QCoreApplication>
#include "sms.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Sms eric;
    eric.singleMessage("awesomeerictech","+254742954736,","Just Testing the shortcode with bulk sms off, regards Director Eric","ANETQUICK","0");
 //  eric.singleMessage("awesomeerictech","+254742954736,+254715785746,+254727802446","Good afternoon Directors, regards Director Eric","ANETQUICK","1");
    return a.exec();
}
