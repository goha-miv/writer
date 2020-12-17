#ifndef FLASH_H
#define FLASH_H
#include <iostream>
#include <fcntl.h>
#include <sys/vfs.h>
#include <sys/stat.h>
typedef long long qint64;           /* 64 bit signed */
//-------------------------------------
//содержит информацию о usb накопителе
//-------------------------------------
class Flash
{
private:
    std::string m_devname;  // например /dev/sdb1
    std::string m_model;    //модель накопителя
    std::string m_path;     //точка монтирования накопителя
public:
    Flash(std::string _devname="", std::string _model="", std::string _path=""):
        m_devname(_devname),
        m_model(_model),
        m_path(_path)
    {
        m_devname+="1";
    }
    //Methods
    std::string model() { return m_model; }
    std::string devname() { return m_devname; }
    std::string path() { return m_path; }
    qint64 freeSpace();
    qint64 totalSpace();


    bool is_mount();
    void mount();       // монтирование.размонтирование usb накопителей
};

#endif // FLASH_H
