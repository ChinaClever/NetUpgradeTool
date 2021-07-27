#include "upgradewid.h"
#include "ui_upgradewid.h"
#include "msgbox.h"
#include "myMd5.h"


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

#if LANGUAGE==1
    ui->label->setText("File");
    ui->openBtn->setText("Open");
    ui->label_4->setText("Sub progress");
    ui->breakBtn->setText("Break");
    ui->label_2->setText("Progress");
    ui->updateBtn->setText("Upgrade");
    ui->label_3->setText("Status");
    ui->exportBtn->setText("Export result");
    ui->stateLab->setText("Please start");
#endif
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
    //QByteArray crcs;
    QByteArray md5Str;
    for(int i=0; i<array.size(); ) {
        QByteArray temp;
        int k=0;
        for( ;(k<1024) && (i<array.size()); k++) {
            temp.append(array.at(i++));
        }

//        char str[40];
//        MyMd5((unsigned char*)(temp+md5Str).data(),str , temp.size()+md5Str.size());
//        md5Str.clear();
//        for(size_t j = 0 ; j < 32 ; j++)
//            md5Str.append(str[j]);
        QByteArray test;
        test.append(temp+md5Str);
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(test); //将btArray作为参数加密
        md5Str=hash.result();
        qDebug()<<md5Str.size()<<" sha256 "<<md5Str<<"   string "<<md5Str.toHex()<<endl;
        //CRC32_Update((unsigned char*)temp.data(),k);
        //crcs.append(rtu_crc(temp));
    }

    //QByteArray ret = CRC32_Final();
    char FixedBuf[11]="@PLD?FDFQ5";
    char strtemp[40];
    char str1[100];
//    strncpy(str1,md5Str.data(),32);
    for(int i = 0 ; i < 32 ; i++)
    {
        str1[i] = md5Str.data()[i];
    }
    strncpy(&str1[32],FixedBuf,11);
//    MyMd5((unsigned char*)str1,strtemp , 42);
//    md5Str.clear();
//    for(size_t j = 0 ; j < 32 ; j++)
//        md5Str.append(strtemp[j]);
//    QByteArray testlast;
//    testlast.append(str1);
//    qDebug()<<testlast.size()<<" testlast "<<testlast<<"  testlast string "<<testlast.toHex()<<endl;
    QCryptographicHash hashlast(QCryptographicHash::Sha256);
    hashlast.addData(str1 , 42); //将btArray作为参数加密
    md5Str=hashlast.result();

    qDebug()<<md5Str.size()<<" last sha256 "<<md5Str<<"   string "<<md5Str.toHex()<<endl;
    qDebug()<<"Last md5Str" << md5Str<<endl;
    return md5Str.toHex();
}


/**
 * @brief MyMd5
 * @param InBuf,OutBuf
 * @return void
 */
void UpgradeWid::MyMd5(unsigned char *InBuf,char *OutBuf , int len)
{
    char i;
    unsigned char decrypt[16]={0x00};
    MD5_CTX md5;
    char ch[2]={0,0};

    MD5Init(&md5);
    MD5Update(&md5,InBuf,len);
    MD5Final(&md5,decrypt);

    for(i=0;i<16;i++)
    {
        sprintf(ch,"%02x",decrypt[i]);
        OutBuf[2*i] = ch[0];
        OutBuf[2*i+1] = ch[1];
    }
}

bool UpgradeWid::checkFileCrc(const QString &fn)
{
    bool ret = false;
    QByteArray crcs;
    QByteArray array = readFile(fn);
    int len = array.size();
    int newlen = len;
    //CRC32_Init();
    if(checkFlagAndVer(array, newlen)){
        if(newlen > 512) {
            for( int i = 512 ; i > 0 ; i--)
                crcs.append(array.at(newlen-i));
            array.remove(newlen - 512 ,512);
            QByteArray res = crcs;
            QByteArray md5 = appendCrc(array);
//            if(md5.size() == 32 && res.size() == 512 && checkStr(md5,32) && checkStr(res,512))
            if(md5.size() == 64 && res.size() == 512 &&  checkStr(res,512))
                ret = rsaVerifier(md5,res);
        }
    }
    //ret = true;
    return ret;
}

bool UpgradeWid::checkStr(QByteArray array , int len)
{
    bool ret = true;
    for(int i = 0 ; i < len ; i++)
    {
        if((array.at(i)>='0' && array.at(i)<='9') || (array.at(i)>='a' && array.at(i)<='f'))
            continue;
        else{
            ret = false;
            break;
        }
    }
    return ret;
}

