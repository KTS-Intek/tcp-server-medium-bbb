#ifndef TCPSERVERMEDIUMLOCALSOCKET_H
#define TCPSERVERMEDIUMLOCALSOCKET_H

///[!] ipc
#include "localsockets/regularlocalsocket.h"

#include <QObject>
#include <QtCore>


class TcpServerMediumLocalSocket : public RegularLocalSocket
{
    Q_OBJECT
public:
    explicit TcpServerMediumLocalSocket(bool verboseMode, QObject *parent = nullptr);

    void decodeReadData(const QVariant &dataVar, const quint16 &command);

signals:

public slots:
};

#endif // TCPSERVERMEDIUMLOCALSOCKET_H
