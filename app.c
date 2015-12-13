#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "buddy.h"
#include <sys/time.h>
#define SPACE 1000
void* allocatedSpace[SPACE+1] = {NULL};
int randomi(int max)
{
    return rand() % (max+1);
}
//size , seed
int main(int argc, char *argv[])
{
    srand(atoi(argv[2]));
    int size = atoi(argv[1]);
	void* chunk = malloc(size*1024);
	int a = binit(chunk,size);
	if (a == -1)
	{
	    printf("-1\n");
	    return 0;
	    
	}
	//printf("%d\n",a);
	int i = 0;
	//Calculate time
	struct timeval start, end;
    long mtime, seconds, useconds;
    //START
    gettimeofday(&start, NULL);
    for (i = 0 ; i < 20000; i++)
	   allocatedSpace[randomi(SPACE)] = balloc((1 << (randomi(7)+9)) + randomi(128));
	
	for (i = 0 ; i < 20000 ; i++)
	{
	    int r = randomi(SPACE);
	    if (allocatedSpace[r] != NULL)
	        bfree(allocatedSpace[r]);
	    allocatedSpace[r] = NULL;
	}
    //END
    gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = (seconds * 1000000 + useconds);	
	printf("%ld\n",mtime);	
	return 0;
}
