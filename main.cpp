#include <QCoreApplication>

#include "thelordofservicescover.h"
#include <QTextStream>
#include <QDebug>

///[!] type-converter
#include "src/base/prettyvalues.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if(qApp->arguments().contains("-d")){ //build version
        QTextStream qout(stdout); //I connect the stout to my qout textstream
        qout << MYAPP_VERSION << " " << PrettyValues::getBuildDateStr() ;
        return 0;
    }

    if(qApp->arguments().contains("-v")){ //build version
        qDebug() <<  QString("tcp-medium-server-bbb %1").arg(QString(MYAPP_VERSION));
        return 0;
    }

    TheLordOfServicesCover cover;
    cover.setupYourSelf();

    return a.exec();
}
