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

bool TcpMediumSocket::initObject(const QStringList &allowlist, const int &secs)
{
    mysett.remip = NetworkConvertHelper::showNormalIP(peerAddress());
    mysett.descr = QString::number(socketDescriptor());

    mysett.isLocalConnection = (mysett.remip == "::1" || mysett.remip == "127.0.0.1");//for internal usage

    mysett.isStopped = false;
    setSecs2kickOff(secs);

    emit append2SmplLogSlot(tr("New connection IP '%1', descriptor '%2'").arg(mysett.remip).arg(mysett.descr));

    if(!mysett.isLocalConnection && !allowlist.isEmpty()){
        //check white list

        if(!NetworkConvertHelper::isIpGood(mysett.remip, allowlist)){

            emit append2SmplLogSlot(tr("IP '%1' is not in the allowed list").arg(mysett.remip));
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

void TcpMediumSocket::setIdStr(const QString &idstr, const QString &prettyidstr)
{
    mysett.idstr = idstr;
    mysett.prettyidstr = prettyidstr;
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

void TcpMediumSocket::write2socket(QByteArray writearr, bool isLocalConnection)
{
    if(mysett.isLocalConnection == isLocalConnection)
        return;

    if(!isConnectionWorking()){
        onDisconnected();
        return;
    }

    write(writearr);
    emit onWriteData(writearr, mysett.prettyidstr);
    mysett.bytesWrite += quint64(writearr.length());
    onReadWriteSlot();
}
//-----------------------------------------------------------------------------------
void TcpMediumSocket::write2socketByID(QByteArray writearr, QString idstr)
{
//    append2logSlot(QString("write2socketByID %1, %2, %3").arg(QString(writearr)).arg(idstr).arg(mysett.idstr));
    if(idstr == mysett.idstr){

        write2socket(writearr, !mysett.isLocalConnection);
    }
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
    if(!isConnectionWorking())
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
    emit onReadData(readarr, mysett.isLocalConnection, mysett.prettyidstr);
    mysett.bytesRead += quint64(readarr.length());
}

//-----------------------------------------------------------------------------------

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

bool TcpMediumSocket::isConnectionWorking()
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
