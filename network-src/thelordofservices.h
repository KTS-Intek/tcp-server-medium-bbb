#ifndef THELORDOFSERVICES_H
#define THELORDOFSERVICES_H

#include <QObject>
#include <QVariantHash>
#include <QVariantMap>



#include "tcpservermediumtypes.h"

class TheLordOfServices : public QObject
{
    Q_OBJECT
public:
    explicit TheLordOfServices(QObject *parent = nullptr);



    struct MySettsOfServices
    {
        QString shmemnane;
        QString shmemsema;
        QString filename;
        bool verboseMode;

        QString shmemnanelogs;
        QString shmemsemalogs;

        QMap<quint16, bool> mapstate;//port - isListenning
        QMap<quint16, QVariantHash> map2serverHistory;//port - history

        QMap<quint16, TcpMediumServerSett> mapsettings;//port - last settings
        MySettsOfServices() : verboseMode(false) {}
    } mysett;


signals:
    void setSharedMemData(QString key, QVariant data);
    void flushAllData();
    void killAllThreads();

    void onReady2receiveData();

//to server
    void stopSocketSlotServer(quint16 port, QString ip, QString descr);

    void stopServerAndKickOff(quint16 port);


    void setOneServerSett(quint16 port, TcpMediumServerSett sett);


    void appendLogDataListByServers(QString key, QStringList log, QString splitter, int maxLogSize);

public slots:
    void onThreadStarted();


    void append2history(quint16 port, QVariantHash hash);

    void setTheseSettings(QVariantHash hashsett, QVariantHash defsett);

    void checkActiveServers();

    void appendLogDataListByServersPort(QString portstr, QString lines);


};

#endif // THELORDOFSERVICES_H
