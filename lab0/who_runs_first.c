// who_runs_first.c
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define ITER_MAX 1000
#define SLEEP 0

int main(void)
{
    char msg_p[] = "parent";
    char msg_c[] = "child";
    char nl[] = "\n";

    for (int i = 0; i < ITER_MAX; i++)
    {
        if (fork())
        {
            sleep(SLEEP);
            printf("parent ");
            fflush(stdout);
            //write(STDOUT_FILENO, msg_p, sizeof(msg_p) - 1);
            wait(NULL);
        }
        else
        {
            sleep(SLEEP);

            printf("child ");
            fflush(stdout);
            //write(STDOUT_FILENO, msg_c, sizeof(msg_c) - 1);
            return 0;
        }
        sleep(SLEEP);

        printf("%i\n",i);
        fflush(stdout);
        //write(STDOUT_FILENO, nl, sizeof(nl) - 1);
    }
    return 0;
}