#include "devselectwid.h"
#include "ui_devselectwid.h"
#include "msgbox.h"

DevSelectWid::DevSelectWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DevSelectWid)
{
    ui->setupUi(this);
    mData = DataPacket::bulid()->data;
    timer = new QTimer(this);
    timer->start(200);
    connect(timer, SIGNAL(timeout()),this, SLOT(timeoutDone()));
    mCount = 1;
    //    ui->groupBox->setTitle("Type Of Equipment Selection Zone");
    //    ui->label->setText("username");
    //    ui->label_2->setText("password");
    //    ui->okBtn->setText("confirm");
}

DevSelectWid::~DevSelectWid()
{
    delete ui;
}


void DevSelectWid::timeoutDone(void)
{
    this->setDisabled(mData->isRun);
}

bool DevSelectWid::checkInput()
{
    QString str;
    QString user =ui->userEdit->text();
    if(user.isEmpty())
        str = tr("用户名不能为空，请重新输入!!");
    //str = tr("Username is empty，please re-enter!!");

    QString pwd = ui->pwdEdit->text();
    if(pwd.isEmpty())
        str = tr("密码不能为空，请重新输入!!");
    //str = tr("Password is empty，please re-enter!!");

    bool ret = true;
    if(str.isEmpty()) {
        mData->usr = user;
        mData->pwd = pwd;
    } else {
        ret = false;
        CriticalMsgBox box(this, str);
    }

    return ret;
}

void DevSelectWid::setenabled(bool e)
{
    ui->userEdit->setEnabled(e);
    ui->pwdEdit->setEnabled(e);
}

void DevSelectWid::on_okBtn_clicked()
{
    bool en = false;
    QString str = tr("修改");
    //QString str = tr("modify");
    if(mCount++ %2) {
        if(mData->devtype) {//只有当升级方式为tcp时，才需要账号和密码
            if(!checkInput()) {
                mCount--; return;
            }
        }
    } else {
        en = true;
        str = tr("确认");
        //str = tr("confirm");
    }

    setenabled(en);
    ui->okBtn->setText(str);
}

void DevSelectWid::languageChanged()
{
    ui->retranslateUi(this);
}


void DevSelectWid::on_ChooseTypeBox_currentIndexChanged(int index)
{
    bool en = false;
    if(index) en = true;
    mData->devtype = index;
    ui->userEdit->setEnabled(en);
    ui->pwdEdit->setEnabled(en);
}
