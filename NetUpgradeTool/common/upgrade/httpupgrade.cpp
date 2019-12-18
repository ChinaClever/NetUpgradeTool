#include "httpupgrade.h"
extern void udp_sent_data(const QString &ip, uchar *buf, ushort len);

HttpUpgrade::HttpUpgrade(QObject *parent) : UpgradeThread(parent)
{
    quint16 port = 8080;
    mDaemon = new HttpDaemon(port, this);
    connect(mDaemon, SIGNAL(progressSig(float,QString)), this, SLOT(subProgressSlot(float,QString)));
}

HttpUpgrade::~HttpUpgrade()
{
    breakSent();
    wait();
}

bool HttpUpgrade::upload(const QString &file, const QString &ip)
{
    char *msg = "http upgrade";
    udp_sent_data(ip, (uchar *)msg, strlen(msg));
    mDaemon->shareFile(file);
    return true;
}

void HttpUpgrade::breakSent()
{
    mDaemon->breakDown();
}
