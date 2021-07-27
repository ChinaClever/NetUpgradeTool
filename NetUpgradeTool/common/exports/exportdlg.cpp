#include "exportdlg.h"
#include "ui_exportdlg.h"
#include <QFileDialog>
#include "common/msgbox.h"
#include "common/datapacket.h"

ExportDlg::ExportDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDlg)
{
    ui->setupUi(this);
    mExportThread = new Excel_SaveThread(this);

    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()),this, SLOT(timeoutDone()));
#if LANGUAGE==1
    ui->titleLab->setText("File export");
    ui->label->setText("Save path");
    ui->label_2->setText("File name");
    ui->pushButton->setText("Open");
    ui->exportBtn->setText("Export");
    ui->quitBtn->setText("Quit");
#endif

}

ExportDlg::~ExportDlg()
{
    delete ui;
}

void ExportDlg::on_pushButton_clicked()
{

#if LANGUAGE==1
    QFileDialog dlg(this,tr("Path select"));
#else
    QFileDialog dlg(this,tr("路径选择"));
#endif
    dlg.setFileMode(QFileDialog::DirectoryOnly);
    dlg.setDirectory("E:");
    if(dlg.exec() == QDialog::Accepted) {
        QString fn = dlg.selectedFiles().at(0);
        if(fn.right(1) != "/")  fn += "/";
        ui->pathEdit->setText(fn);
    }
}


/**
 * @brief 检查输入
 */
bool ExportDlg::checkInput()
{
    QString str = ui->pathEdit->text();
#if LANGUAGE==1
    if(str.isEmpty()) {
        CriticalMsgBox box(this, tr("The export path isn't empty！"));
        return false;
    }

    str = ui->fileEdit->text();
    if(str.isEmpty()) {
        CriticalMsgBox box(this, tr("The export file name isn't empty！"));
        return false;
    }

    str = ui->pathEdit->text() + ui->fileEdit->text() +".xlsx";
    QFile file(str);
    if (file.exists()){
        CriticalMsgBox box(this, str + tr("\nFile already exists！!"));
        return false;
    }
#else
    if(str.isEmpty()) {
        CriticalMsgBox box(this, tr("导出路径不能为空！"));
        return false;
    }

    str = ui->fileEdit->text();
    if(str.isEmpty()) {
        CriticalMsgBox box(this, tr("导出文件名不能为空！"));
        return false;
    }

    str = ui->pathEdit->text() + ui->fileEdit->text() +".xlsx";
    QFile file(str);
    if (file.exists()){
        CriticalMsgBox box(this, str + tr("\n文件已存在！!"));
        return false;
    }
#endif

    return true;
}

/**
 * @brief 导出完成
 */
void ExportDlg::exportDone()
{
    ui->exportBtn->setEnabled(true);
    ui->quitBtn->setEnabled(true);
#if LANGUAGE==1
    InfoMsgBox box(this, tr("\nExport successful!!\n"));
#else
    InfoMsgBox box(this, tr("\n导出完成!!\n"));
#endif
}


void ExportDlg::timeoutDone()
{
    int progress = mExportThread->getProgress();
    if(progress < 100)
        ui->progressBar->setValue(progress);
    else {
        ui->progressBar->setValue(100);
        timer->stop();
        exportDone();
    }
}

void ExportDlg::on_exportBtn_clicked()
{
    bool ret = checkInput();
    if(ret) {
        timer->start(100);
        ui->exportBtn->setDisabled(true);
        ui->quitBtn->setDisabled(true);

        sDataPacket *data = DataPacket::bulid()->data;
        data->file = ui->pathEdit->text() + ui->fileEdit->text();
        mExportThread->saveData(data->file, data->logs);
    }
}

void ExportDlg::on_quitBtn_clicked()
{
    if(!timer->isActive())
        this->close();
    else
    {

#if LANGUAGE==1
        InfoMsgBox box(this, tr("\nExport is not complete and cannot be shut down!!\n"));
#else
        InfoMsgBox box(this, tr("\n导出还没有完成，还不能关闭!!\n"));
#endif
    }
}
