#include "page-manager/DbFile.hpp"

#include <system_error>
#include <unistd.h>
#include <iostream>
#include <bitset>
#include <sys/fcntl.h>
#include <filesystem>

namespace DB {

    static DbFile* singletonInstance = nullptr;

    void DbFile::initialize(const string& path, bool ifMissing) {
        if (singletonInstance == nullptr) {
            singletonInstance = new DbFile(path, ifMissing);
        }
    }

    DbFile& DbFile::getInstance() {
        if (!singletonInstance) {
            throw std::runtime_error("DbFile has not been initialized. Call DbFile::initialize() first.");
        }
        return *singletonInstance;
    }

    void DbFile::checkIfFileDescriptorValid(int aFd) {
        if (aFd < 0) {
            throw std::system_error(errno, std::generic_category(), "Error with provided File Descriptor");
        }
    }

    DbFile::DbFile(const string& path, bool ifMissing) : 
    theDbFd(-1), 
    theFdMap()
    {
        int flags{O_RDWR};
        if (ifMissing) {
            flags |= O_CREAT;
        }
        theDbFd = ::open(path.c_str(), flags, 0644); //0644 is octal for rw for all users
        if (theDbFd < 0) {
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
    ssize_t DbFile::db_read_at(off_t offset, Page& buffer) {
        checkIfFileDescriptorValid(theDbFd);
        ssize_t myReadBytes = pread(theDbFd, &buffer, PAGE_SIZE, offset * PAGE_SIZE);
        if(myReadBytes == 0) {
            std::cout << "EOF\n";
        }
        if(myReadBytes != PAGE_SIZE) {
            perror("Did not read enough bytes");
            return -1;
        }

        return myReadBytes;
    }

    ssize_t DbFile::read_at(off_t offset, Page& buffer, int fd) {
        checkIfFileDescriptorValid(fd);
        ssize_t myReadBytes = pread(fd, &buffer, PAGE_SIZE, offset * PAGE_SIZE);
        if(myReadBytes == 0) {
            std::cout << "EOF\n";
        }

        return myReadBytes;
    }

    ssize_t DbFile::db_write_at(off_t offset, Page& buffer) {
        checkIfFileDescriptorValid(theDbFd);

        ssize_t myWrittenBytes = pwrite(theDbFd, &buffer, PAGE_SIZE, offset * PAGE_SIZE);
        if(myWrittenBytes != PAGE_SIZE) {
            perror("Did not write enough bytes. Undo-ing the write");
            return -1;
        }
        return myWrittenBytes;
    }

    ssize_t DbFile::write_at(off_t offset, Page& buffer, int fd) {
        checkIfFileDescriptorValid(fd);

        ssize_t myWrittenBytes = pwrite(fd, &buffer, PAGE_SIZE, offset * PAGE_SIZE);
        if(myWrittenBytes != PAGE_SIZE) {
            perror("Did not write enough bytes. Undo-ing the write");
            return -1;
        }
        return myWrittenBytes;
    }

    int DbFile::get_path(const string& path) {
        if(!theFdMap.contains(path)) {
            return -1;
        }
        return theFdMap[path];
    }

    int DbFile::add_path(const string& path) {
        if(get_path(path) != -1) {
            std::cout << "file descriptor already exists in map" << std::endl;
            return -1;
        }
        int flags{O_CREAT | O_RDWR};
        int fd = ::open(path.c_str(), flags, 0644);
        theFdMap[path] = fd;
        std::cout << "successfully added new file descriptor for path " << path << std::endl;
        return fd;
    }

    void DbFile::sync() {

    }
    
    void lock(DbFile::LockMode mode) {

    }  
    void unlock() {

    }

    void DbFile::close()
    {
        if (theDbFd >= 0) {
            ::close(theDbFd);
            theDbFd = -1;
        }
    }
}