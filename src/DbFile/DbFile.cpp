#include "../include/DbFile.hpp"
#include <system_error>
#include <unistd.h>
#include <iostream>
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

/**
 * Assume that the pointer in buff is pointing to space valid.
 * Return -1 on error
 */
ssize_t DbFile::read_at(void* buf, off_t offset, size_t sz) {
    checkIfFileDescriptorValid(theFd);
    ssize_t myReadBytes = pread(theFd, buf, sz, offset);
    if(myReadBytes == 0) {
        std::cout << "EOF\n";
    }
    if(myReadBytes != sz) {
        perror("Did not read enough bytes");
        return -1;
    }
    return myReadBytes;
}

ssize_t DbFile::write_at(void* buf, off_t offset, size_t sz) {
    checkIfFileDescriptorValid(theFd);
    ssize_t myWrittenBytes = pwrite(theFd, buf, sz, offset);
    if(myWrittenBytes != sz) {
        //Undo write and return failed. Let caller deal with it
        perror("Did not write enough bytes. Undo-ing the write");
        return -1;
    }
    return myWrittenBytes;
}

void DbFile::sync() {

}
void lock(DbFile::LockMode mode) {

}  
void unlock() {

}

void DbFile::close()
{
    if (theFd >= 0) {
        ::close(theFd);
        theFd = -1;
    }
}

static void checkIfFileDescriptorValid(int aFd) {
    if (aFd < 0) {
        throw std::system_error(errno, std::generic_category(), "Error with provided File Descriptor");
    }
}