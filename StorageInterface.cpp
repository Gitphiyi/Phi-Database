#include <fcntl.h> 
#include <unistd.h>
#include <string>

// g++ -o main main.cpp StorageInterface.cpp

class StorageInfc {
    public:
        int a;
        /*
        * r/w/x
        */
        int create_file(const char* filename) {
            int fd = open(filename, O_RDWR | O_CREAT, 0644);
            printf("%d\n", fd);
            return 1;
        }
        int open_file() {
            return 1;
        }

        int read_file(const char* filename, int size, int) {
            return 1;
        }
};
