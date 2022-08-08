#include <stdio.h>
#include <stdlib.h>
#include <math.h>


//Just to waste more time in computation than a single add operation
double waste_time(double n)
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
	FILE *inp_a, *inp_b, *out_c;
	//reading the vector size from command line argument
	long int size = atol(argv[4]);
	long int i;
	double *a, *b, *c;

	//vector allocation
	a = (double *) malloc(sizeof(double) * size);
	b = (double *) malloc(sizeof(double) * size);
	c = (double *) malloc(sizeof(double) * size);

	//opening files (2 input and 1 output) - names passed as command line arguments
	inp_a = fopen(argv[1], "r");
	inp_b = fopen(argv[2], "r");
	out_c = fopen(argv[3], "w");
	
	for (i=0; i<size; i++) 
	{
		//reading input values to vectors A and B
		fscanf(inp_a, "%lf\n", &a[i]);
		fscanf(inp_b, "%lf\n", &b[i]);

		//computing vector C
		//doing computation intesive stuff - not just a sum
		c[i]=waste_time(abs(a[i]))+waste_time(abs(b[i]));
		//c[i] = a[i] + b[i];

		//writing C to the output file
		fprintf(out_c, "%lf\n", c[i]);
	}
	
	//closing files
	fclose(inp_a);
	fclose(inp_b);
	fclose(out_c);

	//freeing memory
	free(a);
	free(b);
	free(c);
	return 0;
}
