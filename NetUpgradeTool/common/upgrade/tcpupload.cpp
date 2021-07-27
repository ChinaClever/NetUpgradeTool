#include "tcpupload.h"
#include "common/aes/aesencrypt.h"
#include <QCryptographicHash>

int chipVer = 0;

TcpUpload::TcpUpload(QObject *parent) : QObject(parent)
{
    mTcpClient = new TcpClient(this);
    connect(mTcpClient,SIGNAL(connectSig(int)), this,SLOT(connectSlot(int)));
}


/**
 * @brief 读文件所有内容
 * @param file
 */
bool TcpUpload::readFile(const QString &fn)
{
    QFile file(fn);
    bool ret = file.open(QIODevice::ReadOnly);
    if(ret){
        mByFile = file.readAll();
        int len = mByFile.size();
        if(len > 32) {
            for( int i = 32 ; i > 0 ; i--)
                mEndFile.append(mByFile.at(len-i));
        }
        file.close();
    }
    else
        qDebug() << " UP_TcpSent readFile err";

    return ret;
}

/**
 * @brief 发送文件长度
 * @return
 */
bool TcpUpload::sentLen(void)
{
    bool ret =  readFile(mTcpUpdateStr.file);
    if(ret) {
        char buf[100] = {0};
        int len = mByFile.length();
        QByteArray str= checkFlagAndVer(mByFile, len);
        if(str.size() == 0){
            ret = false;
        }else{
            mSentLen =  mByFile.length();
            sprintf(buf,"%d%s",mSentLen, str.data());
            qDebug()<<"str.size()"<<str.size()<<endl;
            ret = mTcpClient->sentMessage((uchar *)buf,strlen(buf)); // 发送文件长度
        }
    }
    return ret;
}

/**
 * @brief 检查包是否
 * @return
 */
QByteArray TcpUpload::checkFlagAndVer(QByteArray &array , int& index)
{
    bool flag = false;
    int len = array.size();
    QByteArray byte;
    QByteArray retArr ="";
    int count = 0;
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
        int pre = len - 1;
        for( int i = len-1 ; i > len-50 ; i--){
            if( array.at(i) == '&' ){
                count++;
                if(pre == len-1)
                    pre = i;
                else{
                    if(pre - i ==0)//避免&之间没有内容
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

        if(flag == false) return retArr;

        retArr= array.right(len-index);
        array.remove(index, len-index);
    }
    return retArr;
}

/**
 * @brief 开始发送
 * @return
 */
bool TcpUpload::startSent(void)
{
    bool ret = sentLen(); // 发送文件长度
    if(ret) {
        if(mTcpClient != NULL)
            ret =  mTcpClient->sentMessage(Sha256(mTcpUpdateStr.usr).toLatin1()+rand().toLatin1());
        if(mTcpClient != NULL)
            ret =  mTcpClient->sentMessage(Sha256(mTcpUpdateStr.pwd).toLatin1()+rand().toLatin1()); // 发送用户名信息
    }
    return ret;
}

/**
 * @brief 随机数
 * @return retStr
 */
QString TcpUpload::rand(void)
{
    QString retStr;
    QTime time = QTime::currentTime();
    qsrand(time.msec()*qrand()*qrand()*qrand()*qrand()*qrand()*qrand());
    int r = qrand();
    r = r < 0 ? -r : r;//处理随机数为负数的情况
    unsigned int n = r % 9999;//产生4位的随机数
    if(n < 10)
        retStr ="000"+ QString::number(n);
    else if( n < 100)
        retStr ="00"+ QString::number(n);
    else if( n < 1000)
        retStr ="0"+ QString::number(n);
    else
        retStr =QString::number(n);
    return retStr;
}

/**
 * @brief Md5
 * @param str
 * @return md5
 */
QString TcpUpload::Sha256(QString str)
{
    QString pwd=str;
    QString sha256;
    QByteArray ba,bb;
    QCryptographicHash md(QCryptographicHash::Sha256);
    ba.append(pwd);
    md.addData(ba);
    bb = md.result();
    sha256.append(bb.toHex());
    qDebug()<<sha256<<endl;
    return sha256;
}
/**
 * @brief 接收验证信息
 * @return
 */
bool TcpUpload::recvVerify(void)
{
    bool ret = true;
    int id = UP_CMD_SENT_OK;

    QByteArray data;
    int rtn = mTcpClient->getData(data); // 接收回应信息
    if(rtn > 0) {
        QString str;
        str.append(data);
        if(str.contains("ERR")){ // 验证错误
            ret = isStart = isRun = false;
            if(!isVeried) id = UP_CMD_PWDERR;// 账号错误
            else id = UP_CMD_ERR; // 账号错误
            emit connectSig(id); // 验证成功
            ret = false;
        }else if(str.contains("OK")){
            ret = true;
        }else if(str.contains("FINISH") && isStart){
            isStart = false;
            mByFile.clear();
            emit connectSig(UP_CMD_SENT_OK);
        }

        isVeried = true;
    }

    return ret;
}


bool TcpUpload::connectServer(const QString &ip)
{
    if(!ip.isEmpty()) {
        isVeried = false;
        isStart = true; // 开始运行
        mTcpClient->newConnect(ip);
    }

    return true;
}

void TcpUpload::upload(sTcpUpload &tcpStr)
{
    mTcpUpdateStr = tcpStr;
    connectServer(tcpStr.ip);
    mSentLen = 0;
    emit progressSig(0, "OK");
}

/**
 * @brief 计算进度
 */
void TcpUpload::progress(void)
{
    int num = mByFile.size();
    float p = (((mSentLen - num)*1.0)/mSentLen) * 100;
    emit progressSig(p, "OK");
}

/**
 * @brief 发送数据
 * @return
 */
bool TcpUpload::sentData(void)
{
    bool ret = true;
    if(mByFile.size() > 0)
    {
        int len = mByFile.size();
        if(len > 1024) len = 1024;   // 一次只发送1024个数据

        QByteArray data;
        for(int i=0; i<len; ++i) {
            data.append(mByFile.at(i));
        }

        mCount++;  // 连续发送数据包数量
        mByFile.remove(0, len);
        ret = mTcpClient->sentMessage(data);
        if(ret) progress();
    }

    return ret;
}




/**
 * @brief 连接响应函数
 * @param step 1
 */
void TcpUpload::connectSlot(int step)
{
    switch (step) {
    case UP_CMD_CONNECT: // 连接成功 首先发送文件长度
        startSent();
        mCount = 0;
        emit connectSig(UP_CMD_CONNECT); // 账号错误
        break;
    case UP_CMD_PWDERR:
    {
        isStart = isRun = false;
        mTcpClient->closeConnect();
        emit connectSig(UP_CMD_PWDERR); // 账号错误
        break;
    }
    case UP_CMD_READ: // 读取验证信息，
        if(recvVerify()) sentData();
        break;

    default:
        mByFile.clear();
        if(isStart) { // 没发送完数据时，异常情况抛出
            emit connectSig(step);
        }
        break;
    }
}


void TcpUpload::breakDown()
{
    isStart = false;
    mByFile.clear();
    mTcpClient->isOver = true;

    // mTcpClient->closeConnect();
}






