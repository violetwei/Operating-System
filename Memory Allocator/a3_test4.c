#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sma.h"

int main(int argc, char *argv[])
{
	int i, count = 0;
	void *ptr, *limitafter = NULL, *limitbefore = NULL;
	char *c[32], *ct;
	int *c2[32];
	char str[60];

	// Test 4: Next Fit Test
	puts("Test 4: Check for Next Fit algorithm...");
	// Sets Policy to Next Fit
	sma_mallopt(NEXT_FIT);

    // Allocating 512 kbytes of memory..
	for (i = 0; i < 32; i++) {
		c2[i] = (int *)sma_malloc(16 * 1024);
	}

	// Now deallocating some of the slots ..to free
	// One chunk of 5x16 kbytes
	sma_free(c2[31]);
	sma_free(c2[30]);
	sma_free(c2[29]);
	sma_free(c2[28]);
	sma_free(c2[27]);

	// One chunk of 3x16 kbytes
	sma_free(c2[25]);
	sma_free(c2[24]);
	sma_free(c2[23]);

	// One chunk of 2x16 kbytes
	sma_free(c2[20]);
	sma_free(c2[19]);

	// One chunk of 3x16 kbytes
	sma_free(c2[10]);
	sma_free(c2[9]);
	sma_free(c2[8]);

	// One chunk of 2x16 kbytes
	sma_free(c2[5]);
	sma_free(c2[4]);

	int *cp3 = (int *)sma_malloc(16 * 1024 * 3);
	int *cp4 = (int *)sma_malloc(16 * 1024 * 2);

	// Testing if the correct holes have been allocated
	if (cp3 == c2[8] && cp3 != NULL)
	{
		if (cp4 == c2[19])
		{
			sprintf(str, "C2[19]: %p", c2[19]);
			puts(str);
			sprintf(str, "CP4: %p", cp4);
			puts(str);
			puts("\t\t\t\t PASSED\n");
		}
		else
		{
			puts("\t\t\t\t FAILED\n");
		}
	}
	else
	{
		puts("\t\t\t\t FAILED\n");
	}
	
	return (0);
}