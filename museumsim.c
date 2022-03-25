#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "museumsim.h"

struct shared_data {
	// Add any relevant synchronization constructs and shared state here.

	pthread_mutex_t ticket_mutex;	//one mutex is enough...

	pthread_cond_t waiting_visitors;	//visitors waiting to enter(for available guides)
	pthread_cond_t waiting_guides_admit;		//guides waiting for visitors to admit(once there are visitors arriving...)
	pthread_cond_t wait_enter_guides;	//guides waiting to enter (bc of the museum is occupied..?)
	pthread_cond_t finishing_guides;	//guides finishing their tour and waiting to leave

	int tickets;
	int nVisitors;				//num of visitors already inside the museum;
	int nGuides;				//num of guides already inside the museum
	int visitors_waiting_queue;	//once the visitor arrives, it begins to wait
	int pending_admissions;		//flags to indicate if a visitor can really start touring. This is not a boolean flag
	int finished_guides;		//there will have guides finished but not yet left
};

static struct shared_data shared;


/**
 * Set up the shared variables.
 * 
 * `museum_init` will be called before any threads of the simulation
 * are spawned.
 */
void museum_init(int num_guides, int num_visitors)
{
	//init mutexes
	pthread_mutex_init(&shared.ticket_mutex, NULL);

	//init cond vars
	pthread_cond_init(&shared.waiting_visitors, NULL);
	pthread_cond_init(&shared.waiting_guides_admit, NULL);
	pthread_cond_init(&shared.wait_enter_guides, NULL);
	pthread_cond_init(&shared.finishing_guides, NULL);

	//init integers
	shared.tickets = MIN(VISITORS_PER_GUIDE * num_guides, num_visitors);
	shared.nGuides = 0;
	shared.nVisitors = 0;
	shared.visitors_waiting_queue = 0;
	shared.pending_admissions = 0;
	shared.finished_guides = 0;
}


/**
 * Tear down the shared variables.
 * 
 * `museum_destroy` will be called after all threads of the simulation
 * are done executing.
 */
void museum_destroy()
{
	pthread_mutex_destroy(&shared.ticket_mutex);

	pthread_cond_destroy(&shared.waiting_visitors);
	pthread_cond_destroy(&shared.waiting_guides_admit);
	pthread_cond_destroy(&shared.wait_enter_guides);
	pthread_cond_destroy(&shared.finishing_guides);
}


/**
 * Implements the visitor arrival, touring, and leaving sequence.
 */
void visitor(int id)
{
	visitor_arrives(id);

	pthread_mutex_lock(&shared.ticket_mutex);
	{
		//The visitor arrives
		if(shared.tickets == 0)		// if there is no tickets, the visitor leaves
		{			
			pthread_mutex_unlock(&shared.ticket_mutex);		//unlock the ticket mutex before leaving!!
			return;
		}
		else
		{
			shared.tickets--;
			shared.visitors_waiting_queue++;	//the new visitor goes into the waiting queue
			pthread_cond_broadcast(&shared.waiting_guides_admit);		//wake up one of the guides that is waiting for the visitors
		}

		//The visitor waits for a guide
		while(shared.pending_admissions == 0)	//only care about if the guide gives out the flag saying it's available to admit one
		{
			pthread_cond_wait(&shared.waiting_visitors, &shared.ticket_mutex);
		}

		//Touring
		shared.pending_admissions--;		//visitor starts touring and downs the flag
	}
	pthread_mutex_unlock(&shared.ticket_mutex);

	//no need to protect touring
	visitor_tours(id);

	//leave
	pthread_mutex_lock(&shared.ticket_mutex);
	{
		shared.nVisitors--;
		//every time a visitor is gonna leave, wake up all the gudies to let them check if there are any remaining visitors
		pthread_cond_broadcast(&shared.finishing_guides);
		visitor_leaves(id);
	}
	pthread_mutex_unlock(&shared.ticket_mutex);
}

/**
 * Implements the guide arrival, entering, admitting, and leaving sequence.
 */
void guide(int id)
{
	guide_arrives(id);

	pthread_mutex_lock(&shared.ticket_mutex);
	{
		//if there are no visitor waiting and no visitor will come
		if(shared.visitors_waiting_queue == 0 && shared.tickets == 0)
		{	
			//The guide should enter and immediately leave
			guide_enters(id);
			guide_leaves(id);
			pthread_mutex_unlock(&shared.ticket_mutex);
			return;
		}

		while(shared.nGuides == GUIDES_ALLOWED_INSIDE)	//the guides inside are 2 or one of the guides is waiting to leave. 2 gudies as a cohort!
		{
			pthread_cond_wait(&shared.wait_enter_guides, &shared.ticket_mutex);
		}	

		//Guide enters
		shared.nGuides++;
		guide_enters(id);

		//Guide admits
		int visitors_admitted = 0;	//a local var for the guide to keep track of the number of visitors admitted;
		int flag = 0;				//a helper flag helping me to break out of the outer while loop
		while(visitors_admitted < VISITORS_PER_GUIDE)
		{
			if(shared.visitors_waiting_queue == 0 && shared.tickets == 0)	//special case: no need to wait, no admit anymore
				break;
			while(shared.visitors_waiting_queue == 0)
			{
				pthread_cond_wait(&shared.waiting_guides_admit, &shared.ticket_mutex);
				if(shared.visitors_waiting_queue == 0 && shared.tickets == 0)
				{
					flag =  1;
					break;
				}
			}
			if(flag)	//make the code work better if we can break out of the loop earlier
				break;

			
			shared.pending_admissions++;						//give out a adimission flag
			pthread_cond_signal(&shared.waiting_visitors);		//The guide will wake up only one visitor at a time!
			guide_admits(id);

			// guide runs so fast that we should let the guide deal with visitor queue and visitors inside, since the guide need
			// to check these conditions in this while loop
			shared.visitors_waiting_queue--;
			shared.nVisitors++;

			visitors_admitted++;
		}

		//one more finished guide yet not left
		shared.finished_guides++;

		//Waits for all visitors to leave and for any other guide inside to FINISH (not leave)
		while(shared.nVisitors != 0 || shared.nGuides - shared.finished_guides != 0)
		{
			pthread_cond_wait(&shared.finishing_guides, &shared.ticket_mutex);
		}

		//Guide leaves
		guide_leaves(id);
		shared.finished_guides--;	//this finished guide leaves
		shared.nGuides--;
		if(shared.nGuides == 0)		//make sure that it's the last guide that can broadcast the guides waiting to enter
		{
			pthread_cond_broadcast(&shared.wait_enter_guides);
		}
	}
	pthread_mutex_unlock(&shared.ticket_mutex);
}