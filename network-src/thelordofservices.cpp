#include "thelordofservices.h"

#include <QTimer>
#include <QThread>


#include "network-types/tcpmediumtypeconverter.h"
#include "network-src/themediumtcpserver.h"

///[!] sharedmemory
#include "src/shared/sharedmemowriter.h"

#include "matildalimits.h"
#include "ifaceexchangetypesdefs.h"

//------------------------------------------------------------------------------------------

TheLordOfServices::TheLordOfServices(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------------------

void TheLordOfServices::onThreadStarted()
{
    if(!QMetaType::isRegistered(QMetaType::type("TcpMediumServerSett")))
        qRegisterMetaType<TcpMediumServerSett>("TcpMediumServerSett");

    SharedMemoWriter *writer = new SharedMemoWriter(mysett.shmemnane, mysett.shmemsema, mysett.filename, 1111, 60000, mysett.verboseMode);
    connect(this, SIGNAL(setSharedMemData(QString,QVariant)), writer, SLOT(setSharedMemData(QString, QVariant)) );
    connect(this, &TheLordOfServices::flushAllData, writer, &SharedMemoWriter::flushAllNow);

    writer->initObject(false);

    const QVariantHash hashStatus = writer->getLastSavedObj();

    for(int i = 0, imax = MAX_TCP_MEDIUM_SERVER_INSTANCES, p = MIN_TCP_MEDIUM_SERVER_PORT; i < imax; i++, p++){
        if(!hashStatus.value(QString::number(p)).toHash().isEmpty())
            mysett.map2serverHistory.insert(p, hashStatus.value(QString::number(p)).toHash());
    }

    SharedMemoWriter *writerlogs = new SharedMemoWriter(mysett.shmemnanelogs, mysett.shmemsemalogs, "", 2222, 60000, mysett.verboseMode);
    connect(this, &TheLordOfServices::appendLogDataListByServers, writerlogs, &SharedMemoWriter::appendLogDataList);
    connect(this, &TheLordOfServices::flushAllData, writerlogs, &SharedMemoWriter::flushAllNow);

    writerlogs->initObject(true);

    QTimer::singleShot(1111, this, SIGNAL(onReady2receiveData()));
}

//------------------------------------------------------------------------------------------

void TheLordOfServices::append2history(quint16 port, QVariantHash hash)
{
    mysett.map2serverHistory.insert(port, hash);
    emit setSharedMemData(QString::number(port), QVariant::fromValue(hash));
}

//------------------------------------------------------------------------------------------

void TheLordOfServices::setTheseSettings(QVariantHash hashsett, QVariantHash defsett)
{
    for(int i = 0, imax = MAX_TCP_MEDIUM_SERVER_INSTANCES, p = MIN_TCP_MEDIUM_SERVER_PORT; i < imax; i++, p++){
        mysett.mapsettings.insert(p, TcpMediumTypeConverter::convertFromVarHash(hashsett.value(QString::number(p)).toHash(), defsett));
    }
    checkActiveServers();
}

//------------------------------------------------------------------------------------------

void TheLordOfServices::checkActiveServers()
{
    for(int i = 0, imax = MAX_TCP_MEDIUM_SERVER_INSTANCES, p = MIN_TCP_MEDIUM_SERVER_PORT; i < imax; i++, p++){
        const TcpMediumServerSett sett = mysett.mapsettings.value(p);

        if(mysett.mapstate.value(p) == sett.enable){ //isAllowed == isListenning
            if(sett.enable)
                emit setOneServerSett(p, sett);
            continue;
        }
        if(mysett.mapstate.value(p)){//if isListenning , kill
            emit stopServerAndKickOff(p);
            mysett.mapstate.insert(p, false);
            continue;
        }

        mysett.mapstate.insert(p, true);


        //if !isListennig, start
        TheMediumTcpServer *server = new TheMediumTcpServer;
        QThread *thread = new QThread;

        thread->setObjectName(QString("TMTS%1").arg(p));

        server->moveToThread(thread);
        connect(this, SIGNAL(killAllThreads()), server, SLOT(stopServerForced()));

        connect(server, SIGNAL(destroyed(QObject*)), thread, SLOT(quit()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(thread, SIGNAL(started()), server, SLOT(onThreadStarted()));


        server->setServerSett(sett, p, mysett.map2serverHistory.value(p));

        connect(this, &TheLordOfServices::setOneServerSett, server, &TheMediumTcpServer::setOneServerSett);
        connect(this, &TheLordOfServices::stopSocketSlotServer, server, &TheMediumTcpServer::stopSocketSlotServer);
        connect(this, &TheLordOfServices::stopServerAndKickOff, server, &TheMediumTcpServer::stopServerAndKickOff);

        connect(server, &TheMediumTcpServer::append2history, this, &TheLordOfServices::append2history);

        connect(server, &TheMediumTcpServer::appendLogDataListByServersPort, this, &TheLordOfServices::appendLogDataListByServersPort);
        thread->start();

    }

}

//------------------------------------------------------------------------------------------

void TheLordOfServices::appendLogDataListByServersPort(QString portstr, QString lines)
{
    emit appendLogDataListByServers(portstr, lines.split("\n"), "\n", 300);
}

//------------------------------------------------------------------------------------------
