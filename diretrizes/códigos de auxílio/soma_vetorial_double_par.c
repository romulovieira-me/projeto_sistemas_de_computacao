#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define __USE_GNU
#include <sched.h>
#undef __USE_GNU
#include <errno.h>
#include <math.h>

double waste_time(long n)
{
    double res = 0;
    long i = 0;
    while(i <n * 200) {
        i++;
        res += sqrt (i);
    }
    return res;
}

int main(int argc, char **argv)
{
	//unsigned long int affinity_mask;
	int status;
	//cpu_set_t affinity_mask;
	//CPU_ZERO(&affinity_mask);
	pid_t pid[2];
	FILE *inp_a, *inp_b, *out_c;
	long int size = atol(argv[4]);
	long int i;
	double *a, *b, *c;

	a = (double *) malloc(sizeof(double) * size);
	b = (double *) malloc(sizeof(double) * size);
	inp_a = fopen(argv[1], "r");
	inp_b = fopen(argv[2], "r");
	int sid_c = shmget(IPC_PRIVATE, sizeof(double)*size, SHM_R|SHM_W|IPC_CREAT);
	for (i=0; i<size; i++) 
	{
		fscanf(inp_a, "%lf\n", &a[i]);
		fscanf(inp_b, "%lf\n", &b[i]);
	}
	fclose(inp_a);
	fclose(inp_b);
	//affinity_mask = 1;
	/*CPU_SET(0, &affinity_mask);
	if (sched_setaffinity(0, sizeof(affinity_mask), &affinity_mask) < 0) 
		perror("sched_setaffinity 1");*/
	pid[0]=fork();
	if (pid[0] < 0) { //erro
        	fprintf(stderr, "Fork falhou!\n");
        	return 1;
	}
	else if (pid[0] == 0) { //processo filho
		c = (double *) shmat(sid_c, NULL, 0);
		for (i=0; i<size/2; i++) 
		{
			c[i]=waste_time(abs(a[i]))+waste_time(abs(b[i]));
			//c[i] = a[i] + b[i];
		}
		shmdt(c);
		//printf("Filho 1 terminou\n");
		/*float x=waste_time (2000);
		printf ("result: %f\n", x);*/
	}
	else { //processo pai
		//affinity_mask = 1<<1;
		//CPU_ZERO(&affinity_mask);
		//CPU_SET(1, &affinity_mask);
		//if (sched_setaffinity(0, sizeof(affinity_mask), &affinity_mask) < 0) 
		//	perror("sched_setaffinity 1");
		pid[1]=fork();
		if (pid[1] < 0) { //erro
        		//fprintf(stderr, "Fork falhou!\n");
        		return 1;
		}
		else if (pid[1] == 0) { //processo filho			
			c = (double *) shmat(sid_c, NULL, 0);
			for (i=size/2; i<size; i++) 
			{
				c[i]=waste_time(abs(a[i]))+waste_time(abs(b[i]));
				//c[i] = a[i] + b[i];
			}
			shmdt(c);
			//printf("Filho 2 terminou\n");
			/*float x=waste_time (2000);
			printf ("result: %f\n", x);*/
		}
		else
		{
			for(i=0; i<2; i++)
			{
				wait();
				//waitpid(pid[i], &status, 0);
			}
			//intf("Filhos terminaram!\n");
			c = (double *) shmat(sid_c, NULL, 0);
			out_c = fopen(argv[3], "w");
			for (i=0; i<size; i++) 
			{
				fprintf(out_c, "%lf\n", c[i]);
			}
			fclose(out_c);
			shmdt(c);
			shmctl(sid_c, IPC_RMID, NULL);
		}
	}
	return 0;
}
