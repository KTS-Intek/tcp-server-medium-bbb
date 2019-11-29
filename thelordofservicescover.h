#ifndef THELORDOFSERVICESCOVER_H
#define THELORDOFSERVICESCOVER_H

#include "network-src/thelordofservices.h"

class TheLordOfServicesCover : public TheLordOfServices
{
    Q_OBJECT
public:
    explicit TheLordOfServicesCover(QObject *parent = nullptr);

    void setupYourSelf();

    void setSettings(const QString &shmemnane, const QString &shmemsema, const QString &filename, const bool &verboseMode, const QString logsmemo, const QString &logssema);


signals:

public slots:
    void reloadSettingsFromTheMemory();

    void initObjects();


    void onConfigChanged(quint16 command, QVariant dataVar);


};

#endif // THELORDOFSERVICESCOVER_H
