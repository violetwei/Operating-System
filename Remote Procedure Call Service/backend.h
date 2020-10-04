#ifndef BACKEND_H_
#define BACKEND_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>


int addInts(int a, int b);

int multiplyInts(int a, int b);

float divideFloats(float a, float b);

int sleepXSeconds(int x);

uint64_t factorial(int x);

#endif