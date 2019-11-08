#include "themediumtcpserver.h"

#include <QTimer>
#include <QDateTime>

#include "network-types/tcpmediumtypeconverter.h"
#include "network-src/tcpmediumsocket.h"

//-------------------------------------------------------------------------------

TheMediumTcpServer::TheMediumTcpServer(QObject *parent) : QTcpServer(parent)
{

}

//-------------------------------------------------------------------------------

bool TheMediumTcpServer::setServerSett(const TcpMediumServerSett &sett, const quint16 &port, const QVariantHash &hashhistory)
{
    mysett.sett = sett;
    mysett.port = port;
    mysett.activeconnections.clear();
    mysett.log = hashhistory.value("log").toStringList();
    mysett.historyofconnections = hashhistory.value("disl").toStringList();
    setMaxPendingConnections(5);
    return sett.enable;
}

//-------------------------------------------------------------------------------

bool TheMediumTcpServer::startServer()
{
    if(isListening()){
        emit stopAllSocketsSlot();
        close();
    }
    append2logService(tr("The server is going to listen on the port %1 ").arg(QString::number(mysett.port)));

    const bool r = listen(QHostAddress::Any, mysett.port);
    if(!r){
        append2logService(tr("Couldn't listen, port %1, error %2").arg(QString::number(mysett.port)).arg(errorString()));
        close();
    }
    return r;
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::onThreadStarted()
{
    QTimer *tmrSendLater = new QTimer(this);
    tmrSendLater->setSingleShot(true);
    tmrSendLater->setInterval(1111);
    connect(this, SIGNAL(append2history(quint16,QVariantHash)), tmrSendLater, SLOT(start()));
    connect(tmrSendLater, &QTimer::timeout, this, &TheMediumTcpServer::sendInfoHashNow);

    mysett.isStopped = false;
    QTimer::singleShot(11, this, SLOT(restartServerLater()));
    QTimer::singleShot(1111, tmrSendLater, SLOT(start()));
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::stopServerAndKickOff(quint16 port)
{
    if(port == mysett.port && killObjectLater()){
        append2logService(tr("The server was stopped on the port %1 ").arg(QString::number(mysett.port)));
    }
}

void TheMediumTcpServer::stopServerForced()
{
    killObjectLater();
    close();
    deleteLater();
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::setNewSetts(int secs, int maxconn, QStringList whitelist)
{
    mysett.sett.seckill = secs;
    mysett.sett.maxconn = maxconn;
    mysett.sett.whitelist = whitelist;

    emit setSecs2kickOff(secs);

}

void TheMediumTcpServer::setOneServerSett(quint16 port, TcpMediumServerSett sett)
{
    if(port == mysett.port){
        setNewSetts(sett.seckill, sett.maxconn, sett.whitelist);
    }

}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::append2log(QString remip, QString descr, QString message)
{
    append2logService(QString("%1-%2 %3").arg(remip).arg(descr).arg(message));
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::onConnectionDown(QString remip, QString descr, QString message, qint64 msecdisconn)
{

    mysett.historyofconnections.append(TcpMediumTypeConverter::convertActiveConnection2history(mysett.activeconnections.take(getLocalIpDescrPair(remip, descr)).toHash(),
                                                                                               msecdisconn, remip, descr).join(" "));
    //connected msec/disconnected msec/remote ip/descriptor/received/transmitted (space separated)

    append2log(remip, descr, message);//sendInfoHash


}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::onReadWrite(QString remip, QString descr, quint64 readbytes, quint64 writebytes)
{
    QVariantHash oneconnection = mysett.activeconnections.value(getLocalIpDescrPair(remip, descr)).toHash();
    oneconnection.insert("rcvd", readbytes);
    oneconnection.insert("trnsm", writebytes);
    mysett.activeconnections.insert(getLocalIpDescrPair(remip, descr), oneconnection);
    sendInfoHash();
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::restartServerLater()
{
    if(mysett.isStopped)
        return;
    if(!startServer())
        QTimer::singleShot(3333, this, SLOT(restartServerLater()));
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::stopSocketSlotServer(quint16 port, QString ip, QString descr)
{
    if(mysett.port == port)
        emit stopSocketSlot(ip, descr);
}

//-------------------------------------------------------------------------------

QString TheMediumTcpServer::getLogLine(const QString &line)
{
    return QString("%1 %2").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss t")).arg(line);
}

//-------------------------------------------------------------------------------

QString TheMediumTcpServer::getLocalIpDescrPair(const QString &ip, const QString &descr)
{
    return QString("%1\t%2").arg(ip).arg(descr);
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::append2logService(const QString &line)
{
    mysett.log.append(getLogLine(line));

    sendInfoHash();
}

//-------------------------------------------------------------------------------

bool TheMediumTcpServer::killObjectLater()
{
    mysett.kickOffCounter++;
    emit stopAllSocketsSlot();
    if(!mysett.isStopped){
        mysett.isStopped = true;
        QTimer::singleShot(555, this, SLOT(deleteLater()));
        return true;
    }
    if(mysett.kickOffCounter > 10)
        deleteLater();
    return false;
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::sendInfoHash()
{
    mysett.sendHashCounter++;
    if(mysett.sendHashCounter > 20)
       sendInfoHashNow();
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::sendInfoHashNow()
{
    QVariantHash h;
    h.insert("actl", mysett.activeconnections);
    h.insert("disl", mysett.historyofconnections);
    if(mysett.log.size() > 100)
        mysett.log = mysett.log.mid(mysett.log.size() - 100); //remove old events

    h.insert("log", mysett.log);
    emit append2history(mysett.port, h);
    mysett.sendHashCounter = 0;
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::incomingConnection(qintptr socketDescr)
{
    TcpMediumSocket *socket = new TcpMediumSocket(this);
    if(!socket->setSocketDescriptor(socketDescr)){
        append2logService(tr("Broken connection"));
        qDebug() << "!incomingConnection " << socket->errorString();
        socket->deleteLater();
        return;
    }


    connect(socket, &TcpMediumSocket::append2log, this, &TheMediumTcpServer::append2log);
    connect(socket, &TcpMediumSocket::append2SmplLogSlot, this, &TheMediumTcpServer::append2logService);


    connect(this, &TheMediumTcpServer::setSecs2kickOff, socket, &TcpMediumSocket::setSecs2kickOff);
    connect(this, &TheMediumTcpServer::stopAllSocketsSlot, socket, &TcpMediumSocket::stopAllSlot);
    connect(this, &TheMediumTcpServer::stopSocketSlot, socket, &TcpMediumSocket::stopSlot);
    socket->setSecs2kickOff(mysett.sett.seckill);


    if(!socket->initObject(mysett.sett.whitelist, mysett.sett.seckill)){

        return;
    }





    QVariantHash oneconnection;
    oneconnection.insert("msec", QDateTime::currentMSecsSinceEpoch());
    mysett.activeconnections.insert(getLocalIpDescrPair(socket->mysett.remip, socket->mysett.descr), oneconnection);

    onReadWrite(socket->mysett.remip, socket->mysett.descr, 0, 0);


    connect(socket, &TcpMediumSocket::onReadData, this, &TheMediumTcpServer::onReadData);
    connect(this, &TheMediumTcpServer::onReadData, socket, &TcpMediumSocket::write2socket);

    connect(socket, &TcpMediumSocket::onReadWrite, this, &TheMediumTcpServer::onReadWrite);
    connect(socket, &TcpMediumSocket::onConnectionDown, this, &TheMediumTcpServer::onConnectionDown);

    if(mysett.activeconnections.size() > mysett.sett.maxconn){
        emit stopSocketSlot(socket->mysett.remip, socket->mysett.descr);//limit was reached
    }
}

//-------------------------------------------------------------------------------
