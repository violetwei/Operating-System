/* Includes */
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

	// Test 3: Worst Fit Test
    puts("Test 3: Check for Worst Fit algorithm...");
	// Sets Policy to Worst Fit
	sma_mallopt(WORST_FIT);

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

	int *cp2 = (int *)sma_malloc(16 * 1024 * 2);

	// Testing if the correct hole has been allocated
	if (cp2 != NULL)
	{
		if (cp2 == c2[27] || cp2 == c2[28] || cp2 == c2[29] || cp2 == c2[30]) {
			sprintf(str, "CP2: %p", cp2);
			puts(str);
			sprintf(str, "C2[27]: %p", c2[27]);
			puts(str);
			sprintf(str, "C2[28]: %p", c2[28]);
			puts(str);
			sprintf(str, "C2[29]: %p", c2[29]);
			puts(str);
			sprintf(str, "C2[30]: %p", c2[30]);
			puts(str);
			puts("\t\t\t\t PASSED\n");
		} else {
			puts("\t\t\t\t FAILED\n");
		}
	}
	else
	{
		puts("\t\t\t\t FAILED\n");
	}

	//	Freeing cp2
	sma_free(cp2);

	return (0);
}