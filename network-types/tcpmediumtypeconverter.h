#ifndef TCPMEDIUMTYPECONVERTER_H
#define TCPMEDIUMTYPECONVERTER_H

#include <QVariantHash>
#include <QStringList>

#include "tcpservermediumtypes.h"


class TcpMediumTypeConverter
{
public:
   static QStringList convertActiveConnection2history(const QVariantHash &activeconn, const qint64 &msecdisconn, const QString &remip, const QString &descr);

   static bool isIpGood(const QString &ip, const QStringList &whitelist);

   static TcpMediumServerSett convertFromVarHash(const QVariantHash &onesett, const QVariantHash &defsett);


};

#endif // TCPMEDIUMTYPECONVERTER_H
