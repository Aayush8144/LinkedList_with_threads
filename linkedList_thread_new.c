#include <stdio.h> // to access i/o 
#include <pthread.h> // c functions for pthread
#include <stdlib.h> // for malloc() and free()

// this program is a linked list adds to the end, deletes from the end with
// threads doing add, delete, and  search 
// we going to use 3 threads in total to do the following task
// th0 -> search
// th1 -> add
// th2 -> deletelast

// lock variable
pthread_mutex_t lock;

// basic node structure
struct nodeStruct
{
	int m_data; // holds data of the node
	struct nodeStruct* m_next; // holds pointer to next Node
};

// a structure that holds data that is shared among thread
struct sharedResourceStruct
{
	struct	nodeStruct* m_headPtr; // points to the head pointer
	int m_data; // data that need to be searched or added
	int m_threadIndex; // hold current thread id
	int m_totalAddThread; // hold the total number of threads that process add function
	int m_totalSearchThread; // hold the total number of threads that process search functino
	int m_totalDeleteThread; // hold the total number of threads that process delete last function
};

// a structre that is created every time a thread is created
// solely created to be passed as argument in pthread_create
// it is destroyed inside the thread, once the sharedResourceStruct is updated
// serving its purpose 
struct threadArgumentStruct
{
	struct sharedResourceStruct* m_sharedResourceStructptr; // pointer to sharedResourceStruct
	int m_data; // data that need to be searched or added
	int m_threadIndex; // hold current thread id
};		

/*
 GetNewNode dynamically allocates memory for a new node, inializes it 
 and returns pointer to that newly created node
 @param a_data: data to set in the node
 @returns a pointer to a new node 
*/
struct nodeStruct* GetNewNode(int a_data);

/*
 UpdateSharedResourceStruct updates the data and threadIndex member variable of the sharedResource structure
 @param a_sharedResourcePtr: pointer to sharedResource structure that is to be updated
 @param a_data: data to be update in the sharedResource structure
 @param a_threadIndex: index to be update in the sharedResource structure
*/
void UpdateSharedResourceStruct(struct sharedResourceStruct* a_sharedResourcePtr,int a_data,int a_threadIndex);

/*
 GetNewThreadArg dynamically allocates memory for a new threadArg
 initializes it and updaes the total threadcount in shared resource struct
  and returns a pointer to newly created threadArg structure
 @param a_data: data to set in the shared resource
 @param a_threadIndex: thread index to set in the shared resource
 @returns a pointer to newly created threadArg structure
 		returns NULL if a invalid threadCall is given
*/
struct threadArgumentStruct* GetNewThreadArg(struct sharedResourceStruct* a_sharedResourceStructptr,int a_data,int a_threadIndex);

/*
 IncreaseTotalAddThread increases total number of Add thread member variable of the sharedResource structure by 1
 @param a_sharedResourcePtr: pointer to sharedResource structure that is to be updated
*/
void IncreaseTotalAddThread(struct sharedResourceStruct* a_sharedResourcePtr) 
{
	a_sharedResourcePtr->m_totalAddThread++;
}

/*
 DecreaseTotalAddThread decreases total number of Add thread member variable of the sharedResource structure by 1
 @param a_sharedResourcePtr: pointer to sharedResource structure that is to be updated
*/
void DecreaseTotalAddThread(struct sharedResourceStruct* a_sharedResourcePtr) 
{
	a_sharedResourcePtr->m_totalAddThread--;
}

/*
 IncreaseTotalSearchThread increases total number of Search thread member variable of the sharedResource structure by 1
 @param a_sharedResourcePtr: pointer to sharedResource structure that is to be updated
*/
void IncreaseTotalSearchThread(struct sharedResourceStruct* a_sharedResourcePtr) 
{
	a_sharedResourcePtr->m_totalSearchThread++;
}

/*
 DecreaseTotalSearchThread decreases total number of Search thread member variable of the sharedResource structure by 1
 @param a_sharedResourcePtr: pointer to sharedResource structure that is to be updated
*/
void DecreaseTotalSearchThread(struct sharedResourceStruct* a_sharedResourcePtr) 
{
	a_sharedResourcePtr->m_totalSearchThread--;
}

/*
 IncreaseTotalDeleteThread increases total number of Delete thread member variable of the sharedResource structure by 1
 @param a_sharedResourcePtr: pointer to sharedResource structure that is to be updated
*/
void IncreaseTotalDeleteThread(struct sharedResourceStruct* a_sharedResourcePtr) 
{
	a_sharedResourcePtr->m_totalDeleteThread++;
}

