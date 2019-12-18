#ifndef HTTPUPGRADE_H
#define HTTPUPGRADE_H

#include "http/httpdaemon.h"
#include "upgradethread.h"

class HttpUpgrade : public UpgradeThread
{
    Q_OBJECT
public:
    explicit HttpUpgrade(QObject *parent = nullptr);
    ~HttpUpgrade();

protected:
    bool upload(const QString &file, const QString &ip);
    void breakSent();

private:
    HttpDaemon *mDaemon;
};

#endif // HTTPUPGRADE_H
