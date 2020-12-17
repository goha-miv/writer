#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <string>
#include <QString>
class Flash;

//------------------------
//класс сторонних функций
//------------------------
class Utils
{
private:
    Utils(){}
public:
    static struct udev_device* get_child(
         struct udev* udev, struct udev_device* parent, const char* subsystem);
    static void listFlash(std::vector<Flash *> &);                                  //получить список usb накопителей

    static void enumerate_usb_mass_storage(struct udev* udev, std::vector<Flash *> &);

    //вызов системных команд linux
    //возвращает результат выполнения команды в виде строки
    static std::string sys_call(const std::string& str);

    //возвращает размер файла с постфиксом
    //B в байтах
    //M в мегабайтах
    //G в гигабайтах
    //T в терабайтах
    static QString fileSize(qint64 mSize);
};

#endif // UTILS_H
