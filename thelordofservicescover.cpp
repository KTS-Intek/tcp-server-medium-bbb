#include "thelordofservicescover.h"


#include "tcpservermediumlocalsocket.h"


///[!] matilda-bbb-settings
#include "src/matilda/settloader4matilda.h"
#include "src/matilda/settloader4matildadefaults.h"


///[!] sharedmemory
#include "src/shared/sharedmemohelper.h"


#include <QThread>

#include "moji_defy.h"

TheLordOfServicesCover::TheLordOfServicesCover(QObject *parent) : TheLordOfServices(parent)
{

}

void TheLordOfServicesCover::setupYourSelf()
{
    setSettings(SharedMemoHelper::defTcpMediumServerMemoName(), SharedMemoHelper::defTcpMediumServerSemaName()
                , "", qApp->arguments().contains("-vv")
                , SharedMemoHelper::defTcpMediumServerLogsMemoName(), SharedMemoHelper::defTcpMediumServerLogsSemaName());

    QThread *t = new QThread(this);
    this->moveToThread(t);

    connect(this, SIGNAL(destroyed(QObject*)), t, SLOT(quit()));
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));

    connect(t, SIGNAL(started()), this, SLOT(initObjects()));
    t->start();

}

void TheLordOfServicesCover::setSettings(const QString &shmemnane, const QString &shmemsema, const QString &filename, const bool &verboseMode, const QString logsmemo, const QString &logssema)
{
    mysett.shmemnane = shmemnane;
    mysett.shmemsema = shmemsema;
    mysett.filename = filename;
    mysett.verboseMode = verboseMode;
    mysett.shmemnanelogs = logsmemo;
    mysett.shmemsemalogs = logssema;
}

void TheLordOfServicesCover::reloadSettingsFromTheMemory()
{
    setTheseSettings(SettLoader4matilda().loadOneSett(SETT_TCP_MEDIUM_SERVER).toHash(), SettLoader4matildaDefaults::getDefMediumServerSett());
}

void TheLordOfServicesCover::initObjects()
{
    TcpServerMediumLocalSocket *extSocket = new TcpServerMediumLocalSocket(mysett.verboseMode);
    extSocket->activeDbgMessages = false;

    extSocket->initializeSocket(MTD_EXT_NAME_TCP_MEDIUM_SERVER);
    QThread *extSocketThrd = new QThread(this);
    extSocket->moveToThread(extSocketThrd);

    connect(extSocketThrd, &QThread::started, extSocket, &TcpServerMediumLocalSocket::onThreadStarted);
    connect(extSocket, &TcpServerMediumLocalSocket::onConfigChanged , this, &TheLordOfServicesCover::onConfigChanged  );

    extSocketThrd->start();

    connect(this, &TheLordOfServicesCover::onReady2receiveData, this, &TheLordOfServicesCover::reloadSettingsFromTheMemory);

    onThreadStarted();

}

void TheLordOfServicesCover::onConfigChanged(quint16 command, QVariant dataVar)
{
      switch(command){
      case MTD_EXT_COMMAND_RESTART_APP:  {
          emit flushAllData();
          setTheseSettings(QVariantHash(), SettLoader4matildaDefaults::getDefMediumServerSett()); //kill all services
          QThread::sleep(3);
          qApp->quit();
          return;}
      case MTD_EXT_COMMAND_RELOAD_SETT: {//reaload all settings
          reloadSettingsFromTheMemory();
          break;}

      case MTD_EXT_CUSTOM_COMMAND_3:{
          const QVariantHash h = dataVar.toHash();
          const QList<QString> lk = h.keys();
          for(int i = 0, imax = lk.size(); i < imax; i++){

              const quint16 port = lk.at(i).toUInt();
              if(port > 1){
                  const QStringList l = h.value(lk.at(i)).toStringList();//<ip>\t<port>
                  for(int j = 0, jmax = l.size(); j < jmax; j++){
                      const QStringList oneconn = l.at(j).split("\t");
                      emit stopSocketSlotServer(port, oneconn.first(), oneconn.last());
                  }

              }
          }

          break;//kill sockets by ip descr
      }

      default: qDebug() << qAppName() << " unknown command " << command; qDebug() << dataVar; break;

      }
}
