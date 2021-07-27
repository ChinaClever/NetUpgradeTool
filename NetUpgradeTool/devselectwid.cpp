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
    ui->ChooseTypeBox->setCurrentIndex(mData->devtype);
#if LANGUAGE==1
    ui->groupBox->setTitle("Type Of Equipment Selection Zone");
    ui->label->setText("Username");
    ui->label_2->setText("Password");
    ui->okBtn->setText("Confirm");
    ui->label_3->setText("Upgrade mode");
#endif
    if(!mData->devtype)
        setenabled(false);
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
#if LANGUAGE==1
        str = tr("Username is empty，please re-enter!!");
#else
        str = tr("用户名不能为空，请重新输入!!");
#endif

    QString pwd = ui->pwdEdit->text();
    if(pwd.isEmpty())
#if LANGUAGE==1
        str = tr("Password is empty，please re-enter!!");
#else
        str = tr("密码不能为空，请重新输入!!");
#endif

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
#if LANGUAGE==1
    QString str = tr("Modify");
#else
    QString str = tr("修改");
#endif
    if(mCount++ %2) {
        if(mData->devtype) {//只有当升级方式为tcp时，才需要账号和密码
            if(!checkInput()) {
                mCount--; return;
            }
        }
    } else {
        en = true;
#if LANGUAGE==1
        str = tr("Confirm");
#else
        str = tr("确认");
#endif
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
