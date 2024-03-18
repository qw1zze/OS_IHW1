#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <ctype.h>

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

void solve_process(char first_pipe[], char second_pipe[], char* buffer) {

    int first_fd;
    if((first_fd = open(first_pipe, O_RDONLY)) < 0){
        printf("Can\'t open FIFO for reading\n");
        exit(-1);
    }

    ssize_t read_pipe = read(first_fd, buffer, 5000);

    if (read_pipe < 0) {
        printf("Can\'t read from pipe\n");
        exit(-1);
    }

    if(close(first_fd) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }

    ssize_t count = get_count(buffer, buffer, read_pipe);

    int second_fd;
    if((second_fd = open(second_pipe, O_WRONLY)) < 0){
        printf("Can\'t open FIFO for reading\n");
        exit(-1);
    }

    ssize_t write_pipe = write(second_fd, buffer, count);

    if (write_pipe != count) {
        printf("Can\'t write string\n");
        exit(-1);
    }

    if (close(second_fd) < 0) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }
}

int main(int argc, char** argv) {

    char buffer[5000];
    char first_pipe[] = "first.fifo";
    char second_pipe[] = "second.fifo";

    solve_process(first_pipe, second_pipe, buffer);
}
