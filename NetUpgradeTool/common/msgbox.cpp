/*
 * msgbox.cpp
 * 信息提示框
 *  Created on: 2016年10月11日
 *      Author: Lzy
 */
#include "msgbox.h"
#include "datapacket.h"

extern void com_setBackColour(const QString &str,QWidget *target);

QuMsgBox::QuMsgBox(QWidget *parent,QString strContext) : QMessageBox(parent)
{
    //    setWindowFlags(Qt::WindowStaysOnTopHint);
    //    setWindowTitle(tr("提示信息"));
    //    com_setBackColour(tr("提示信息"),this);
    //    setWindowIcon(QIcon(":/images/logo.jpg"));
    resize(600, 400);

    setIcon(QMessageBox::Question);
    setText(strContext);

#if LANGUAGE==1
    confirmBut = addButton(tr("Confirm"), QMessageBox::AcceptRole);
    cancelBut = addButton(tr("Cancel"), QMessageBox::AcceptRole);
#else
    confirmBut = addButton(tr("确定"), QMessageBox::AcceptRole);
    cancelBut = addButton(tr("取消"), QMessageBox::AcceptRole);
#endif
    confirmBut->setFixedSize(60,25);
    cancelBut->setFixedSize(60,25);
    setDefaultButton(confirmBut);

    cancelBut->setFocus();
    confirmBut->setFocus();

    setStyleSheet( "QPushButton:hover{background-color:rgb(91, 237, 238);}" );
}

QuMsgBox::~QuMsgBox()
{
}

bool QuMsgBox::Exec(void)
{
    exec();
    if( clickedButton() == dynamic_cast<QAbstractButton *>(confirmBut))
        return true ;
    else if(clickedButton() == dynamic_cast<QAbstractButton *>(cancelBut))
        return false ;
    return false ;
}


WaringMsgBox::WaringMsgBox(QWidget *parent,QString strContext) : QMessageBox(parent)
{
    //    setWindowFlags(Qt::WindowStaysOnTopHint);
    //    setWindowTitle(tr("警告信息"));
    //    com_setBackColour(tr("警告信息"),this);
    //    setWindowIcon(QIcon(":/images/logo.jpg"));
    resize(500, 400);

    setIcon(QMessageBox::Warning);
    setText(strContext);

#if LANGUAGE==1
    confirmBut = addButton(tr("Confirm"), QMessageBox::AcceptRole);
    cancelBut = addButton(tr("Cancel"), QMessageBox::AcceptRole);
#else
    confirmBut = addButton(tr("确定"), QMessageBox::AcceptRole);
    cancelBut = addButton(tr("取消"), QMessageBox::AcceptRole);
#endif
    confirmBut->setMinimumSize(75,29);
    cancelBut->setMinimumSize(75,29);

    setDefaultButton(confirmBut);

    confirmBut->setFocus();
    cancelBut->setFocus();
    setStyleSheet( "QPushButton:hover{background-color:rgb(91, 237, 238);}" );
}

WaringMsgBox::~WaringMsgBox()
{
}

bool WaringMsgBox::Exec(void)
{
    exec();
    if( clickedButton() == dynamic_cast<QAbstractButton *>(confirmBut))
        return true ;
    else if(clickedButton() == dynamic_cast<QAbstractButton *>(cancelBut))
        return false ;
    return false ;
}


InfoMsgBox::InfoMsgBox(QWidget *parent,QString strContext) :
    QMessageBox(parent)
{
    //    setWindowFlags(Qt::WindowStaysOnTopHint);
    //    setWindowTitle(tr("信息提示"));
    //    com_setBackColour(tr("信息提示"),this);
    //    setWindowIcon(QIcon(":/images/logo.jpg"));
    resize(500, 400);

    setIcon(QMessageBox::Information);
    setText(strContext);

#if LANGUAGE==1
    confirmBut = addButton(tr("Confirm"), QMessageBox::AcceptRole);
#else
    confirmBut = addButton(tr("确定"), QMessageBox::AcceptRole);
#endif
    setDefaultButton(confirmBut);
    confirmBut->setMinimumSize(75,29);

    confirmBut->setFocus();
    setStyleSheet( "QPushButton:hover{background-color:rgb(91, 237, 238);}" );

    //QTimer::singleShot(AutoOutTime*1000,this,SLOT(close())); 自动关闭

    exec(); /* 自动 运行*/
}

InfoMsgBox::~InfoMsgBox()
{
}


CriticalMsgBox::CriticalMsgBox(QWidget *parent,QString strContext) :
    QMessageBox(parent)
{
    //    setWindowFlags(Qt::WindowStaysOnTopHint);
    //    setWindowTitle(tr("错误提示"));
    //    com_setBackColour(tr("错误提示"),this);
    //    setWindowIcon(QIcon(":/images/logo.jpg"));
    resize(500, 400);

    setIcon(QMessageBox::Critical);
    setText(strContext);


#if LANGUAGE==1
    confirmBut = addButton(tr("Confirm"), QMessageBox::AcceptRole);
#else
    confirmBut = addButton(tr("确定"), QMessageBox::AcceptRole);
#endif

    setDefaultButton(confirmBut);
    confirmBut->setMinimumSize(75,29);

    confirmBut->setFocus();
    setStyleSheet( "QPushButton:hover{background-color:rgb(91, 237, 238);}" );

    //QTimer::singleShot(AutoOutTime*1000,this,SLOT(close())); 自动关闭

    exec(); /* 自动 运行*/
}

CriticalMsgBox::~CriticalMsgBox()
{
}

