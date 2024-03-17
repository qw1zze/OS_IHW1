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

void write_process(int* fdHandlerWriter, char* buffer, char* output_name) {

    ssize_t read_pipe = read(fdHandlerWriter[0], buffer, 5000);

    if (read_pipe < 0) {
        printf("Can\'t read from pipe\n");
        exit(-1);
    }

    ssize_t write_count = write_file(output_name, buffer, read_pipe);

    if(write_count != read_pipe){
        printf("Can\'t write string\n");
        exit(-1);
    }

    if(close(fdHandlerWriter[0]) < 0){
        printf("Can\'t close pipe\n");
        exit(-1);
    }
}

void solve_process(int* fdReaderHandler, int* fdHandlerWriter, char* buffer) {
    if (close(fdHandlerWriter[0]) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }

    if (close(fdReaderHandler[1]) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }

    ssize_t read_pipe = read(fdReaderHandler[0], buffer, 5000);

    if (read_pipe < 0) {
        printf("Can\'t read from pipe\n");
        exit(-1);
    }

    if(close(fdReaderHandler[0]) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }

    ssize_t count = get_count(buffer, buffer, read_pipe);

    ssize_t write_pipe = write(fdHandlerWriter[1], buffer, count);

    if (write_pipe != count) {
        printf("Can\'t write string\n");
        exit(-1);
    }

    if (close(fdHandlerWriter[1]) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }
}

void read_process(int* fdReaderHandler, int* fdHandlerWriter, char* buffer, char* input_name) {
    ssize_t read_file = read_from_file(input_name, buffer, 5000);

    if(close(fdReaderHandler[0]) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }

    if(close(fdHandlerWriter[1]) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }

    ssize_t write_pipe = write(fdReaderHandler[1], buffer, read_file);
    if (write_pipe != read_file) {
        printf("Can\'t write to pipe\n");
        exit(-1);
    }

    if (close(fdReaderHandler[1]) < 0) {
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

    int fdReaderHandler[2];
    int fdHandlerWriter[2];

    if (pipe(fdReaderHandler) < 0 || pipe(fdHandlerWriter) < 0) {
        printf("Error to open pipe\n");
        exit(-1);
    }

    int pid = fork();

    if (pid == -1) {
        printf("Incorrect fork");
        exit(-1);
    } else if (pid == 0) {
        //Если ребенок, обрабатывает строку
        solve_process(fdReaderHandler, fdHandlerWriter, buffer);

    } else {
        //Если родитель, читает и записывает строки
        read_process(fdReaderHandler, fdHandlerWriter, buffer, input_name);
        write_process(fdHandlerWriter, buffer, output_name);

    }
}
