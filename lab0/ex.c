#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    FILE *fp = fopen("log.txt", "w");
    fprintf(fp, "Start\n");

    if (fork() == 0) {
        for (int i = 0; i < 1000; i++) {
            fprintf(fp, "Child line %d\n", i);
        }
    } else {
        wait(NULL);
        for (int i = 0; i < 1000; i++) {
            fprintf(fp, "Parent line %d\n", i);
        }
        
    }

    fclose(fp);
    return 0;
}