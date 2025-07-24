#include "../include/DbFile.hpp"
#include <system_error>
#include <unistd.h>
#include <sys/fcntl.h>

DbFile::DbFile(const std::string& path, bool ifMissing, int permissions) : theFd(-1)
{
    int flags{O_RDWR};
    if (ifMissing) {
        flags |= O_CREAT;
    }
    theFd = ::open(path.c_str(), flags, permissions);
    if (theFd < 0) {
        throw std::system_error(errno, std::generic_category(), "File could not be created");
    }
}

DbFile::~DbFile()
{
    close();
}

void DbFile::close()
{
    if (theFd >= 0) {
        ::close(theFd);
        theFd = -1;
    }
}