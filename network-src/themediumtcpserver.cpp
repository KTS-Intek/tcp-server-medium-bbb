#include "themediumtcpserver.h"

#include <QTimer>
#include <QDateTime>

#include "network-types/tcpmediumtypeconverter.h"
#include "network-src/tcpmediumsocket.h"

///[!] type-converter
#include "src/shared/ifacehelper.h"


#include "ifaceconnectiondefs.h"



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
    mysett.portstr = QString::number(port);
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


    QTimer *tmrClearCurrentID = new QTimer(this);
    tmrClearCurrentID->setSingleShot(true);
    tmrClearCurrentID->setInterval(11111);

    connect(this, SIGNAL(startTmrClearCurrentID(int)), tmrClearCurrentID, SLOT(start(int)));
    connect(tmrClearCurrentID, SIGNAL(timeout()), this, SLOT(onTmrClearCurrentID()));


    IfaceHelper *ifceHlpr = new IfaceHelper(true, this);
    connect(ifceHlpr, SIGNAL(ifaceLogStr(QString)), this, SLOT(ifaceLogStr(QString)) );
    connect(this, SIGNAL(showHexDump(QByteArray,QString,bool)), ifceHlpr, SLOT(showHexDump(QByteArray,QString,bool)) );


}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::stopServerAndKickOff(quint16 port)
{
    if(port == mysett.port && killObjectLater()){
        append2logService(tr("The server was stopped on the port %1 ").arg(QString::number(mysett.port)));
    }
}
//-------------------------------------------------------------------------------
void TheMediumTcpServer::stopServerForced()
{

    killObjectLater();
    close();
    deleteLater();
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::setNewSetts(int secs, int maxconn, QStringList allowlist)
{
    mysett.sett.seckill = secs;
    mysett.sett.maxconn = maxconn;
    mysett.sett.allowlist = allowlist;

    emit setSecs2kickOff(secs);

}
//-------------------------------------------------------------------------------
void TheMediumTcpServer::setOneServerSett(quint16 port, TcpMediumServerSett sett)
{
    if(port == mysett.port){
        setNewSetts(sett.seckill, sett.maxconn, sett.allowlist);
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

    const QString socketidstr = getLocalIpDescrPair(remip, descr);

    if(mysett.localConnectionIDs.contains(socketidstr)){
        mysett.localConnectionIDs.removeOne(socketidstr);
    }

    if(mysett.notifIfaceIsFree == socketidstr)
        mysett.notifIfaceIsFree.clear();

    if(mysett.activeLolacConnectionID == socketidstr)
        clearCurrentLocalID();

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
    ifaceLogStr(tr("isDisabled %1").arg(int(mysett.isStopped)));
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

void TheMediumTcpServer::onTmrClearCurrentID()
{
    clearCurrentLocalID();
}
//-------------------------------------------------------------------------------
void TheMediumTcpServer::clearCurrentLocalID()
{

    mysett.activeLolacConnectionID.clear();

    if(!mysett.notifIfaceIsFree.isEmpty()){
        emit write2socketByID(QByteArray(PORT_IS_FREE), mysett.notifIfaceIsFree);
        mysett.notifIfaceIsFree.clear();
    }
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::ifaceLogStr(QString line)
{
    emit appendLogDataListByServersPort(mysett.portstr, line);
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::onReadDataSlot(const QByteArray &readarr, const bool &isLocalConnection, const QString &idstr)
{

    showHexDumpF(readarr, idstr, true);

    if(isLocalConnection){
        if(mysett.activeLolacConnectionID != idstr){
            if(!mysett.activeLolacConnectionID.isEmpty()){
                if(mysett.notifIfaceIsFree.isEmpty())
                    mysett.notifIfaceIsFree = idstr;

                //write that iface is busy
                emit write2socketByID(QByteArray(PORT_IS_BUSY), idstr);
                return;
            }
            mysett.activeLolacConnectionID = idstr;
        }
        //start tmr clear connection id

        emit startTmrClearCurrentID(15000);
    }
    emit onReadData(readarr, isLocalConnection);
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::onWriteDataSlot(const QByteArray &writearr, const QString &idstr)
{
    showHexDumpF(writearr, idstr, false);
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

QString TheMediumTcpServer::getLocalIpDescrPairPretty(const QString &ip, const QString &descr)
{
    const QString s = QString("%1_%2").arg(ip).arg(descr.rightJustified(2, '0'));

    if(s.length() > mysett.thelongestifacename)
        mysett.thelongestifacename = qMin(30, s.length());
    return s;
}

//-------------------------------------------------------------------------------

void TheMediumTcpServer::append2logService(const QString &line)
{
    mysett.log.append(getLogLine(line));

    sendInfoHash();
}

void TheMediumTcpServer::showHexDumpF(const QByteArray &arr, const QString &ifaceName, const bool &isRead)
{
    const int len = ifaceName.length();
    if(len > mysett.thelongestifacename){
        mysett.thelongestifacename = qMin(30, len);
    }
    emit showHexDump(arr, ifaceName.rightJustified(mysett.thelongestifacename, ' '), isRead);
}

//-------------------------------------------------------------------------------

bool TheMediumTcpServer::killObjectLater()
{
    mysett.kickOffCounter++;
    emit stopAllSocketsSlot();
    if(!mysett.isStopped){
        ifaceLogStr(tr("stopping"));
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


    if(!socket->initObject(mysett.sett.allowlist, mysett.sett.seckill)){

        return;
    }

    const QString socketidstr = getLocalIpDescrPair(socket->mysett.remip, socket->mysett.descr);

    if(socket->mysett.isLocalConnection ){

        if(!mysett.localConnectionIDs.contains(socketidstr))
            mysett.localConnectionIDs.append(socketidstr);
        connect(this, &TheMediumTcpServer::write2socketByID, socket, &TcpMediumSocket::write2socketByID);
    }



    socket->setIdStr(socketidstr, getLocalIpDescrPairPretty(socket->mysett.remip, socket->mysett.descr));
    QVariantHash oneconnection;
    oneconnection.insert("msec", QDateTime::currentMSecsSinceEpoch());
    mysett.activeconnections.insert(socketidstr, oneconnection);

    onReadWrite(socket->mysett.remip, socket->mysett.descr, 0, 0);


    connect(socket, &TcpMediumSocket::onReadData, this, &TheMediumTcpServer::onReadDataSlot);
    connect(this, &TheMediumTcpServer::onReadData, socket, &TcpMediumSocket::write2socket);
    connect(socket, &TcpMediumSocket::onWriteData, this, &TheMediumTcpServer::onWriteDataSlot);

    connect(socket, &TcpMediumSocket::onReadWrite, this, &TheMediumTcpServer::onReadWrite);
    connect(socket, &TcpMediumSocket::onConnectionDown, this, &TheMediumTcpServer::onConnectionDown);

    if(mysett.activeconnections.size() > mysett.sett.maxconn){
        emit stopSocketSlot(socket->mysett.remip, socket->mysett.descr);//limit was reached
    }
}

//-------------------------------------------------------------------------------
