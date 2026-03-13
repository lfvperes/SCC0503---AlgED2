/* program to read characters from a file and write them to the terminal screen */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main() {
    char c;
    int fd;     /* file descriptor*/
    char filename[20];

    printf("Enter the name of the file ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = '\0';
    
    fd = open(filename,O_RDONLY);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    while (read(fd, &c, 1) > 0)
        write(STDOUT_FILENO, &c, 1);

    close(fd);
    
    return 0;
}