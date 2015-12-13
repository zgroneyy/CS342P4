#ifndef BUDDY_H
#define BUDDY_H

#define MAX_CHUNK_SIZE 25
#define MIN_CHUNK_SIZE 15

#define MAX_REQUEST_SIZE 16
#define MIN_REQUEST_SIZE 8

int binit(void *, int);
void *balloc(int);
void bfree(void *);
void bprint(void); 

#endif