bool UpgradeWid::checkFlagAndVer(QByteArray &array , int& index)
{
    bool ret = false;
    bool flag = false;
    int len = array.size();
    QByteArray byte;
    int count = 0;
    int chipVer = 0;
    bool ok;
    if( array.at(len - 1) != '&' )
    {
        chipVer = array.right(1).toInt(&ok,16);
        array.remove(len - 1, 1);
        len = len - 1;
    }else{
        chipVer = 0;
    }
    if(len > 50) {
        int pre = len-1;
        for( int i = len-1 ; i > len-50 ; i--){
            if( array.at(i) == '&' ){
                count++;
                if(pre == len-1)
                    pre = i;
                else{
                    if( pre - i ==0)//避免&之间没有内容
                        break;
                    if(count == 2 && pre - i < 1)//避免后两个&之间内容长度必须大于1
                        break;
                    if(count == 3 && pre - i < 2)//避免前两个&之间内容固定长度为2
                        break;
                    pre = i;
                }

                if(count == 3){
                    flag = true;
                    index = i;
                    break;
                }
            }
        }
        if(flag == false) return ret;

        array.remove(index, len-index);
        ret = true;
    }
    return ret;
}

bool UpgradeWid::rsaVerifier(QByteArray& md5 , QByteArray &res)
{
    using namespace CryptoPP;
    bool ret = false;
    QByteArray publicKeydecBase64 = QByteArray::fromBase64(
    QString("MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuUtwPUEpohcEwy3/saBV \
            xr68u7Sibv6qmJYh+NfIlRcY/u3sAyXYmOOXyYSS3edkLas1yhHypyLFHKgtGXQO \
            6pbf3hku8dWiVbfsj047KT0pQdZpZl5qv+QeIMQNmEHL3S1dskB+TQDjx3Ee4Lx4 \
            SnpXAt+aqnaicwRTV/UOO1gsW5/5nZuYO7jxC5XFiN+cO0xrsrIOI4aDgTyCqvbC \
            2l08GvVkbsWwWTCdMRMDWuA0BpL6cIPNtiVF7MZy9F4AOF/4v8ZOiBjB+/CKLeYy \
            Cj/dRQszJ5WXiL8lpWhCMxYZac16nVapViVzh0f1nLZ8nkhrpzvoOovf6bZGkz3y \
            qwIDAQAB").toLatin1());

    ByteQueue Publicqueue;
    Weak::RSASSA_PKCS1v15_MD5_Verifier verifier;
    HexDecoder decoder(new Redirector(Publicqueue));
    std::string dec = QString(publicKeydecBase64.toHex()).toStdString();
    decoder.Put((const unsigned char*)dec.data(), dec.size());
    decoder.MessageEnd();
    verifier.AccessKey().Load(Publicqueue);

    StringSource signatureFile( QString(res).toStdString(), true, new CryptoPP::HexDecoder);
    if (signatureFile.MaxRetrievable() != verifier.SignatureLength())
    { throw std::string( "Signature Size Problem" ); }

    SecByteBlock signature1(verifier.SignatureLength());
    signatureFile.Get(signature1, signature1.size());

    // Verify
    SignatureVerificationFilter *verifierFilter = new SignatureVerificationFilter(verifier);
    verifierFilter->Put(signature1, verifier.SignatureLength());
    StringSource s(QString(md5).toStdString(), true, verifierFilter);

    // Result
    if(true == verifierFilter->GetLastResult()) {
        ret = true;
        qDebug() << "Signature on message verified" << endl;
    } else {
        qDebug() << "Message verification failed" << endl;
    }
    return ret;
}

bool UpgradeWid::checkFile()
{
    bool ret = false;
    QString fn = ui->lineEdit->text();
    if(!fn.isEmpty()) {
        if(fn.contains("bin") || fn.contains("tar")||fn.contains("cl") || fn.contains("leg")) {
            if(mData->devtype)
                ret = checkFileCrc(fn);
            else
                ret = true;
            if(ret) {
                mData->file = fn;
            } else {
#if LANGUAGE==1
                CriticalMsgBox box(this, tr("Upgrade file content error!, please re-select"));
#else
                CriticalMsgBox box(this, tr("升级文件内容验证错误! 请重新选择"));
#endif
            }
        } else {
#if LANGUAGE==1
            CriticalMsgBox box(this, tr("Upgrade file format error!, please re-select"));
#else
            CriticalMsgBox box(this, tr("升级文件格式错误! 请重新选择"));
#endif
        }
    } else {
#if LANGUAGE==1
        CriticalMsgBox box(this, tr("Please select the upgrade file "));
#else
        CriticalMsgBox box(this, tr("请选择升级文件"));
#endif
    }

    return ret;
}

void UpgradeWid::on_openBtn_clicked()
{
    static QString fn;
#if LANGUAGE==1
    fn = QFileDialog::getOpenFileName(0,tr("File selection"),fn,"",0);
#else
    fn = QFileDialog::getOpenFileName(0,tr("文件选择"),fn,"",0);
#endif
    if (!fn.isNull()) {
        ui->lineEdit->setText(fn);
        checkFile();
    }
}


