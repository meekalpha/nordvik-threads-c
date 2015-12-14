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

/*number of cars and trucks heading in each direction*/
#define CARS_EAST 3
#define CARS_WEST 3
#define TRUCKS_EAST 2
#define TRUCKS_WEST 2

/*max cars allowed on bridge at any one time*/
#define MAXCARS 3

/*car or truck randomly arrives within first MAXWAIT seconds*/
#define MAXWAIT 6
/*and takes CROSSINGTIME to make is across the bridge*/
#define CROSSINGTIME 4

/* colors to be fancy like the example*/
#define KORG  "\x1B[35m"
#define RESET "\033[0m"

/*binary semaphores*/
int count_lock_east;	/*protects cars_count_east*/
int count_lock_west;	/*protects cars_count_west*/
int bridge_lock;		/*protects the bridge*/

int car_sem_east;		/*makes sure no more than MAXCARS cars on bridge heading east*/
int car_sem_west;		/*makes sure no more than MAXCARS cars on bridge heading west*/

int cars_count_east;	/*num cars currently on bridge heading east*/
int cars_count_west;	/*num cars currently on bridge heading west*/

/*cleanup deletes semaphores at end*/
void cleanup(void){
	rm_sem(count_lock_east);
	rm_sem(count_lock_west);
	rm_sem(bridge_lock);
	rm_sem(car_sem_east);
	rm_sem(car_sem_west);
	exit(1);
}

/*cars heading east*/
void *car_east(void * arg){
	int *me = (int *) arg;	/*id of particular car*/

	sleep( rand() % MAXWAIT );	/*wait for a while before showing up at the bridge*/
	
	P(car_sem_east);	/*proceed when there's less then 3 cars
						heading east on the bridge*/
	P(count_lock_east);	/*lock counter access*/
	cars_count_east++;	/*increment num cars heading east*/
	if(cars_count_east == 1) 	/*I'm the first car heading east on the bridge*/
		P(bridge_lock);			/*proceed when the bridge is free*/
	V(count_lock_east);		/*unlock counter access*/
	/*announce it*/
	printf("Car "KORG"%d"RESET" going east "KORG"on"RESET" the bridge\n", *me );
	/*proceed to cross the bridge*/
	sleep( CROSSINGTIME );
	/*announce it*/
	printf("Car "KORG"%d"RESET" going east "KORG"off"RESET" the bridge\n", *me );

	
	P(count_lock_east);	/*lock counter access*/
	cars_count_east--;	/*decrement counter*/
	if(cars_count_east == 0)	/*if i'm the last car on the bridge*/
		V(bridge_lock);		/*unlock the bridge for other traffic*/
	V(count_lock_east);	/*unlock counter access*/
	V(car_sem_east);	/*exit the bridge*/
	pthread_exit( NULL );	/*exit the thread*/
}
/*cars heading west*/
void *car_west(void * arg){
	int *me = (int *) arg;	/*id of particular car*/

	sleep( rand() % MAXWAIT );	/*wait for a while before showing up at the bridge*/
	
	P(car_sem_west);	/*proceed when there's less then 3 cars
						heading west on the bridge*/
	P(count_lock_west);	/*lock counter access*/
	cars_count_west++;	/*increment num cars heading west*/
	if(cars_count_west == 1) /*I'm the first car heading west on the bridge*/
		P(bridge_lock);	/*proceed when the bridge is free and lock bridge*/
	V(count_lock_west);	/*unlock counter access*/

	/*announce it*/
	printf( "Car "KORG"%d"RESET" going west "KORG"on"RESET" the bridge\n", *me );
	/*proceed to cross the bridge*/
	sleep( CROSSINGTIME );
	/*announce it*/
	printf( "Car "KORG"%d"RESET" going west "KORG"off"RESET" the bridge\n", *me );

	P(count_lock_west);	/*lock counter access*/
	cars_count_west--;	/*decrement counter*/
	if(cars_count_west == 0)	/*if i'm the last car on the bridge*/
		V(bridge_lock);	/*unlock the bridge for other traffic*/
	V(count_lock_west); /*unlock counter access*/
	V(car_sem_west); 	/*exit the bridge*/
	pthread_exit( NULL );	/*exit the thread*/
}
/*trucks heading east*/
void *truck_east(void * arg){
	int *me = (int *) arg; /*id of particular truck*/

	sleep( rand() % MAXWAIT );	/*wait for a while before showing up at the bridge*/

	P(bridge_lock); /*proceed when bridge is free, lock the bridge*/
	/*announce it*/
	printf( "Truck "KORG"%d"RESET" going east "KORG"on"RESET" the bridge\n", *me );
	/*proceed to cross the bridge*/
	sleep( CROSSINGTIME );
	/*announce it*/
	printf( "Truck "KORG"%d"RESET" going east "KORG"off"RESET" the bridge\n", *me );
	V(bridge_lock); /*unlock the bridge*/
	pthread_exit( NULL );	/*exit the thread*/
}
/*trucks heading west*/
void *truck_west(void * arg){
	int *me = (int *) arg; /*id of particular truck*/

	sleep( rand() % MAXWAIT ); /*wait for a while before showing up at the bridge*/

	P(bridge_lock); /*proceed when bridge is free, lock the bridge*/
	/*announce it*/
	printf( "Truck "KORG"%d"RESET" going west "KORG"on"RESET" the bridge\n", *me );
	/*proceed to cross the bridge*/
	sleep( CROSSINGTIME );
	/*announce it*/
	printf( "Truck "KORG"%d"RESET" going west "KORG"off"RESET" the bridge\n", *me );
	V(bridge_lock); /*unlock the bridge*/
	pthread_exit( NULL ); /*exit the thread*/
}

