#ifndef DATAPACKET_H
#define DATAPACKET_H
#include <QtCore>

/**
 * 数据包
 */
#define LANGUAGE 0 //1代表英文，0代表中文

struct sDataPacket
{
    bool isRun;
    int devtype;
    int progress, subPorgress;
    QString usr,pwd;

    QString file;
    QString status, subStatus;

    QStringList ips;
    QStringList errs;
    QStringList oks;
    QList<QStringList> logs;
};


class DataPacket
{
    DataPacket();
public:
    static DataPacket *bulid();

    sDataPacket *data;
};

#endif // DATAPACKET_H
