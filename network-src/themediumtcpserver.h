#ifndef THEMEDIUMTCPSERVER_H
#define THEMEDIUMTCPSERVER_H

#include <QTcpServer>
#include <QVariantHash>
#include <QStringList>

#include "tcpservermediumtypes.h"


class TheMediumTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TheMediumTcpServer(QObject *parent = nullptr);

    bool setServerSett(const TcpMediumServerSett &sett, const quint16 &port, const QVariantHash &hashhistory);

    bool startServer();

    struct MyServerSettState
    {
        quint16 port;
        TcpMediumServerSett sett;

        QVariantHash activeconnections;
        QStringList historyofconnections;
        QStringList log;


        quint8 sendHashCounter;
        bool isStopped;
        quint8 kickOffCounter;

        QStringList localConnectionIDs;
        QString activeLolacConnectionID;
        QString notifIfaceIsFree;
        QString portstr;

        int thelongestifacename;
        MyServerSettState() : port(0), sendHashCounter(0), isStopped(false), kickOffCounter(0), thelongestifacename(17) {}
    } mysett;

signals:
    //tcp mananger
    void append2history(quint16 port, QVariantHash hash);




    //to sockets
    void setSecs2kickOff(int secs);

    void stopAllSocketsSlot();

    void stopSocketSlot(QString ip, QString descr);

//    void write2socket(QByteArray writearr, bool isLocalConnection);

    void onReadData(QByteArray readarr, bool isLocalConnection);//it uses as a commutator

    void write2socketByID(QByteArray writearr, QString idstr);



    void startTmrClearCurrentID(int msec);


    void appendLogDataListByServersPort(QString portstr, QString lines);



    void showHexDump(QByteArray arr, QString ifaceName, bool isRead);


public slots:
    void onThreadStarted();

    //tcp manager
    void stopServerAndKickOff(quint16 port);

    void stopServerForced();

    void setNewSetts(int secs, int maxconn, QStringList allowlist);

    void setOneServerSett(quint16 port, TcpMediumServerSett sett);


    //socket
    void append2log(QString remip, QString descr, QString message);

    void onConnectionDown(QString remip, QString descr, QString message, qint64 msecdisconn);

    void onReadWrite(QString remip, QString descr, quint64 readbytes, quint64 writebytes);


    void restartServerLater();

    void stopSocketSlotServer(quint16 port, QString ip, QString descr);


    void onTmrClearCurrentID();
    void clearCurrentLocalID();

    void ifaceLogStr(QString line); //to GUI


    void onReadDataSlot(const QByteArray &readarr, const bool &isLocalConnection, const QString &idstr);//it uses as a commutator

    void onWriteDataSlot(const QByteArray &writearr, const QString &idstr);

private:
    QString getLogLine(const QString &line);

    QString getLocalIpDescrPair(const QString &ip, const QString &descr);
    QString getLocalIpDescrPairPretty(const QString &ip, const QString &descr);

    void append2logService(const QString &line);

    void showHexDumpF(const QByteArray &arr, const QString &ifaceName, const bool &isRead);

    bool killObjectLater();


    void sendInfoHash();

private slots:

    void sendInfoHashNow();

protected:
    void incomingConnection(qintptr socketDescr);


};

#endif // THEMEDIUMTCPSERVER_H
