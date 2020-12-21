#include "errstabwid.h"

ErrsTabWid::ErrsTabWid(QWidget *parent) : IpsTabWid(parent)
{
    initWid();
}

void ErrsTabWid::initWid()
{

#if LANGUAGE==1
    QString title = tr("Fail list");
#else
    QString title = tr("升级失败列表");
#endif
    QStringList header;
    header <<title;
    //header <<tr("fail list");

    initTableWid(header, 0, title);
}


void ErrsTabWid::timeoutDone()
{
    updateData(mData->errs);
}

void ErrsTabWid::languageChanged()
{
    mheader.clear();
    static int count = 1;
    if(count%3)
    {
        if(count == 5)
            count = 1;
        mheader<<"升级失败列表";
    }
    else
    {
        mheader<<"Fail list";
    }
    qDebug()<<count<<mheader;
    count++;
    changeHeader(mheader);
}
