#include "flash.h"
#include "utils.h"

bool Flash::is_mount()
{
    std::string command="grep "+m_devname+" /proc/mounts | awk '{printf \"%s\\n\",$2}'";
    m_path=Utils::sys_call(command);
    return(!m_path.empty());
}

void Flash::mount()
{
    std::string command = "udisksctl ";
    command +=m_path.empty()? "mount":"unmount";
    command +=" -b " + m_devname;
    Utils::sys_call(command);
}

qint64 Flash::freeSpace()
{
    struct statfs st;
    if(statfs(m_path.c_str(), &st) == 0)
            return st.f_frsize * st.f_bfree;
    return 0;
}

qint64 Flash::totalSpace()
{
    struct statfs st;
    if(statfs(m_path.c_str(), &st) == 0)
            return st.f_frsize * st.f_blocks;
    return 0;
}
