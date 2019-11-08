#include "tcpmediumtypeconverter.h"

#include <QRegularExpression>



//-----------------------------------------------------------------------------------------------------------

QStringList TcpMediumTypeConverter::convertActiveConnection2history(const QVariantHash &activeconn, const qint64 &msecdisconn, const QString &remip, const QString &descr)
{
    QStringList l;
   /* - actl - active connections QVH (IP to Socket descriptor)  <ip>\t<desr> (key) - msec - rcvd - trnsm
   *                          - disl - history of connections QSL (last 50 connections - connected msec/disconnected msec/remote ip/descriptor/received/transmitted (space separated)
   * */

    l.append(activeconn.value("msec").toString());
    l.append(QString::number(msecdisconn));
    l.append(remip);
    l.append(descr);
    l.append(activeconn.value("rcvd").toString());
    l.append(activeconn.value("trnsm").toString());

    return l;
}

//-----------------------------------------------------------------------------------------------------------

bool TcpMediumTypeConverter::isIpGood(const QString &ip, const QStringList &whitelist)
{
    if(whitelist.isEmpty())
        return true;

    for(int i = 0, imax = whitelist.size(); i < imax; i++){

        const QRegularExpression rx(whitelist.at(i), QRegularExpression::ExtendedPatternSyntaxOption);

        if(rx.isValid()){
            if(rx.match(ip, 0, QRegularExpression::NormalMatch).hasMatch()){
                return true;
            }
        }else{
            if(ip == whitelist.at(i)){
                return true;
            }
        }


    }
    return false;
}

//-----------------------------------------------------------------------------------------------------------

TcpMediumServerSett TcpMediumTypeConverter::convertFromVarHash(const QVariantHash &onesett, const QVariantHash &defsett)
{
    TcpMediumServerSett sett;
    sett.enable = onesett.value("en"    , defsett.value("en")   ).toBool();
    sett.seckill = onesett.value("sec"  , defsett.value("sec")  ).toInt();
    sett.maxconn = onesett.value("max"  , defsett.value("max")  ).toInt();
    sett.memo = onesett.value("memo"    , defsett.value("memo") ).toString();
    sett.whitelist = onesett.value("wl" , defsett.value("wl")   ).toStringList();

    return sett;
}

//-----------------------------------------------------------------------------------------------------------