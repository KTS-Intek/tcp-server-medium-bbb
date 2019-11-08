#include "tcpservermediumlocalsocket.h"


#include "moji_defy.h"
#include "dbgaboutsourcetype.h"

TcpServerMediumLocalSocket::TcpServerMediumLocalSocket(bool verboseMode, QObject *parent) : RegularLocalSocket(verboseMode, parent)
{

}

void TcpServerMediumLocalSocket::decodeReadData(const QVariant &dataVar, const quint16 &command)
{
//       switch(command){
//       case MTD_EXT_CUSTOM_COMMAND_0: {
//           if(verboseMode) qDebug() << "ext " << mtdExtName << dataVar;

//           if(activeDbgMessages)
//               emit appendDbgExtData(DBGEXT_THELOCALSOCKET, QString("command r: %1, data=%2").arg(command).arg(dataVar.toHash().value("d").toString()));
//           emit command4dev(dataVar.toHash().value("c").toUInt(), dataVar.toHash().value("d").toString());
//           break;}



//       default: {
//           if(verboseMode)
//               qDebug() << "default ext " << command << mtdExtName << dataVar;
//           emit onConfigChanged(command,dataVar);
//           break;}
//       }
       emit onConfigChanged(command,dataVar);

}