void UpgradeWid::timeoutDone(void)
{
    QString str = mData->status;
    if(str.isEmpty())
#if LANGUAGE==1
        str = tr("Please start");
#else
        str = tr("请开始");
#endif
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

#if LANGUAGE==1
        QuMsgBox box(this, tr("Do you want to interrupt transmission?"));
#else
        QuMsgBox box(this, tr("是否要中断传输?"));
#endif
        if(box.Exec()) {
            mUpgradeThread->breakDown();
            if(0 == mData->devtype) {

#if LANGUAGE==1
                InfoMsgBox msg(this, tr("Software will reboot!!!"));
#else
                InfoMsgBox msg(this, tr("软件即将重启!!!"));
#endif
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

static unsigned long crc32_tbl[256] =
{
    0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL, 0x076DC419L,
    0x706AF48FL, 0xE963A535L, 0x9E6495A3L, 0x0EDB8832L, 0x79DCB8A4L,
    0xE0D5E91EL, 0x97D2D988L, 0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L,
    0x90BF1D91L, 0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
    0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L, 0x136C9856L,
    0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL, 0x14015C4FL, 0x63066CD9L,
    0xFA0F3D63L, 0x8D080DF5L, 0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L,
    0xA2677172L, 0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
    0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L, 0x32D86CE3L,
    0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L, 0x26D930ACL, 0x51DE003AL,
    0xC8D75180L, 0xBFD06116L, 0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L,
    0xB8BDA50FL, 0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
    0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL, 0x76DC4190L,
    0x01DB7106L, 0x98D220BCL, 0xEFD5102AL, 0x71B18589L, 0x06B6B51FL,
    0x9FBFE4A5L, 0xE8B8D433L, 0x7807C9A2L, 0x0F00F934L, 0x9609A88EL,
    0xE10E9818L, 0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
    0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL, 0x6C0695EDL,
    0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L, 0x65B0D9C6L, 0x12B7E950L,
    0x8BBEB8EAL, 0xFCB9887CL, 0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L,
    0xFBD44C65L, 0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
    0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL, 0x4369E96AL,
    0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L, 0x44042D73L, 0x33031DE5L,
    0xAA0A4C5FL, 0xDD0D7CC9L, 0x5005713CL, 0x270241AAL, 0xBE0B1010L,
    0xC90C2086L, 0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
    0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L, 0x59B33D17L,
    0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL, 0xEDB88320L, 0x9ABFB3B6L,
    0x03B6E20CL, 0x74B1D29AL, 0xEAD54739L, 0x9DD277AFL, 0x04DB2615L,
    0x73DC1683L, 0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
    0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L, 0xF00F9344L,
    0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL, 0xF762575DL, 0x806567CBL,
    0x196C3671L, 0x6E6B06E7L, 0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL,
    0x67DD4ACCL, 0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
    0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L, 0xD1BB67F1L,
    0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL, 0xD80D2BDAL, 0xAF0A1B4CL,
    0x36034AF6L, 0x41047A60L, 0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL,
    0x4669BE79L, 0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
    0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL, 0xC5BA3BBEL,
    0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L, 0xC2D7FFA7L, 0xB5D0CF31L,
    0x2CD99E8BL, 0x5BDEAE1DL, 0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL,
    0x026D930AL, 0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
    0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L, 0x92D28E9BL,
    0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L, 0x86D3D2D4L, 0xF1D4E242L,
    0x68DDB3F8L, 0x1FDA836EL, 0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L,
    0x18B74777L, 0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
    0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L, 0xA00AE278L,
    0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L, 0xA7672661L, 0xD06016F7L,
    0x4969474DL, 0x3E6E77DBL, 0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L,
    0x37D83BF0L, 0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
    0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L, 0xBAD03605L,
    0xCDD70693L, 0x54DE5729L, 0x23D967BFL, 0xB3667A2EL, 0xC4614AB8L,
    0x5D681B02L, 0x2A6F2B94L, 0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL,
    0x2D02EF8DL
};

void UpgradeWid::CRC32_Init()
{
    mCrc = 0xFFFFFFFFL;
}

void UpgradeWid::CRC32_Update(unsigned char *data, size_t len)
{
    unsigned long i;
    for (i = 0; i < len; i++)
    {
        mCrc = (mCrc >> 8) ^ crc32_tbl[(mCrc & 0xFF) ^  *data++];
    }
}

QByteArray UpgradeWid::CRC32_Final()
{
    mCrc ^= 0xFFFFFFFFUL;
    unsigned char temp[8];
    QByteArray ret;
    temp[0] = (mCrc & 0xFF000000UL) >> 24;
    ret.append(temp[0]);
    temp[1] = (mCrc & 0x00FF0000UL) >> 16;
    ret.append(temp[1]);
    temp[2] = (mCrc & 0x0000FF00UL) >> 8;
    ret.append(temp[2]);
    temp[3] = (mCrc & 0x000000FFUL);
    ret.append(temp[3]);

    return ret;
}