/*
 DecreaseTotalDeleteThread decreases total number of Delete thread member variable of the sharedResource structure by 1
 @param a_sharedResourcePtr: pointer to sharedResource structure that is to be updated
*/
void DecreaseTotalDeleteThread(struct sharedResourceStruct* a_sharedResourcePtr) 
{
	a_sharedResourcePtr->m_totalDeleteThread--;
}

// a function to print the promt
void PrintPromt();

/// gets user command input 
/// @param userCommandInput: address of variable to store the user commande input
void getUserCommandInput(char* userCommandInput);

/// gets user command input 
/// @param userCommandInput: address of variable to store the user commande input
void getUserDataInput(int* userDataInput);

/*
 Prints the whole list
 @param a_sharedResourcePtr: a pointer to the sharedResource structure
*/
void PrintList(struct sharedResourceStruct* a_sharedResourcePtr);

/* JoinAllAddThreads waits for all the add threads to join
 @param a_sharedResourcePtr: a pointer to the sharedResource structure
 @param addPid: thread ID of the add thread
*/
void JoinAllAddThreads(struct sharedResourceStruct* a_sharedResourcePtr, pthread_t a_addPid);

/* JoinAlltSeacrchThreads waits for all the search threads to join
 @param a_sharedResourcePtr: a pointer to the sharedResource structure
 @param addPid: thread ID of the search thread
*/
void JoinAllSeacrchThreads(struct sharedResourceStruct* a_sharedResourcePtr, pthread_t a_searchPid);

/* JoinAllDeleteLastThreads waits for all the add threads to join
 @param a_sharedResourcePtr: a pointer to the sharedResource structure
 @param addPid: thread ID of the delete last thread
*/
void JoinAllDeleteLastThreads(struct sharedResourceStruct* a_sharedResourcePtr, pthread_t a_deleteLastPid);


/*
 creates and adds a node to the end of list
 @param a_arg: a void pointer to the threadArg structure
 @returns NULL ptr
*/
void* AddToList(void* a_arg);

/*
 Search the data in the list and prints the index of all the instances of them
 @param a_arg: a void pointer to the threadArg structure
 @returns NULL ptr
*/
void* SearchList(void* a_arg);

/*
 delete the node at the end of list 
 waits for other to finish 
 @param a_arg: a void pointer to the threadArg structure
 @returns NULL ptr
*/
void* DeleteLast(void* a_arg);

/*
 delete all the node in the list
 @param a_sharedResourcePtr: a pointer to the sharedResource structure
*/
void DeleteAllNode(struct sharedResourceStruct* a_sharedResourcePtr);

int main()
{
	int i; // for for-loops

	// initializing the lock
	pthread_mutex_init(&lock, NULL);

	// we going to use 3 threads in total
	// th0 -> search
	// th1 -> add
	// th2 -> deletelast
	int numThreads = 3;
	pthread_t tid[numThreads];
	int thIndex[numThreads];
	
	// initializing the index
	for(i = 0; i < numThreads; i++)
	{
		thIndex[i] = i;
	}

	// creating a new threadArg, and sharedResource
	struct threadArgumentStruct* threadArgPtr = NULL;
	// struct nodeStruct* headNode = NULL;
	struct sharedResourceStruct* sharedResourcePtr = (struct sharedResourceStruct*)calloc(1, sizeof(struct sharedResourceStruct));

	// initializing shared ResourcePtr
	sharedResourcePtr->m_headPtr = NULL;
	sharedResourcePtr->m_data = 0;
	sharedResourcePtr->m_threadIndex = 0;
	sharedResourcePtr->m_totalAddThread = 0;
	sharedResourcePtr->m_totalSearchThread = 0;
	sharedResourcePtr->m_totalDeleteThread = 0;
	
	char userCommandInput; // variable to store user input command
	int userDataInput; // variable to store user input data
	int exit = 0; // variable to if to exit or not, 0 = false, 1 = true

	
	// loops until exit
	while(exit == 0)
	{
		PrintPromt(); // printting the promt

		getUserCommandInput(&userCommandInput); // getting command line input

		switch(userCommandInput)
		{
			case '1':
				// 1. Print List
				//printf("Wait for all the thread to finish? (y/n)\n");
				//getUserCommandInput(&userCommandInput); // getting y/n
				
				// printing the list
				PrintList(sharedResourcePtr);
				break;
			
			case '2':
				// 2. Search list
				printf("Enter the data to search:\n");
				getUserDataInput(&userDataInput); // getting user input for data to search

				// beforing deleting last node waiting for all add and delete last threads to finish
				JoinAllAddThreads(sharedResourcePtr, tid[1]);
				JoinAllDeleteLastThreads(sharedResourcePtr, tid[2]);

				// searching for  user input in the list
				threadArgPtr = GetNewThreadArg(sharedResourcePtr, userDataInput, 0);
				pthread_create(&tid[0],NULL,SearchList,(void*) threadArgPtr);
				break;
			
			case '3':
				// 3. Add to the list

				// getting user input for data to search
				printf("Enter the data to Add:\n");
				getUserDataInput(&userDataInput); 
				
				// adding  user input to the list
				threadArgPtr = GetNewThreadArg(sharedResourcePtr, userDataInput, 1);
				pthread_create(&tid[1],NULL,AddToList,(void*) threadArgPtr);
				break;

			case '4':
				// 4. Delete last node of the list
				
				// beforing deleting last node waiting for all add threads to finish
				JoinAllAddThreads(sharedResourcePtr, tid[1]);

				// Deleting Last node of the list
				threadArgPtr = GetNewThreadArg(sharedResourcePtr, 100, 2);
				pthread_create(&tid[2],NULL,DeleteLast,(void*) threadArgPtr);
				break;

			case '5':
				exit = 1;
				printf("Exiting...\n");
				break;

			default:
				printf("Error! Not a Valid Command.\nTry again!\n");
				break;
		}


	}

	// joining all the threads
	JoinAllAddThreads(sharedResourcePtr, tid[1]);
	JoinAllSeacrchThreads(sharedResourcePtr, tid[0]);
	JoinAllDeleteLastThreads(sharedResourcePtr, tid[2]);

	// destroying the list
	DeleteAllNode(sharedResourcePtr);
	
	//destroying the lock
	pthread_mutex_destroy(&lock);

	// freeing the memory used by sharedResource pointer
	free(sharedResourcePtr);

	return 0;
}

