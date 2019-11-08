#ifndef TCPSERVERMEDIUMTYPES_H
#define TCPSERVERMEDIUMTYPES_H

#include <QStringList>


struct TcpMediumServerSett
{
    bool enable;
    int maxconn;
    int seckill;
    QString memo;
    QStringList whitelist;

    TcpMediumServerSett() : enable(false) {}

    TcpMediumServerSett(const bool &enable, const int &maxconn, const int &seckill, const QString &memo, const QStringList &whitelist) :
        enable(enable), maxconn(maxconn), seckill(seckill), memo(memo), whitelist(whitelist) {}
};

#endif // TCPSERVERMEDIUMTYPES_H
