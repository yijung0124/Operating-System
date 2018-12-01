#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Linked list struct.
struct node{
		int data;
		struct node *next;
} *head;

pthread_t searcher[3];
pthread_t deleter[3];
pthread_t inserter[3];

pthread_mutex_t insert_mutex;
pthread_mutex_t delete_mutex;

sem_t delete_insert_lock;
sem_t delete_search_lock;


void *search(void *arg);
void *delete(void *arg);
void *insert(void *arg);
void add(int num);
void append(int num);
void interrupt_handler();
void print_list(struct node *linked_list);

// Generate random number

void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);
unsigned long genrand_int32(void);
long genrand_int31(void);
double genrand_real1(void);
double genrand_real2(void);
double genrand_real3(void);
double genrand_res53(void);



/* Main Function */
int main(){
	int i;
	
	// Signal catch
	signal(SIGINT, interrupt_handler);
	
	// Initialize mutexes.
	pthread_mutex_init(&insert_mutex, NULL);
    pthread_mutex_init(&delete_mutex, NULL);

	// Initilize semaphores.
    sem_init(&delete_insert_lock, 0, 1);
    sem_init(&delete_search_lock, 0, 1);

	// Create threads.
	for(i = 0; i < 3; i++){
		pthread_create(&searcher[i], NULL, search, NULL);
		pthread_create(&inserter[i], NULL, insert, NULL);
		pthread_create(&deleter[i], NULL, delete, NULL);
	}

	// Join threads.
	for(i = 0; i < 3; i++){
		pthread_join(inserter[i], NULL);
		pthread_join(searcher[i], NULL);
		pthread_join(deleter[i], NULL);
	}
}

/* searches for random number */
/* searcher can execute concurrently with each other */
void *search(void *arg){
	int num;
	int flag = 1, count = 0;
	struct node *temp;
	
	while(1){
		// Generate random number.
		num = genrand_int32() %69;	
		
		// Wait for delete search lock
		sem_wait(&delete_search_lock);
		printf("(SID):%lld  >> Deletes : blocked\n", pthread_self());
		
		temp=head;
		
		printf("(SID):%lld  >> Searching : %d\n", pthread_self(), num);

		// Check if list is empty, else find and print number.
		if(temp==NULL){
			printf("(SID):%lld  >> No %d in Linked list\n", pthread_self(), num);
		}else{
			// Check to see if num is in list
			while(temp != NULL){
				if(temp->data != num){
					temp=temp->next;
					++count;
				}else{
					// We found the num
					printf("(SID):%lld  >> Index #%d Found %d\n ", pthread_self(), count, num);
					print_list(head);
					count = 0;
					flag = 0;
					break;
				}
			}
			
			// If number is not in the list...
			if (flag != 0){
				printf("(SID):%lld  >> No %d in linked list\n", pthread_self(), num);
				print_list(head);
			}
		}
		
		// Unlock the delete search lock
		sem_post(&delete_search_lock);
		printf("(SID):%lld  >> Deletes : unblocked\n", pthread_self());
		sleep(5);
	}
}

/* Deletes random index */
/* At most one deleter process can access the list at a time */
/* deletion must also be mutually exclusive with searches and insertions */
void *delete(void *arg){
	struct node *temp, *prev;
	int count, i, del_val;

	while(1){
		// Wait for the two locks.
		sem_wait(&delete_insert_lock);
		printf("(DID):%lld  >> Inserts : blocked\n", pthread_self());
		sem_wait(&delete_search_lock);
		printf("(DID):%lld  >> Searchs : blocked\n", pthread_self());
		
		// Lock the delete mutex.
		pthread_mutex_lock(&delete_mutex);
		
		temp=head;
		
		// Get the size of the linked list.
		count = 0;
		while(temp != NULL){
			++count;
			temp = temp->next;
		}

		// Pick a random index.
		del_val = genrand_int32() % count;
		
		// Point to the right spot in the list.
		temp = head;
		for(i = 0; i < del_val; i++){
			prev = temp;
			temp = temp->next;
		}
		
		// Delete accordingly.
		if(temp==head){
			head=temp->next;
			free(temp);
			printf("(DID):%lld  >> Number is deleted at Indext #%d.\n", pthread_self(), del_val);
			print_list(head);
		}else{
			prev->next=temp->next;
			free(temp);
			printf("(DID):%lld  >> Number is deleted at Indext #%d.\n", pthread_self(), del_val);
			print_list(head);
		}
		

		
		// Unlock the insert and search semaphores
		sem_post(&delete_insert_lock);
		printf("(DID):%lld  >> Inserts : unblocked.\n", pthread_self());
        sem_post(&delete_search_lock);
        printf("(DID):%lld  >> Searchs : unblocked.\n", pthread_self());

		// Unlock the delete mutex
		pthread_mutex_unlock(&delete_mutex);

		sleep(6);
	}
}