// declaration of GetNewNode()
struct nodeStruct* GetNewNode(int a_data)
{
	// dynamically allocating data
	struct nodeStruct* newNode = (struct nodeStruct*)calloc(1, sizeof(struct nodeStruct)); 
	newNode->m_data = a_data; // setting data
	newNode->m_next = NULL; // points to null
	return newNode; 
}

void UpdateSharedResourceStruct(struct sharedResourceStruct* a_sharedResourcePtr,int a_data,int a_threadIndex)
{
	a_sharedResourcePtr->m_data	= a_data;
	a_sharedResourcePtr->m_threadIndex	= a_threadIndex;
}


struct threadArgumentStruct* GetNewThreadArg(struct sharedResourceStruct* a_sharedResourceStructptr,int a_data,int a_threadIndex)
{
	// dynamically allocating data
	struct threadArgumentStruct* newThreadArg = (struct threadArgumentStruct*)calloc(1, sizeof(struct threadArgumentStruct)); 
	
	// inializing the threadArg
	// setting sharedResourcePtr
	newThreadArg->m_sharedResourceStructptr = a_sharedResourceStructptr;
	// setting the data and threadIndex
	newThreadArg->m_data = a_data;
	newThreadArg->m_threadIndex = a_threadIndex;

	// increasing the total thread number according to the index of thread
	// we going to use 3 threads in total
	// th0 -> search
	// th1 -> add
	// th2 -> deletelast
	if(a_threadIndex == 0)
	{
		// this is a search function call
		// increasing the total number of search thread by 1
		IncreaseTotalSearchThread(a_sharedResourceStructptr);
	}
	else if(a_threadIndex == 1)
	{
		// this is a add function call
		// increasing the total number of add thread by 1
		IncreaseTotalAddThread(a_sharedResourceStructptr);
	}
	else if(a_threadIndex == 2)
	{
		// this is a delete last function call
		// increasing the total number of delete thread by 1
		IncreaseTotalDeleteThread(a_sharedResourceStructptr);
	}
	else
	{
		printf("Invlaid! thread Call\n");
		free(newThreadArg);
		return NULL;
	}

	return newThreadArg;
}

void PrintPromt()
{
	printf("\n");
	printf("Press the corresponding number to perform the said action\n");
	printf("\t1. Print list\n");
	printf("\t2. Search list\n");
	printf("\t3. Add to the list\n");
	printf("\t4. Delete last item from the list\n");
	printf("\t5. Exit\n");
}

void getUserCommandInput(char* userCommandInput)
{
	*userCommandInput = getchar(); // getting user command
	char getExtraNewLineChar = getchar(); // getting the extra '\n' pushed by getchar() into buffer

}

void getUserDataInput(int* userDataInput)
{
	scanf("%d", userDataInput);
	char getExtraNewLineChar = getchar(); // getting the extra '\n' pushed by getchar() into buffer

}

