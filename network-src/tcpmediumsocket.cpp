#include "tcpmediumsocket.h"

#include <QDateTime>

#include "network-types/tcpmediumtypeconverter.h"

///[!] type-converter
#include "src/shared/networkconverthelper.h"




//-----------------------------------------------------------------------------------

TcpMediumSocket::TcpMediumSocket(QObject *parent) : QTcpSocket(parent)
{

}

//-----------------------------------------------------------------------------------

bool TcpMediumSocket::initObject(const QStringList &whitelist, const int &secs)
{
    mysett.remip = NetworkConvertHelper::showNormalIP(peerAddress());
    mysett.descr = QString::number(socketDescriptor());

    mysett.isLocalIPv6 = (mysett.remip == "::1");//for internal usage

    mysett.isStopped = false;
    setSecs2kickOff(secs);

    emit append2SmplLogSlot(tr("New connection IP '%1', descriptor '%2'").arg(mysett.remip).arg(mysett.descr));

    if(!mysett.isLocalIPv6 && !whitelist.isEmpty()){
        //check white list

        if(!TcpMediumTypeConverter::isIpGood(mysett.remip, whitelist)){

            emit append2SmplLogSlot(tr("IP '%1' is not in the white list").arg(mysett.remip));
            QTimer::singleShot(1, this, SLOT(closeLater()));
            return false;
        }
    }

    QTimer *kickOffTimer = new QTimer(this);
    kickOffTimer->setInterval(mysett.msecs2kickOff);
    kickOffTimer->setSingleShot(true);

    connect(this, SIGNAL(startTmrKickOff(int)), kickOffTimer, SLOT(start(int)));
    connect(kickOffTimer, &QTimer::timeout, this, &TcpMediumSocket::onKickOffTmr);



    emit startTmrKickOff(mysett.msecs2kickOff);

    connect(this, &TcpMediumSocket::readyRead, this, &TcpMediumSocket::mReadyRead);
    connect(this, &TcpMediumSocket::disconnected, this, &TcpMediumSocket::onDisconnected);




    QTimer::singleShot(1, this, SLOT(checkConnectionFirstTime()));
    return true;
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::setSecs2kickOff(int secs)
{

    if(!mysett.isStopped){
        const quint32 msec = quint32(secs) * 1000;

        if(msec == mysett.msecs2kickOff)
            return;

        mysett.msecs2kickOff = msec;
        emit startTmrKickOff(mysett.msecs2kickOff);
    }else {
        closeAndKillLater();
    }

}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::stopAllSlot()
{
    try2kickOffTheConnection(tr("The connection will be terminated"));

    if(!mysett.isStopped)
        append2logSlot(tr("The connection will be terminated"));

    closeAndKillLater();
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::stopSlot(QString ip, QString descr)
{
    if(mysett.remip == ip && mysett.descr == descr){
         stopAllSlot();
    }
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::write2socket(QByteArray writearr, bool isFromLocalIPv6)
{
    if(mysett.isLocalIPv6 == isFromLocalIPv6)
        return;

    if(!isConnectionWorks()){
        onDisconnected();
        return;
    }

    write(writearr);
    mysett.bytesWrite += quint64(writearr.length());
    onReadWriteSlot();
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::mReadyRead()
{
    const QByteArray readarr = readAll();
    onReadDataSlot(readarr);
    onReadWriteSlot();
    emit startTmrKickOff(mysett.msecs2kickOff);
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::onDisconnected()
{
    try2kickOffTheConnection(tr("The connection was terminated"));
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::onKickOffTmr()
{
    try2kickOffTheConnection(tr("The connection was inactive"));
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::checkConnectionFirstTime()
{
    if(!isConnectionWorks())
        onDisconnected();
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::append2logSlot(QString message)
{
    emit append2log(mysett.remip, mysett.descr, message);

}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::onConnectionDownSlot(QString message)
{
    emit onConnectionDown(mysett.remip, mysett.descr, message, QDateTime::currentMSecsSinceEpoch());
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::onReadWriteSlot()
{
    emit onReadWrite(mysett.remip, mysett.descr, mysett.bytesRead, mysett.bytesWrite);
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::onReadDataSlot(QByteArray readarr)
{
    emit onReadData(readarr, mysett.isLocalIPv6);
    mysett.bytesRead += quint64(readarr.length());
}

void TcpMediumSocket::closeLater()
{
    closeAndKillLater();
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::try2kickOffTheConnection(const QString &message)
{
    if(!mysett.isStopped)
        append2logSlot(message);
    closeAndKillLater();
}

//-----------------------------------------------------------------------------------

bool TcpMediumSocket::closeAndKillLater()
{
    mysett.kickOffCounter++;
    if(!mysett.isStopped){
        mysett.isStopped = true;

        close();

        onConnectionDownSlot(tr("The connection was closed"));
        QTimer::singleShot(1, this, SLOT(deleteLater()));
        return true;
    }
    if(mysett.kickOffCounter > 10){
        onConnectionDownSlot(tr("Couldn't delete the object"));
        deleteLater();
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------

bool TcpMediumSocket::isConnectionWorks()
{
    return (state() == QTcpSocket::ConnectedState);
}

//-----------------------------------------------------------------------------------

void TcpMediumSocket::closeAndKillLaterForced()
{
    if(!closeAndKillLater())
        deleteLater();
}

//-----------------------------------------------------------------------------------
