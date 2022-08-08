#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h> 

void *pura(void *args);


int main(int argc, char **argv)
{
	int status;
	long int lim = atol(argv[1]);
	pthread_t tid;
	pthread_attr_t attr;
	void * my_args;
	
	long int i;
	
	for(i=0; i<lim; i++)
	{

		pthread_attr_init(&attr);
		pthread_create(&tid, &attr, pura, my_args);
		pthread_join(tid,NULL);
	}
	return 0;
}


void *pura(void *args)
{
	printf("%ld\n",pthread_self());
}

