#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/sem.h>
#include "sem_ops.h"

#define Readers 5
#define Writers 5

#define MAXREADERS 3

#define MAXWAIT 4

int count_lock;
int data_lock;
int reader_sem;
int readers_count;

void cleanup(void){
	rm_sem(count_lock);
	rm_sem(data_lock);
	printf("end of cleanup\n");
	exit(1);
}

void *reader(void * arg){
	sleep( rand() % MAXWAIT );
	int *me = (int *) arg;
	
	P(reader_sem);
	P(count_lock);
	readers_count++;
	if(readers_count == 1) 
		P(data_lock);
	V(count_lock);

	printf( "Reader %d running!\n", *me );
	sleep( rand() % MAXWAIT );
	printf( "Reader %d ending!\n", *me );

	P(count_lock);
	readers_count--;
	if(readers_count == 0)
		V(data_lock);
	V(count_lock);
	V(reader_sem);
	pthread_exit( NULL );
}

void *writer(void * arg){
	sleep( rand() % MAXWAIT );
	int *me = (int *) arg;
	P(data_lock);
	printf( "Writer %d running!\n", *me );
	sleep( rand() % MAXWAIT );
	printf( "Writer %d ending!\n", *me );
	V(data_lock);
	pthread_exit( NULL );
}

int main(void){
	int i;
	pthread_t reader_tid[Readers];
	pthread_t writer_tid[Writers];
	int reader_id[Readers];
	int writer_id[Writers];

	/*initialise no. readers to 0*/
	readers_count = 0;

	/*create semaphores*/
	data_lock = semtran(IPC_PRIVATE);
	count_lock = semtran(IPC_PRIVATE);
	reader_sem = semtran(IPC_PRIVATE);

	/*binary semaphores*/
	V(count_lock);
	V(data_lock);
	/*sem of size MAXREADERS*/
	for(i = 0; i < MAXREADERS; i++)
		V(reader_sem);

	/*set ids*/
	for(i = 0; i < Readers; i++)
		reader_id[i] = i;
	for(i = 0; i < Writers; i++)
		writer_id[i] = i;

	/* create threads*/
	for(i = 0; i < Readers; i++)
		pthread_create( &reader_tid[i], NULL, reader, (void *) &reader_id[i] );
	for(i = 0; i < Writers; i++)
		pthread_create( &writer_tid[i], NULL, writer, (void *) &writer_id[i] );

	/*join threads*/
	for(i = 0; i < Readers; i++)
		pthread_join( reader_tid[i], NULL );
	for(i = 0; i < Writers; i++)
		pthread_join( writer_tid[i], NULL );

	/*delete semaphores*/
	cleanup();
	return 0;
	
}