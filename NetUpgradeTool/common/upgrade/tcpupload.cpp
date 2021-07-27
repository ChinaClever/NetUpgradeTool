#include "tcpupload.h"

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
        char buf[32] = {0};
        mSentLen =  mByFile.length();
        sprintf(buf,"%d",mSentLen);
        ret = mTcpClient->sentMessage((uchar *)buf,strlen(buf)); // 发送文件长度
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
        ret =  mTcpClient->sentMessage(mTcpUpdateStr.usr.toLatin1());
        ret =  mTcpClient->sentMessage(mTcpUpdateStr.pwd.toLatin1()); // 发送用户名信息
    }
    return ret;
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
        if(!str.contains("OK")){ // 验证错误
            ret = isStart = isRun = false;
            if(!isVeried) id = UP_CMD_PWDERR; // 账号错误
            else id = UP_CMD_ERR; // 账号错误
            emit connectSig(id); // 验证成功
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
    } else  {
        if(isStart) {
            isStart = false; // 发送完成，标志为复位
            emit connectSig(UP_CMD_SENT_OK); // 验证成功
        }
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