void PrintList(struct sharedResourceStruct* a_sharedResourcePtr)
{
	// checking a_sharedResourcePtr is not NULL
	if(a_sharedResourcePtr == NULL)
	{
			printf("Error! Pass a pointer to sharedResource as an argument\n");
	}

	// getting the head node from the shared Resource
	struct nodeStruct* tempNode = a_sharedResourcePtr->m_headPtr;

	// checking to see if the Head is null
	if(tempNode	== NULL)
	{
		// list is empty 
		printf("The list is empty!\n");
	}
	else
	{
		// head is not empty
		printf("List:\n   ");
		
		// transvering the list while printing hte data 
		while(tempNode->m_next	!= NULL)
		{
			printf("%d->",tempNode->m_data);
		tempNode = tempNode->m_next; // getting next node
		}
		
		printf("%d\n",tempNode->m_data);
	}
}

void JoinAllAddThreads(struct sharedResourceStruct* a_sharedResourcePtr, pthread_t a_addPid)
{
	int totalAddThreads = a_sharedResourcePtr->m_totalAddThread;

	for(; totalAddThreads == 0; totalAddThreads--)
	{
		pthread_join(a_addPid,NULL);
		DecreaseTotalAddThread(a_sharedResourcePtr);
	}
}

void JoinAllSeacrchThreads(struct sharedResourceStruct* a_sharedResourcePtr, pthread_t a_searchPid)
{
	int totalAddThreads = a_sharedResourcePtr->m_totalSearchThread;

	for(; totalAddThreads == 0; totalAddThreads--)
	{
		pthread_join(a_searchPid, NULL);
		DecreaseTotalSearchThread(a_sharedResourcePtr);
	}
}

void JoinAllDeleteLastThreads(struct sharedResourceStruct* a_sharedResourcePtr, pthread_t a_deleteLastPid)
{
	int totalAddThreads = a_sharedResourcePtr->m_totalSearchThread;

	for(; totalAddThreads == 0; totalAddThreads--)
	{
		pthread_join(a_deleteLastPid,NULL);
		DecreaseTotalDeleteThread(a_sharedResourcePtr);
	}
}

void* AddToList(void* a_arg)
{
	// checking a_arg is not NULL
	if(a_arg == NULL)
	{
		printf("Invlaid! thread Call\n");
		return NULL;
	}

	// a_arg orginally is threadArgStruct
	struct threadArgumentStruct* threadArgPtr = (struct threadArgumentStruct*) a_arg;

	// this is the start of critical section
	// so we lock the shared resources here
	pthread_mutex_lock(&lock);

	// accessig the sharedResourePtr
	struct sharedResourceStruct* sharedResourcePtr = threadArgPtr->m_sharedResourceStructptr;

	// updating the sharedResourse struct
	UpdateSharedResourceStruct(sharedResourcePtr, threadArgPtr->m_data,threadArgPtr->m_threadIndex);

	printf("Thread %d: adding %d to the end of the list\n", 
		sharedResourcePtr->m_threadIndex, sharedResourcePtr->m_data);

	// creating a new node to add to the list
	struct nodeStruct* newNode = GetNewNode(sharedResourcePtr->m_data);

	// adding new node to the list
	// getting the head node from the shared Resource
	struct nodeStruct* tempNode = sharedResourcePtr->m_headPtr;

	// checking to see if the Head is null
	if(tempNode	== NULL)
	{
		// list is empty 
		// adding newNode as head
		sharedResourcePtr->m_headPtr = newNode;

	}
	else
	{
		// head is not empty
		// transvering the list
		while(tempNode->m_next	!= NULL)
		{
			tempNode = tempNode->m_next; // getting next node
		}

		// addding newNode to the end
		tempNode->m_next = newNode;
	}

	// freeing the threadArgStruct pointer
	free(threadArgPtr);

	printf("Thread %d: complete\n",	sharedResourcePtr->m_threadIndex);

	// end of critical section
	// unlocking the shared resources
	pthread_mutex_unlock(&lock);

	pthread_exit(NULL);
}