/* Inserts value at the end of the list */
/* insertions must be mutually exclusive to preclude two inserters from inserting new items at about the same time*/
void *insert(void *arg){
	int num;
	struct node *temp;
	
	while(1){
		// Wait for the delete insert lock semaphore
		sem_wait(&delete_insert_lock);
		printf("(IID):%lld  >> One Insert : unblocked.\n", pthread_self());

		// Lock the insert mutex.
		pthread_mutex_lock(&insert_mutex);
		temp=head;
		
		// If list is empty, generate a list with a random value.
		if(temp==NULL){
			num = genrand_int32() %69;	
			add(num);
			printf("(IID):%lld >> Inserted into head of list with value %d\n", pthread_self(), num);
		}else{
			// Create random num and check if its in list
			num = genrand_int32() %69;
			while(temp != NULL){
				if(temp->data != num){
					temp=temp->next;
				}else{
					// we already have this value...
					num = genrand_int32() %69;
					temp=head;
				}
			}
			
			// Find end of list...
			temp=head;
			while(temp->next!=NULL){
				temp=temp->next;
			}
			// And add item to end of linked list
			append(num);
			printf("(IID):%lld >> Inserted into end of list with value %d\n", pthread_self(), num);
		}
		// Print list.
		print_list(head);
		
		// Unlock delete insert lock semaphore.
		sem_post(&delete_insert_lock);
		printf("(IID):%lld >> The other 2 Inserts : unblocked.\n", pthread_self());
		// Unlock the insert mutex.
        pthread_mutex_unlock(&insert_mutex);
		sleep(5);
	}
}

/* Create a new linked list with value num. */
void add(int num){
    struct node *temp;
    temp=(struct node *)malloc(sizeof(struct node));
    temp->data=num;
	
    if(head== NULL){
		head=temp;
		head->next=NULL;
    }else{
		temp->next=head;
		head=temp;
    }
}

/* Add value num at the end of linked list. */
void append(int num){
    struct node *temp,*right;
    temp= (struct node *)malloc(sizeof(struct node));
    temp->data=num;
	
    right=(struct node *)head;
	
    while(right->next != NULL){
		right=right->next;
	}
	
    right->next =temp;
    right=temp;
    right->next=NULL;
}


/* Print linked list */
void print_list(struct node *linked_list){
	int print_val;
	
	printf("Current Linked list: ");
	
	if(linked_list == NULL){
		printf("Linked list is empty\n");
	}else{
		while(linked_list != NULL){
			print_val = linked_list->data;
			printf("%d ", print_val);
			linked_list = linked_list->next;
		}
		printf("\n");
	}
}

/* interrupt_handler() function */
void interrupt_handler(){
	int i;
	
	for(i = 0; i < 3; i++){
		pthread_detach(inserter[i]);
		pthread_detach(searcher[i]);
		pthread_detach(deleter[i]);
	}

	exit(EXIT_SUCCESS);
}

/* Create random number */
/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

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

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
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

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
    return (long)(genrand_int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
    return genrand_int32()*(1.0/4294967295.0); 
    /* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
    return genrand_int32()*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
    return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void) 
{ 
    unsigned long a=genrand_int32()>>5, b=genrand_int32()>>6; 
    return(a*67108864.0+b)*(1.0/9007199254740992.0); 
} 
/* These real versions are due to Isaku Wada, 2002/01/09 added */
