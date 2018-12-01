
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>

/* Variables for Mersenne Twister */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */
void init_genrand(unsigned long s);
unsigned long genrand_int32(void);

#define NUM_CUSTOMERS 10
#define NUM_CHAIRS 2
sem_t barber_chair;
sem_t barber_sleep;
sem_t waiting_room;
sem_t barber_cape;

pthread_t barber;
pthread_t customers[5];

int chairs_occupied = 0;

void *barberFn(void *arg);
void *customer(void * arg);
void cut_hair();
void get_hair_cut(int num);

int main(){
        sem_init(&barber_cape, 0, 0);
        sem_init(&barber_chair, 0, 1);
        sem_init(&waiting_room, 0, NUM_CHAIRS);
        sem_init(&barber_sleep, 0, 0);

        pthread_create(&barber, NULL, barberFn, NULL);

        for(int i = 0; i < NUM_CUSTOMERS; i++){
                sleep(1); /* Sleep so threads aren't made instantly. Makes output more consistent */
                pthread_create(&customers[i], NULL, customer, (void *)&i);
        }

        pthread_join(barber, NULL);

        for(int i = 0; i < NUM_CUSTOMERS; i++){
                pthread_join(customers[i], NULL);
        }
}

void *barberFn(void *arg){
        while(1){
                printf("Barber is sleeping\n");
                sem_wait(&barber_sleep);  /* If no customers, sleep */

                printf("Barber woke up\n");
                cut_hair();
        }
}
void *customer(void *number){
        int num = *(int *)number;
        printf("Customer[%d] is at the babershop\n", num);

        if(chairs_occupied >= NUM_CHAIRS){
                printf("No seat. Customer[%d] is leaving\n", num);
        }
        else{
        /* Wait for chair */
        sem_wait(&waiting_room);
        chairs_occupied++;

        /* Wait for barber to finish cutting */
        sem_wait(&barber_chair);

        /* Leaving waiting room chair */
        sem_post(&waiting_room);
        chairs_occupied--;

        /* Wake up babrber */
        sem_post(&barber_sleep);

        get_hair_cut(num);

        /* Free up spot in chair */
        sem_post(&barber_chair);
}
}

void cut_hair(){
        int cut_time = genrand_int32() % 10;
        printf("Barber need to wait %d seconds to cut hair\n", cut_time);
        sleep(cut_time);
        sem_post(&barber_cape);
}

void get_hair_cut(int num){
        printf("Customer[%d ]is getting their hair cut\n", num);
        sem_wait(&barber_cape);
}

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}