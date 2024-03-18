#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <ctype.h>

ssize_t read_from_file(char* name, char* buffer, int size) {
    int fd;
    if ((fd = open(name, O_RDONLY)) < 0) {
        printf("Can\'t open file\n");
        exit(-1);
    }

    ssize_t read_bytes = read(fd, buffer, size);
    if (read_bytes == -1) {
        printf("Can\'t read file\n");
        exit(-1);
    }

    buffer[read_bytes] = '\0';
    if(close(fd) < 0) {
        printf("Can\'t close file\n");
        exit(-1);
    }

    return read_bytes;
}

ssize_t write_file(char* name, char* buffer, int size) {
    int fd;
    if((fd = open(name, O_WRONLY | O_CREAT, 0666)) < 0){
        printf("Can\'t open file\n");
        exit(-1);
    }

    ssize_t count_write = write(fd, buffer, size);
    if(count_write != size){
        printf("Can\'t write file\n");
        exit(-1);
    }

    if(close(fd) < 0){
        printf("Can\'t close file\n");
        exit(-1);
    }
    return count_write;
}

void write_process(int second_pipe[], char* buffer, char* output_name) {
    int second_fd;
    if((second_fd = open(second_pipe, O_RDONLY)) < 0){
        printf("Can\'t open FIFO for reading\n");
        exit(-1);
    }

    ssize_t read_pipe = read(second_fd, buffer, 5000);

    if (read_pipe < 0) {
        printf("Can\'t read from pipe\n");
        exit(-1);
    }

    ssize_t write_count = write_file(output_name, buffer, read_pipe);

    if(write_count != read_pipe){
        printf("Can\'t write string\n");
        exit(-1);
    }

    if(close(second_fd) < 0){
        printf("Can\'t close pipe\n");
        exit(-1);
    }
}

void read_process(int first_pipe[], char* buffer, char* input_name) {
    ssize_t read_file = read_from_file(input_name, buffer, 5000);

    int first_fd;
    if((first_fd = open(first_pipe, O_WRONLY)) < 0){
        printf("Can\'t open FIFO for writing\n");
        exit(-1);
    }

    ssize_t write_pipe = write(first_fd, buffer, read_file);
    if (write_pipe != read_file) {
        printf("Can\'t write to pipe\n");
        exit(-1);
    }

    if (close(first_fd) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Incorrect number of arguments");
        exit(-1);
    }

    char buffer[5000];
    char* input_name = argv[1];
    char* output_name = argv[2];
    char first_pipe[] = "first.fifo";
    char second_pipe[] = "second.fifo";

    mknod(first_pipe, S_IFIFO | 0666, 0);
    mknod(second_pipe, S_IFIFO | 0666, 0);

    read_process(first_pipe, buffer, input_name);
    write_process(second_pipe, buffer, output_name);
}
