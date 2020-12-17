#include <worker.h>
#include <time.h>
#include <unistd.h>
#include <QFile>
#include <iostream>
#include <QErrorMessage>
#include <terror.h>
#include <libudev.h>

#include "utils.h"

TaskWriter::TaskWriter()
{
    fsrc=nullptr;
    fdst=nullptr;
}
TaskWriter::~TaskWriter()
{
    if(fsrc)
    {
        fsrc->close();
        delete fsrc;
        fsrc=nullptr;
    }
    if(fdst)
    {
        fdst->close();
        delete fdst;
        fdst=nullptr;
    }
}

void TaskWriter::setSourceFile(QString path)
{
    if(fsrc != nullptr)
        delete fsrc;
    fsrc = new QFile(path);
}
void TaskWriter::setDestinationFile(QString path)
{
    if(fdst != nullptr)
        delete fdst;
    fdst = new QFile(path);
}
const QString TaskWriter::sourceName()
{
    if(!fsrc)
        return QString("");
    return fsrc->fileName();
}
const QString TaskWriter::destinationName()
{
    if(!fdst)
        return QString("");
    return fdst->fileName();
}

void TaskWriter::doWrite()
{
    is_stop = false;

    try
    {
    if(!fsrc->open(QIODevice::ReadOnly))
        throw TError(QString("Ошибка чтения файла %1").arg(fsrc->fileName()));
    if(!fdst->open(QIODevice::WriteOnly))
        throw TError(QString("Ошибка записи файла %1").arg(fdst->fileName()));
    }
    catch(TError err)
    {
        (new QErrorMessage)->showMessage(err.msg);
        emit finished();
        return;
    }

    char a[1024];
    qint64 dst_size=0;

    while( !fsrc->atEnd() && !is_stop )
    {
        int nBlocksize = fsrc->read(a,sizeof(a));
        dst_size+=fdst->write(a,nBlocksize);
        emit progress(dst_size);
    }
    doFinal();

}

void TaskWriter::doFinal()
{
    fsrc->close();
    fdst->close();
    if(is_stop)
    {
        QString file_name = fdst->fileName().replace(" ","\\ ");
        Utils::sys_call("rm -f "+file_name.toStdString());
    }
    delete fsrc;
    delete fdst;
    fsrc=nullptr;
    fdst=nullptr;

    emit finished();
}

//*************************************
//scan connect or disconnect device
//*************************************
TaskScanner::TaskScanner():is_stop(false)
{}

TaskScanner::~TaskScanner()
{}

void TaskScanner::doScan()
{
    struct udev *udev;
    struct udev_device *dev;
    struct udev_monitor *mon;
    int fd;
    //завершаем работ у сканирования если не удалось создать объект udev
    udev = udev_new();
    if (!udev)
    {
        std::cout<<"Can't create udev\n"<<std::endl;
        emit finished();
        return;
    }

    //udev   - посылает событие в userspace программам, используемым libudev
    mon = udev_monitor_new_from_netlink(udev, "udev");
    //отслеживаем флеш накопители
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);
    emit refrash_device();

    while(!is_stop)
    {
        fd_set fds;
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(fd, &fds))
        {
            emit refrash_device();
            dev = udev_monitor_receive_device(mon);
            if (dev)
                udev_device_unref(dev);// освобождаем объект
        }
        sleep(1);   // периодичность опроса 1 секунда
    }

    udev_unref(udev);   //освобождаем объект
    emit finished();    //сигнализируем о звершении задачи
}

