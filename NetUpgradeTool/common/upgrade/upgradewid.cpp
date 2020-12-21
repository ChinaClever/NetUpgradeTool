#include "upgradewid.h"
#include "ui_upgradewid.h"
#include "msgbox.h"

UpgradeWid::UpgradeWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpgradeWid)
{
    ui->setupUi(this);

    mData = DataPacket::bulid()->data;
    mExportDlg = new ExportDlg(this);
    mTcpThread = new TcpUpgrade(this);
    mTftpThread = new TftpUpgrade(this);
    //mHttpThread = new HttpUpgrade(this);

    timer = new QTimer(this);
    timer->start(500);
    connect(timer, SIGNAL(timeout()),this, SLOT(timeoutDone()));

//    ui->label->setText("file");
//    ui->openBtn->setText("open");
//    ui->label_4->setText("sub progress");
//    ui->breakBtn->setText("break");
//    ui->label_2->setText("progress");
//    ui->updateBtn->setText("upgrade");
//    ui->label_3->setText("status");
//    ui->exportBtn->setText("export result");
//    ui->stateLab->setText("please start");
}

UpgradeWid::~UpgradeWid()
{
    delete ui;
}



QByteArray UpgradeWid::readFile(const QString &fn)
{
    QFile file(fn);
    QByteArray array;

    bool ret = file.exists();
    ret = file.open(QIODevice::ReadOnly);
    if(ret) {
        array = file.readAll();
        file.close();
    } else {
        qDebug() << "open file err";
    }

    return array;
}

ushort UpgradeWid::calccrc (ushort crc, uchar crcbuf)
{
    uchar x, kkk=0;
    crc = crc^crcbuf;
    for(x=0;x<8;x++)
    {
        kkk = crc&1;
        crc >>= 1;
        crc &= 0x7FFF;
        if(kkk == 1)
            crc = crc^0xa001;
        crc=crc&0xffff;
    }
    return crc;
}

/**
  * 功　能：CRC校验
  * 入口参数：buf -> 缓冲区  len -> 长度
  * 返回值：CRC
  */
QByteArray UpgradeWid::rtu_crc(QByteArray &array)
{
    ushort crc = 0xffff;
    for(int i=0; i<array.size(); i++)
        crc = calccrc(crc, array.at(i));

    QByteArray res;
    res.append(crc & 0xFF);
    res.append(crc >> 8);

    return res;
}

QByteArray UpgradeWid::appendCrc(QByteArray &array)
{
    QByteArray crcs;
    for(int i=0; i<array.size(); ) {
        QByteArray temp;
        for(int k=0; (k<1024) && (i<array.size()); k++) {
             temp.append(array.at(i++));
        }
        crcs.append(rtu_crc(temp));
    }

    return rtu_crc(crcs);
}


bool UpgradeWid::checkFileCrc(const QString &fn)
{
    bool ret = false;
    QByteArray crcs;
    QByteArray array = readFile(fn);
    int len = array.size();
    if(len > 2) {
        crcs.append(array.at(len-2));
        crcs.append(array.at(len-1));
        array.remove(len-2, 2);

        QByteArray res = appendCrc(array);
        if(res == crcs) ret = true;
    }

    return ret;
}


bool UpgradeWid::checkFile()
{
    bool ret = false;
    QString fn = ui->lineEdit->text();
    if(!fn.isEmpty()) {
        if(fn.contains("bin") || fn.contains("tar")||fn.contains("clever") ) {
            if(mData->devtype) {
                ret = checkFileCrc(fn);
            }
            else
                ret = true;
            if(ret) {
                mData->file = fn;
            } else {
                CriticalMsgBox box(this, tr("升级文件内容验证错误! 请重新选择"));
                //CriticalMsgBox box(this, tr("upgrade file content error!, please re-select"));
            }
        } else {
            CriticalMsgBox box(this, tr("升级文件格式错误! 请重新选择"));
            //CriticalMsgBox box(this, tr("upgrade file format error!, please re-select"));
        }
    } else {
        CriticalMsgBox box(this, tr("请选择升级文件"));
        //CriticalMsgBox box(this, tr("please select the upgrade file "));
    }

    return ret;
}

void UpgradeWid::on_openBtn_clicked()
{
    static QString fn;
    fn = QFileDialog::getOpenFileName(0,tr("文件选择"),fn,"",0);
    //fn = QFileDialog::getOpenFileName(0,tr("file selection"),fn,"",0);
    if (!fn.isNull()) {
        ui->lineEdit->setText(fn);
        checkFile();
    }
}


void UpgradeWid::timeoutDone(void)
{
    QString str = mData->status;
    if(str.isEmpty()) str = tr("请开始");
        //str = tr("please start");
    ui->stateLab->setText(str);

    if(mData->subPorgress>100) mData->subPorgress = 0;
    ui->progressBar_2->setValue(mData->subPorgress);

    bool en = mData->isRun;
    if(mData->ips.isEmpty()) en = true;
    ui->openBtn->setDisabled(en);
    ui->updateBtn->setDisabled(en);
    ui->exportBtn->setDisabled(en);
    ui->breakBtn->setEnabled(en);

    int x = 0;
    int count = mData->ips.size();
    if(count) x = ((mData->progress * 1.0) / count) *100;
    ui->progressBar->setValue(x);
}


void UpgradeWid::on_updateBtn_clicked()
{
    if(mData->devtype) {
        mUpgradeThread = mTcpThread;
    } else {
        mUpgradeThread = mTftpThread;
    }

    if(checkFile()) {
        mUpgradeThread->startSend();
    }
}

void UpgradeWid::on_exportBtn_clicked()
{
    mExportDlg->exec();
}

void UpgradeWid::on_breakBtn_clicked()
{
    if(mData->isRun) {
        QuMsgBox box(this, tr("是否要中断传输?"));
        //QuMsgBox box(this, tr("Do you want to interrupt transmission?"));
        if(box.Exec()) {
            mUpgradeThread->breakDown();
            if(0 == mData->devtype) {
                InfoMsgBox msg(this, tr("软件即将重启!!!"));
                //InfoMsgBox msg(this, tr("Software will reboot!!!"));
                QProcess *process = new QProcess(this);
                process->start("NetUpgradeTool.exe");
                exit(0);
            }
        }
    }
}

void UpgradeWid::languageChanged()
{
    ui->retranslateUi(this);
}
