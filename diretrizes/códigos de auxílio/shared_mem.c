#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc, char **argv)
{
    pid_t pid;
    //key_t key = ftok(argv[0], getpid());
    int sid_a = shmget(IPC_PRIVATE, sizeof(int)*3, SHM_R|SHM_W|IPC_CREAT);
    int sid_x = shmget(IPC_PRIVATE, sizeof(int), SHM_R|SHM_W|IPC_CREAT);
    int *x;
    int *a;
    x = (int *) shmat(sid_x, NULL, 0);
    a = (int *) shmat(sid_a, NULL, 0);
    *x=0;
    pid = fork();

    if (pid < 0) { //erro
        fprintf(stderr, "Fork falhou!\n");
        return 1;
    }
    else if (pid == 0) { //processo filho
	    sleep(20);
	    printf("Filho - %d - %d\n", sid_a, sid_x); 
        x = (int *) shmat(sid_x, NULL, 0);
        *x = 2;
        a = (int *) shmat(sid_a, NULL, 0);
        a[0]=1;
        a[1]=1;
        a[2]=1;
        printf("Sou o filho (%d) - %d - %d \n", getpid(), *x, a[0]); 
        shmdt(x);
        shmdt(a);
    }
    else { //processo pai
        printf("Pai - %d - %d\n", sid_a, sid_x);
        fflush(stdout);
        //wait(NULL);
        
        while (*x!=2);
        
        printf("Pai (%d): Filho (%d) terminou! %d - %d\n", getpid(), pid, *x, a[0]);
        
        shmdt(x);
        shmdt(a);
        shmctl(sid_a, IPC_RMID, NULL);
        shmctl(sid_x, IPC_RMID, NULL);
    }
    return 0;
}

