#ifndef TFTPWIDGET_H
#define TFTPWIDGET_H

#include "exports/exportdlg.h"
#include "tcpupgrade.h"
#include "httpupgrade.h"
#include "cryptopp/cryptopputil.h"
namespace Ui {
class UpgradeWid;
}

class UpgradeWid : public QWidget
{
    Q_OBJECT

public:
    explicit UpgradeWid(QWidget *parent = 0);
    ~UpgradeWid();

protected:
    bool checkFile();
    QByteArray readFile(const QString &fn);
    ushort calccrc (ushort crc, uchar crcbuf);
    QByteArray rtu_crc(QByteArray &array);
    QByteArray appendCrc(QByteArray &array);
    bool checkFileCrc(const QString &fn);

    void CRC32_Init();
    void CRC32_Update(unsigned char *data, size_t len);
    QByteArray CRC32_Final();
    void MyMd5(unsigned char *InBuf,char *OutBuf , int len);
    bool checkFlagAndVer(QByteArray &array, int &index);
    bool rsaVerifier(QByteArray &md5, QByteArray &res);
    bool checkStr(QByteArray array , int len);

private slots:
    void timeoutDone();
    void on_openBtn_clicked();
    void on_updateBtn_clicked();
    void on_exportBtn_clicked();
    void on_breakBtn_clicked();

public slots:
    void languageChanged();

private:
    Ui::UpgradeWid *ui;
    sDataPacket *mData;

    unsigned long mCrc;

    QTimer *timer;
    ExportDlg *mExportDlg;
    TcpUpgrade *mTcpThread;
    TftpUpgrade *mTftpThread;
    HttpUpgrade *mHttpThread;
    UpgradeThread *mUpgradeThread;
};

#endif // TFTPWIDGET_H