void* SearchList(void* a_arg)
{
	// checking a_arg is not NULL
	if(a_arg == NULL)
	{
		printf("Invlaid! thread Call\n");
		return NULL;
	}

	// a_arg orginally is threadArgStruct
	struct threadArgumentStruct* threadArgPtr = (struct threadArgumentStruct*) a_arg;

	// this is the start of critical section
	// so we lock the shared resources here
	pthread_mutex_lock(&lock);

	// accessig the sharedResourePtr
	struct sharedResourceStruct* sharedResourcePtr = threadArgPtr->m_sharedResourceStructptr;

	// updating the sharedResourse struct
	UpdateSharedResourceStruct(sharedResourcePtr, threadArgPtr->m_data, threadArgPtr->m_threadIndex);

	printf("Thread %d: searching %d in the list\n", 
		sharedResourcePtr->m_threadIndex, sharedResourcePtr->m_data);

	// getting the head node from the shared Resource
	struct nodeStruct* tempNode = sharedResourcePtr->m_headPtr;
	int dataToFind = sharedResourcePtr->m_data; // storing the data to find
	int index = 0;
	int dataFound = 0; // a variable to check if the data was ever found or not, 0 = not found, 1 = found
	
	// head is not empty
	// transvering the list and checking the data in each node
	while(tempNode->m_next	!= NULL)
	{
		if(tempNode->m_data == dataToFind)
		{
			// data found
			dataFound = 1; // updating found
			printf("\tData Found at index: %d\n", index); // printing the index
		}

		index++; // updating index
		tempNode = tempNode->m_next; // getting next node
	}

	// checking last node
	if(tempNode->m_data == dataToFind)
	{
		// data found
		dataFound = 1; // updating found
		printf("\tData Found at index: %d\n", index); // printing the index
	}

	// checking if there were no instance of the data in the list
	if (dataFound == 0)
	{
		printf("\tData not found in the list.\n");
	}

	// freeing the threadArgStruct pointer
	free(threadArgPtr);

	printf("Thread %d: complete\n",	sharedResourcePtr->m_threadIndex);

	// end of critical section
	// unlocking the shared resources
	pthread_mutex_unlock(&lock);

	pthread_exit(NULL);
}

void* DeleteLast(void* a_arg)
{
		// checking a_arg is not NULL
	if(a_arg == NULL)
	{
		printf("Invlaid! thread Call\n");
		return NULL;
	}

	// a_arg orginally is threadArgStruct
	struct threadArgumentStruct* threadArgPtr = (struct threadArgumentStruct*) a_arg;

	// this is the start of critical section
	// so we lock the shared resources here
	pthread_mutex_lock(&lock);

	// accessig the sharedResourePtr
	struct sharedResourceStruct* sharedResourcePtr = threadArgPtr->m_sharedResourceStructptr;

	// updating the sharedResourse struct
	UpdateSharedResourceStruct(sharedResourcePtr, threadArgPtr->m_data,threadArgPtr->m_threadIndex);

	printf("Thread %d: deleting Last node of the list\n", sharedResourcePtr->m_threadIndex);

	// getting the head node from the shared Resource
	struct nodeStruct* tempNode = sharedResourcePtr->m_headPtr;
	struct nodeStruct* preTempNode = NULL;

	// checking to see if the Head is null
	if(tempNode	== NULL)
	{
		// list is empty 
		printf("\tThe list is empty!\n");
	}
	else
	{
		// head is not empty
		// transvering the list
		while(tempNode->m_next	!= NULL)
		{
			preTempNode = tempNode;
			tempNode = tempNode->m_next; // getting next node
		}

		// tempNode points to the end
		preTempNode->m_next = NULL; // updating second last node to point to NULL
		free(tempNode); // deleting last node
	}

	// freeing the threadArgStruct pointer
	free(threadArgPtr);

	printf("Thread %d: complete\n",	sharedResourcePtr->m_threadIndex);

	// end of critical section
	// unlocking the shared resources
	pthread_mutex_unlock(&lock);

	pthread_exit(NULL);
}

void DeleteAllNode(struct sharedResourceStruct* a_sharedResourcePtr)
{
	// checking a_arg is not NULL
	if(a_sharedResourcePtr == NULL)
	{
		printf("Invlaid! pass a pointer of the sharedResource as an argument\n");
	}


	printf("Deleting all the nodes in the list\n");

	// getting the head node from the shared Resource
	struct nodeStruct* listHead = a_sharedResourcePtr->m_headPtr;
	struct nodeStruct* currHead = NULL;

	// checking to see if the Head is null
	if (listHead == NULL)
	{
		// list is empty 
		printf("\tThe list is empty!\n");
	}
	else
	{
		// head is not empty
		// transvering the list
		while (listHead->m_next	!= NULL)
		{
			currHead = listHead;
		 	listHead = listHead->m_next; // updating currHeadPtr to next node
			free(currHead); // deleting curr head node
		}

		// listHead points to the end
		free (listHead); // deleting last node

		a_sharedResourcePtr->m_headPtr = NULL; // updating the m_headPtr to point to NULL
	}
}