#include <flash.h>
#include <libudev.h>
#include <iostream>
#include <fstream>

#include "utils.h"
#define BUFFER 200

struct udev_device* Utils::get_child(
    struct udev* udev, struct udev_device* parent, const char* subsystem)
{
    struct udev_device* child = NULL;
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_parent(enumerate, parent);
    udev_enumerate_add_match_subsystem(enumerate, subsystem);
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices) {
      const char *path = udev_list_entry_get_name(entry);
      child = udev_device_new_from_syspath(udev, path);
      break;
    }

    udev_enumerate_unref(enumerate);

    return child;
}


void Utils::listFlash(std::vector<Flash*>& vt_flash)
{
    vt_flash.clear();
    struct udev* udev = udev_new();
    enumerate_usb_mass_storage(udev,vt_flash);
    udev_unref(udev);
}

void Utils::enumerate_usb_mass_storage(struct udev* udev, std::vector<Flash*> &vt_flash)
{
    struct udev_enumerate* enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "scsi");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices)
    {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* scsi = udev_device_new_from_syspath(udev, path);

        struct udev_device* block = get_child(udev, scsi, "block");
        struct udev_device* scsi_disk = get_child(udev, scsi, "scsi_disk");

        struct udev_device* usb = udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");

        if (block && scsi_disk && usb)
        {
            Flash* ndev = new Flash(
                        udev_device_get_devnode(block),
                        udev_device_get_sysattr_value(scsi,"model")
                        );
            ndev->is_mount();
            vt_flash.push_back(ndev);
        }

        if (block)
          udev_device_unref(block);

        if (scsi_disk)
          udev_device_unref(scsi_disk);

        udev_device_unref(scsi);

      }
      udev_enumerate_unref(enumerate);
    }

std::string Utils::sys_call(const std::string &command)
{
    char tmpname[BUFFER];
    std::tmpnam( tmpname );

    std::system((command + ">" + tmpname).c_str());

    std::ifstream file(tmpname );
    std::string result;
    if (file)
    {
        while (!file.eof())
            result.push_back(file.get());
        file.close();
        if(!result.empty())
            result.erase(result.end()-1);
        if(!result.empty())
            result.erase(result.end()-1);
    }
    remove(tmpname);

    QString str = QString::fromStdString(result);
    str.replace("\\040"," ");

    return str.toStdString();
}
QString Utils::fileSize(qint64 mSize)
{
    qreal sz=mSize;
    int i=0;
    for(; sz > 1023; sz /= 1024, i++)
        if(i > 5)
            break;
return QString().setNum(sz,'g', 3) + "BKMGT"[i];
}

