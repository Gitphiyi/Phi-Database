#include <fcntl.h> 
#include <unistd.h>
#include <string>

int create_file(char* filename) {
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    return 1;
}