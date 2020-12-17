#include "dialog.h"
#include "ui_dialog.h"
#include "flash.h"
#include "utils.h"
#include "terror.h"

#include <QFileDialog>
#include <iostream>
#include <QString>
#include <QErrorMessage>


using namespace std;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{

    ui->setupUi(this);
    threads.push_back(new QThread);
    writer = new TaskWriter();
    writer->moveToThread(threads.at(0));

    threads.push_back(new QThread);
    scanner = new TaskScanner();
    scanner->moveToThread(threads.at(1));

    fdlg = new QFileDialog();
    flash = nullptr;


    connect(threads.at(0), SIGNAL(started()), writer, SLOT(doWrite()));
    connect(threads.at(1), SIGNAL(started()), scanner, SLOT(doScan()));

    connect(writer, SIGNAL(progress(qint64)), this, SLOT(updateProgres(qint64)));
    connect(writer, SIGNAL(finished()), this, SLOT(writeFinish()));

    connect(scanner,SIGNAL(refrash_device()),this,SLOT(updateFlash()));
    connect(scanner,SIGNAL(finished()),threads.at(1),SLOT(quit()));

    connect(ui->btSelect, SIGNAL(clicked()), fdlg, SLOT(open()));
    connect(ui->btWrite, SIGNAL(clicked()), this, SLOT(writeFile()));
    connect(ui->btCancel, SIGNAL(clicked()), this,SLOT(writeCancel()));
    connect(ui->btMount, SIGNAL(clicked()), this, SLOT(mountDev()));

    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)),this,SLOT(setFlash(QString)));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)),this,SLOT(changeState()));

    connect(fdlg,SIGNAL(fileSelected(QString)), ui->leFile,SLOT(setText(QString)));


    threads.at(1)->start();
    ui->btCancel->setEnabled(false);
    ui->lbAvailable->setVisible(false);
    ui->lbProgres->setVisible(false);

    setWindowTitle("Writer");

}

Dialog::~Dialog()
{
    writer->doStop();
    scanner->doStop();

    for(auto it: threads)
    {
        it->wait(3000);
        it->quit();
    }
    delete writer;
    delete scanner;
    threads.clear();

    delete fdlg;
    delete ui;
}

void Dialog::changeState()
{
    if(!flash)
    {
        ui->lbAvailable->setVisible(false);
        return;
    }

    bool is_mount=flash->is_mount();
    ui->btMount->setToolTip(is_mount? "размонтировать":"монтировать");
    ui->btMount->setIcon(is_mount ? QIcon(":/icons/unmount.png") : QIcon(":/icons/mount.png"));
    if(is_mount)
    {
        ui->lbAvailable->setText("Доступно: " + Utils::fileSize(flash->freeSpace())+" / " + Utils::fileSize(flash->totalSpace()));
        ui->lbAvailable->setVisible(true);
    }
    else
        ui->lbAvailable->setVisible(false);
}

void Dialog::updateFlash()
{
    Utils::listFlash(vt_flash);

    //добавляем вновь подключенные накопители
    for(auto it:vt_flash)
    {
        int rez = ui->comboBox->findText(QString::fromStdString(it->model()));
        if (rez == -1)
            ui->comboBox->addItem(QString::fromStdString(it->model()));
    }

    //находим накопители которые были извлечены
    vector<QString> delete_list;
    for(int it=0;it<ui->comboBox->count();it++)
    {
        bool find=false;
        for(auto it_vt: vt_flash)
            if(it_vt->model() == ui->comboBox->itemText(it).toStdString())
            {
                find = true;
                break;
            }
        if(!find)
            delete_list.push_back(ui->comboBox->itemText(it));
    }

    //удаляем из списка накопители которых нет в системе
    for(auto it:delete_list)
    {
        int index=ui->comboBox->findText(it);
        if(index!=-1)
            ui->comboBox->removeItem(index);
    }
    setFlash(ui->comboBox->currentText());

    ui->btMount->setEnabled(!vt_flash.empty());
}
void Dialog::updateProgres(qint64 bsize)
{
    QFile fsrc(writer->sourceName());
    if(!fsrc.exists())
        return;

    qint64 prg=bsize*100/ fsrc.size();

    ui->prb->setValue(prg);
    setWindowTitle(QString("Writer %1 %").arg(prg));

    ui->lbProgres->setText(QString("Скопированно: %1 / %2").arg
                           (Utils::fileSize(bsize),
                            Utils::fileSize(fsrc.size())
                            )
                           );
}

void Dialog::setFlash(const QString& model)
{
    if(vt_flash.empty())
        flash = nullptr;
    for(auto it:vt_flash)
        if (it->model() == model.toStdString())
            flash = it;
}

void Dialog::mountDev()
{
    flash->mount();
    bool is_mount=flash->is_mount();
    ui->btMount->setToolTip(is_mount? "размонтировать":"монтировать");
    ui->btMount->setIcon(is_mount ? QIcon(":/icons/unmount.png") : QIcon(":/icons/mount.png"));
    if(is_mount)
    {
        ui->lbAvailable->setText("Доступно: " + Utils::fileSize(flash->freeSpace())+" / " + Utils::fileSize(flash->totalSpace()));
        ui->lbAvailable->setVisible(true);
    }
    else
        ui->lbAvailable->setVisible(false);
}

void Dialog::writeFile()
{
    try
    {
        QString source = ui->leFile->text();
        if(!QFile::exists(source))
            throw TError(QString("Файл %1 не найден").arg(source));

        QString file_name = "";

        //получаем имя файла из полного пути
        for(int i=source.length()-1; i!=0; i--)
        {
            if(source.at(i) == '/')
                break;
            file_name.prepend(source.at(i));
        }


        if(!flash)
            throw TError(QString("Не подключено ни одного usb накопителя"));
        if(flash && flash->path().empty())
            throw TError(QString("Накопитель %1 не смонтирован").arg(QString::fromStdString(flash->model())));
        if(flash->freeSpace() < QFile(source).size())
            throw TError(QString("Недостаточно свободного места на %1").arg(QString::fromStdString(flash->model())));

        writer->setSourceFile(source);
        QString dest(QString::fromStdString(flash->path()));
        if(!QDir(dest).exists())
                throw TError(QString("каталог %1 для записи не найден").arg(dest));
        dest+="/"+file_name;

        if(source == dest)
             throw TError(QString("исходный файл и файл назначения являются одним и тем же файлом"));
        writer->setDestinationFile(dest);

        ui->lbProgres->setVisible(true);
        ui->btCancel->setEnabled(true);
        ui->btWrite->setEnabled(false);

        threads.at(0)->start();
    }
    catch(TError err)
    {
        (new QErrorMessage)->showMessage(err.msg);
    }
}

void Dialog::writeCancel()
{
    writer->doStop();
}

void Dialog::writeFinish()
{
    ui->btCancel->setEnabled(false);
    ui->btWrite->setEnabled(true);
    ui->lbProgres->setVisible(false);
    ui->prb->setValue(0);
    setWindowTitle("Writer ");
    threads.at(0)->wait(4000);
    threads.at(0)->quit();
}