int main(void){
	int i;	/*index*/
	pthread_t car_tid_east[CARS_EAST];	/*thread ids of cars heading east*/
	pthread_t car_tid_west[CARS_WEST];	/*thread ids of cars heading west*/
	pthread_t truck_tid_east[TRUCKS_EAST]; /*thread ids of trucks heading east*/
	pthread_t truck_tid_west[TRUCKS_WEST]; /*thread ids of trucks heading west*/
	int car_id_east[CARS_EAST]; /*car ids of cars heading east*/
	int car_id_west[CARS_WEST]; /*car ids of cars heading west*/
	int truck_id_east[TRUCKS_EAST]; /*truck ids of trucks heading east*/
	int truck_id_west[TRUCKS_WEST]; /*truck ids of trucks heading west*/

	/*initialise no. cars to 0*/
	cars_count_east = 0;
	cars_count_west = 0;

	/*create semaphores*/
	bridge_lock = semtran(IPC_PRIVATE);	
	count_lock_east = semtran(IPC_PRIVATE);
	count_lock_west = semtran(IPC_PRIVATE);
	car_sem_east = semtran(IPC_PRIVATE);
	car_sem_west = semtran(IPC_PRIVATE);

	/*binary semaphores, initalise to 1*/
	V(count_lock_east);
	V(count_lock_west);
	V(bridge_lock);

	/*initialise car semaphores to size MAXCARS*/
	for(i = 0; i < MAXCARS; i++)
		V(car_sem_east);
	for(i = 0; i < MAXCARS; i++)
		V(car_sem_west);

	/*set ids of vehicles*/
	for(i = 0; i < CARS_WEST; i++)
		car_id_west[i] = i;
	for(i = 0; i < CARS_EAST; i++)
		car_id_east[i] = i;
	for(i = 0; i < TRUCKS_EAST; i++)
		truck_id_east[i] = i;
	for(i = 0; i < TRUCKS_WEST; i++)
		truck_id_west[i] = i;

	/* create threads -- remembering to check for errors!*/
	for(i = 0; i < CARS_WEST; i++)
		if(pthread_create( &car_tid_west[i], NULL, car_west, (void *) &car_id_west[i] )){
			perror("Could not create thread");
			exit(EXIT_FAILURE);
		}
	for(i = 0; i < CARS_EAST; i++)
		if(pthread_create( &car_tid_east[i], NULL, car_east, (void *) &car_id_east[i] )){
			perror("Could not create thread");
			exit(EXIT_FAILURE);
		}
	for(i = 0; i < TRUCKS_EAST; i++)
		if(pthread_create( &truck_tid_east[i], NULL, truck_east, (void *) &truck_id_east[i] )){
			perror("Could not create thread");
			exit(EXIT_FAILURE);
		}
	for(i = 0; i < TRUCKS_WEST; i++)
		if(pthread_create( &truck_tid_west[i], NULL, truck_west, (void *) &truck_id_west[i] )){
			perror("Could not create thread");
			exit(EXIT_FAILURE);
		}

	/*join threads*/
	for(i = 0; i < CARS_WEST; i++)
		if(pthread_join( car_tid_west[i], NULL )){
			perror("Thread join failed");
			exit(EXIT_FAILURE);		
		}
	for(i = 0; i < CARS_EAST; i++)
		if(pthread_join( car_tid_east[i], NULL )){
			perror("Thread join failed");
			exit(EXIT_FAILURE);		
		}
	for(i = 0; i < TRUCKS_EAST; i++)
		if(pthread_join( truck_tid_east[i], NULL )){
			perror("Thread join failed");
			exit(EXIT_FAILURE);		
		}
	for(i = 0; i < TRUCKS_WEST; i++)
		if(pthread_join( truck_tid_west[i], NULL )){
			perror("Thread join failed");
			exit(EXIT_FAILURE);		
		}

	/*delete semaphores*/
	cleanup();
	return 0;
	
}
