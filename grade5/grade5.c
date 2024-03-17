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

ssize_t get_count(char* str, char* buffer, size_t size) {
    size_t count_digits = 0;
    size_t count_letters = 0;

    for(size_t i = 0; i < size; ++i){
        if(isdigit(str[i])) {
            ++count_digits;
        } else if (islower(str[i]) || isupper(str[i])) {
            ++count_letters;
        }
    }

    char to_str[5000];
    sprintf(to_str, "Count of digits: %d \nCount of letters: %d", count_digits, count_letters);

    int ind = 0;
    while (to_str[ind] != '\0') {
        printf("%c", to_str[ind]);
        buffer[ind] = to_str[ind];
        ++ind;
    }

    buffer[ind] = '\0';
    return ind;
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

void write_process(char pipe[], char* buffer, char* output_name) {
    int fdWriter;
    if((fdWriter = open(pipe, O_RDONLY)) < 0){
        printf("Can'\t open fifo");
        exit(-1);
    }

    ssize_t read_pipe = read(fdWriter, buffer, 5000);

    if (read_pipe < 0) {
        printf("Can\'t read from pipe\n");
        exit(-1);
    }

    ssize_t write_count = write_file(output_name, buffer, read_pipe);

    if(write_count != read_pipe){
        printf("Can\'t write string\n");
        exit(-1);
    }

    if(close(fdWriter) < 0){
        printf("Can\'t close pipe\n");
        exit(-1);
    }
}

void solve_process(char first_pipe[], char second_pipe[], char* buffer) {
    int fdHandler;
    if((fdHandler = open(first_pipe, O_RDONLY)) < 0){
        printf("Can'\t open fifo");
        exit(-1);
    }
    ssize_t read_pipe = read(fdHandler, buffer, 5000);

    if (read_pipe < 0) {
        printf("Can\'t read from pipe\n");
        exit(-1);
    }

    if(close(fdHandler) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }

    ssize_t count = get_count(buffer, buffer, read_pipe);

    if((fdHandler = open(second_pipe, O_WRONLY)) < 0){
        printf("Can'\t open fifo");
        exit(-1);
    }

    ssize_t write_pipe = write(fdHandler, buffer, count);

    if (write_pipe != count) {
        printf("Can\'t write string\n");
        exit(-1);
    }

    if (close(fdHandler) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }
}

void read_process(char pipe[], char* buffer, char* input_name) {
    ssize_t read_file = read_from_file(input_name, buffer, 5000);

    int fdReader;
    if((fdReader = open(pipe, O_WRONLY)) < 0){
        printf("Can'\t writing fifo");
        exit(-1);
    }

    ssize_t write_pipe = write(fdReader, buffer, read_file);
    if (write_pipe != read_file) {
        printf("Can\'t write to pipe\n");
        exit(-1);
    }

    if (close(fdReader) < 0) {
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

    int pid = fork();

    if (pid == -1) {
        printf("Incorrect fork");
        exit(-1);
    } else if (pid == 0) {
        //Если ребенок, записывает в файл
        write_process(second_pipe, buffer, output_name);

    } else {
        pid_t reader_pid = fork();

        if (reader_pid == -1) {
            printf("Incorrect fork");
            exit(-1);
        } else if (reader_pid == 0) {
            //Если ребенок, обрабатывает строку
            solve_process(first_pipe, second_pipe, buffer);

        } else {
            //Если родитель, читает файл
            read_process(first_pipe, buffer, input_name);

        }
    }
}
