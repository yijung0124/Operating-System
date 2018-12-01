#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

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

pthread_t agent;
pthread_t paper_smoker, match_smoker, tobacco_smoker;
pthread_t pusher_a, pusher_b, pusher_c;

pthread_mutex_t mut;
sem_t agent_sem;
sem_t tobacco, paper, match;
sem_t tobacco_sem, paper_sem, match_sem;

bool is_tobacco, is_paper, is_match;

void *agent_func(void *arg);
void *tobacco_smoke(void *arg);
void *paper_smoke(void *arg);
void *match_smoke(void *arg);
void *pusher_a_func(void *arg);
void *pusher_b_func(void *arg);
void *pusher_c_func(void *arg);
void make_cigarette(int n);
void smoke(int n);

int main(){

    is_tobacco = is_paper = is_match = false;

    sem_init(&tobacco, 0, 0);
    sem_init(&paper, 0, 0);
    sem_init(&match, 0, 0);
    sem_init(&agent_sem, 0, 1);

    sem_init(&tobacco_sem, 0, 0);
    sem_init(&paper_sem, 0, 0);
    sem_init(&match_sem, 0, 0);

    pthread_mutex_init(&mut, NULL);

    pthread_create(&match_smoker, NULL, match_smoke, NULL);
    pthread_create(&tobacco_smoker, NULL, tobacco_smoke, NULL);    pthread_create(&paper_smoker, NULL, paper_smoke, NULL);

    pthread_create(&agent, NULL, agent_func, NULL);

    pthread_create(&pusher_a, NULL, pusher_a_func, NULL);
    pthread_create(&pusher_b, NULL, pusher_b_func, NULL);
    pthread_create(&pusher_c, NULL, pusher_c_func, NULL);

    pthread_join(match_smoker, NULL);
    pthread_join(tobacco_smoker, NULL);
    pthread_join(paper_smoker, NULL);
    pthread_join(agent, NULL);
    pthread_join(pusher_a, NULL);
    pthread_join(pusher_b, NULL);
    pthread_join(pusher_c, NULL);

    return 0;
}
/* Thread function for the agent, which randomly does one
   of three things */
void *agent_func(void *arg){
    int agent_num;
    while(1){
        agent_num = (genrand_int32() % 3) + 1;

        sem_wait(&agent_sem);
        if(agent_num == 1){
            printf("Agent[%d]:provide tobacco and paper\n", agent_num);
            sem_post(&tobacco);
            sem_post(&paper);
        }
        else if(agent_num == 2){
            printf("Agent[%d]:provide paper and matches\n", agent_num);
            sem_post(&paper);
            sem_post(&match);
        }
        else if(agent_num == 3){
            printf("Agent[%d]:provide tobacco and matches\n", agent_num);
            sem_post(&tobacco);
            sem_post(&match);
        }
        else{
            printf("<Agent random numebr is error>\n");
        }
    }
}

/* Thread function for smoker with tobacco */
void *tobacco_smoke(void *arg){
    int num = 1;
    printf("Tobacco smoker num : [%d]\n", num);
    while(1){
        sem_wait(&tobacco_sem);
        make_cigarette(num);
        sem_post(&agent_sem);
        smoke(num);
    }
}

/* Thread function for smoker with paper */
void *paper_smoke(void *arg){
    int num = 2;
    printf("Paper smoker num : [%d]\n", num);
    while(1){
        sem_wait(&paper_sem);
        make_cigarette(num);
        sem_post(&agent_sem);
        smoke(num);
    }
}

/* Thread function for smoker with matches */
void *match_smoke(void *arg){
    int num = 3;
    printf("Match smoker num : [%d]\n", num);
    while(1){
        sem_wait(&match_sem);
        make_cigarette(num);
        sem_post(&agent_sem);
        smoke(num);
    }
}

void *pusher_a_func(void *arg){
    while(1){
        sem_wait(&tobacco);
        printf("Pusher <A>\n");
        pthread_mutex_lock(&mut);

        if(is_paper){
            is_paper = false;
            sem_post(&match_sem);
        }
        else if(is_match){
            is_match = false;
            sem_post(&paper_sem);
        }
        else{
            is_tobacco = true;
        }

        pthread_mutex_unlock(&mut);
    }
}

void *pusher_b_func(void *arg){
    while(1){
        sem_wait(&paper);
        printf("Pusher <B>\n");
        pthread_mutex_lock(&mut);

        if(is_tobacco){
            is_tobacco = false;
            sem_post(&match_sem);
        }
        else if(is_match){
            is_match = false;
            sem_post(&tobacco_sem);
        }
        else{
            is_paper = true;
        }

        pthread_mutex_unlock(&mut);
    }
}

void *pusher_c_func(void *arg){
    while(1){
        sem_wait(&match);
        printf("Pusher <C>\n");
        pthread_mutex_lock(&mut);

        if(is_paper){
            is_paper = false;
            sem_post(&tobacco_sem);
        }
        else if(is_tobacco){
            is_tobacco = false;
            sem_post(&paper_sem);
        }
        else{
            is_match = true;
        }

        pthread_mutex_unlock(&mut);
    }
}

void smoke(int n){
    int smoke_time = genrand_int32() % 10;
    printf("Smoker[%d]: smoking\n", n);
    sleep(smoke_time);
}

void make_cigarette(int n){
    int cig_time = genrand_int32() % 10;
    printf("Smoker[%d]: rolling a cigarette\n", n);
    sleep(cig_time);
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