#include "DbFile.hpp"
#include <system_error>
#include <unistd.h>
#include <iostream>
#include <bitset>
#include <sys/fcntl.h>
#include <filesystem>

namespace DB {

    static DbFile* singletonInstance = nullptr;

    void DbFile::initialize(const std::string& path, bool ifMissing, Page& page) {
        if (singletonInstance == nullptr) {
            singletonInstance = new DbFile(path, ifMissing, page);
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

    DbFile::DbFile(const std::string& path, bool ifMissing, Page& page) : theFd(-1), theBuffer(page)
    {
        int flags{O_RDWR};
        if (ifMissing) {
            flags |= O_CREAT;
        }
        // if (!std::filesystem::exists("db")) {
        //     std::filesystem::create_directory("db");
        // }
        string filepath = path;
        std::cout << filepath << std::endl;
        theFd = ::open(filepath.c_str(), flags, 0644); //0644 is octal for rw for all users
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
    ssize_t DbFile::read_at(off_t offset) {
        checkIfFileDescriptorValid(theFd);
        ssize_t myReadBytes = pread(theFd, &theBuffer, PAGE_SIZE + PAGE_METADATA, offset * PAGE_SIZE);
        if(myReadBytes == 0) {
            std::cout << "EOF\n";
        }
        if(myReadBytes != PAGE_SIZE) {
            perror("Did not read enough bytes");
            return -1;
        }

        return myReadBytes;
    }

    ssize_t DbFile::read_at(Page& buffer, off_t offset) {
        checkIfFileDescriptorValid(theFd);
        ssize_t myReadBytes = pread(theFd, &buffer, PAGE_SIZE + PAGE_METADATA, offset * PAGE_SIZE);
        if(myReadBytes == 0) {
            std::cout << "EOF\n";
        }
        if(myReadBytes != PAGE_SIZE) {
            perror("Did not read enough bytes");
            return -1;
        }

        return myReadBytes;
    }

    ssize_t DbFile::write_at(off_t offset) {
        checkIfFileDescriptorValid(theFd);

        ssize_t myWrittenBytes = pwrite(theFd, &theBuffer, PAGE_SIZE + PAGE_METADATA, offset * PAGE_SIZE);
        if(myWrittenBytes != PAGE_SIZE) {
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
}