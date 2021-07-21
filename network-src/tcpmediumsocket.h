#ifndef TCPMEDIUMSOCKET_H
#define TCPMEDIUMSOCKET_H

#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>


class TcpMediumSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpMediumSocket(QObject *parent = nullptr);

    struct MySocketSettState
    {
        QString descr;
        QString remip;

        quint64 bytesRead;
        quint64 bytesWrite;


        quint32 msecs2kickOff;


        bool isLocalConnection;
        bool isStopped;
        quint8 kickOffCounter;

        QString idstr;
        QString prettyidstr;
        MySocketSettState() : bytesRead(0), bytesWrite(0), msecs2kickOff(3600), isLocalConnection(false), isStopped(false), kickOffCounter(0) {}
    } mysett;

    bool initObject(const QStringList &allowlist, const int &secs);

    void setIdStr(const QString &idstr, const QString &prettyidstr);

signals:
    //internal
    void startTmrKickOff(int msec);

    //external
    void append2SmplLogSlot(QString message);

    void append2log(QString remip, QString descr, QString message);

    void onConnectionDown(QString remip, QString descr, QString message, qint64 msecdisconn);

    void onReadWrite(QString remip, QString descr, quint64 readbytes, quint64 writebytes);


    void onReadData(QByteArray readarr, bool isLocalConnection, QString idstr);

    void onWriteData(QByteArray writearr, QString idstr);



public slots:

    void setSecs2kickOff(int secs);

    void stopAllSlot();

    void stopSlot(QString ip, QString descr);

    void write2socket(QByteArray writearr, bool isLocalConnection);

    void write2socketByID(QByteArray writearr, QString idstr);


private slots:
    void mReadyRead();

    void onDisconnected();

    void onKickOffTmr();


    void checkConnectionFirstTime();



    void append2logSlot(QString message);

    void onConnectionDownSlot(QString message);

    void onReadWriteSlot();


    void onReadDataSlot(QByteArray readarr);

    void closeLater();
private:
    void try2kickOffTheConnection(const QString &message);

    bool closeAndKillLater();

    bool isConnectionWorking();

    void closeAndKillLaterForced();


};

#endif // TCPMEDIUMSOCKET_H
