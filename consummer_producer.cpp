// This file defines structures and functions used for multithreaded processing
// of bacteria data, including creation, stochastic computation, and comparison.
// 
// **Structs Overview:**
// 1. `BacteriaProcessingResult`: Holds data related to bacteria processing results.
//    - Members include various maps for tracking values, totals, and complements.
// 
// 2. `ThreadData`: Defines parameters for bacteria comparison across threads.
//    - Includes thread ID, index bounds, and references to bacteria array and matrix.
// 
// 3. `ThreadDataCreator`: Defines parameters for bacteria creation threads.
//    - Includes thread ID and reference to the bacteria array.
// 
// 4. `ThreadDataCreatorWorker`: Handles bacteria creation with work queues.
//    - Includes thread ID, bacteria array, and thread-safe queues.
// 
// 5. `ThreadStochComputeWorker`: Handles stochastic computation for bacteria.
//    - Includes thread ID, bacteria array, and thread-safe queues for task management.
// 
// 6. `ThreadDataComparatorWorker`: Handles bacteria comparison with work queues.
//    - Includes thread ID, bacteria array, a thread-safe queue, and a correlation matrix.
//
// **Functions Overview:**
// - `CREATOR`: Creates bacteria objects across threads, bounded by specific indices.
// - `CREATOR_WORKER`: Dynamically creates bacteria objects using thread-safe queues.
// - `COMPARATOR`: Compares bacteria and updates the correlation matrix.
// - `STOCHASTOR_WORKER`: Performs stochastic computation on bacteria and queues results.
// - `COMPARATOR_WORKER`: Processes comparisons between bacteria using stochastic results.
// - `CompareAllBacteria`: Coordinates the entire workflow, including bacteria creation,
//   stochastic computation, and comparison using multiple threads.

struct BacteriaProcessingResult {
	std::unordered_map<long, long> vector;
	std::unordered_map<long, long> second;
	std::unordered_map<short, long> one_l;
	long total;
	long total_l;
	long complement;
	int index;

	// Constructor to initialize all members
	BacteriaProcessingResult()
		: total(0), total_l(0), complement(0) {}
};

struct ThreadData {
	int threadId;
	int i_start;
	int i_end;
	Bacteria**& b;
	Matrix& m;

	ThreadData(int tid, int start, int end, Bacteria**& bacteriaArray, Matrix& matrix) : threadId(tid), i_start(start), i_end(end), b(bacteriaArray), m(matrix) {}
};

struct ThreadDataCreator {
	int threadId;
	Bacteria**& b;

	ThreadDataCreator(int id, Bacteria**& bacteriaArray)
		: threadId(id), b(bacteriaArray) {}
};

struct ThreadDataCreatorWorker {
	int threadId;
	Bacteria**& b;
	ThreadSafeQueue<int>* queue;
	ThreadSafeQueue<int>* bactQueue;

	ThreadDataCreatorWorker(int id, Bacteria**& bacteriaArray, ThreadSafeQueue<int>* q, ThreadSafeQueue<int>* b)
		: threadId(id), b(bacteriaArray), queue(q), bactQueue(b) {}
};

struct ThreadStochComputeWorker {
	int threadId;
	Bacteria**& b;
	ThreadSafeQueue<int>* bactQueue;
	ThreadSafeQueue<int>* stochQueue;

	ThreadStochComputeWorker(int id, Bacteria**& bacteriaArray, ThreadSafeQueue<int>* b, ThreadSafeQueue<int>* s)
		: threadId(id), b(bacteriaArray), bactQueue(b), stochQueue(s) {}
};

struct ThreadDataComparatorWorker {
	int threadId;
	Bacteria**& b;
	ThreadSafeQueue<int>* stochQueue;
	Matrix& m;

	ThreadDataComparatorWorker(int id, Bacteria**& bacteriaArray, ThreadSafeQueue<int>* b, Matrix& matrix)
		: threadId(id), b(bacteriaArray), stochQueue(b), m(matrix) {}
};

DWORD WINAPI CREATOR(LPVOID lpThreadParameter)
{
	ThreadDataCreator* data = static_cast<ThreadDataCreator*>(lpThreadParameter);
	int threadID = data->threadId;
	Bacteria**& b = data->b;
	int itemsPerThread = (number_bacteria + NUMBER_THREADS_CREATOR - 1) / NUMBER_THREADS_CREATOR;
	int startIndex = itemsPerThread * threadID;
	int endIndex = min(startIndex + itemsPerThread, number_bacteria);
	printf("Bound of bacteria creation: %d to %d\n", startIndex, endIndex);
	for (int j = startIndex; j < endIndex; j++)
	{
		printf("Creating bacteria index: %d on threadId %d\n", j, threadID);
		b[j] = new Bacteria(bacteria_name[j]);
	}
	printf("Creator thread over. %d\n.", threadID);
	return 0;
};

DWORD WINAPI CREATOR_WORKER(LPVOID lpThreadParameter)
{
	ThreadDataCreatorWorker* data = static_cast<ThreadDataCreatorWorker*>(lpThreadParameter);
	int threadID = data->threadId;
	Bacteria**& b = data->b;
	ThreadSafeQueue<int>* initQueue = data->queue;
	ThreadSafeQueue<int>* bacteriaQueue = data->bactQueue;

	while (!initQueue->isEmpty())
	{
		//remove index of the bacteria to process
		int index = initQueue->dequeue();
		printf("Creating bacteria for index: %d on threadId %d\n", index, threadID);
		b[index] = new Bacteria(bacteria_name[index]);
		//once processed, add it to the next for comparations
		bacteriaQueue->enqueue(index);

	}
	return 0;
}

DWORD WINAPI COMPARATOR(LPVOID lpThreatParameter)
{
	//the threads need the leftmost and rightmost i. From this we can compute the bounds
	ThreadData* data = static_cast<ThreadData*>(lpThreatParameter);
	Bacteria**& b = data->b;
	int i_start = data->i_start;
	int i_end = data->i_end;
	Matrix& correlationMatrix = data->m;

	printf("i_start: %d i_end: %d:\n", i_start, i_end);
	for (int i = i_start; i < i_end; i++)
	{
		for (int j = 0; j < i; j++)
		{
			printf("%2d %2d -> ", i, j);
			double correlation = CompareBacteria(b[i], b[j]);
			correlationMatrix.setValue(i, j, correlation);
			printf("%.20lf\n", correlation);
		}
	}
	correlationMatrix.saveMatrix("correlation_matrix.txt");
	return 0;
};

int count = 0;



DWORD WINAPI STOCHASTOR_WORKER(LPVOID lpThreatParameter)
{
	Sleep(5000);
	//worker to process the bacteria with vectors: remove the vectors and compute the stochastic
	ThreadStochComputeWorker* data = static_cast<ThreadStochComputeWorker*>(lpThreatParameter);
	Bacteria**& b = data->b;
	int threadID = data->threadId;
	ThreadSafeQueue<int>* bacteria_queue = data->bactQueue;
	ThreadSafeQueue<int>* stoch_queue = data->stochQueue;
	// Call the compute method on the Bacteria object
	while (!bacteria_queue->isEmpty())
	{
		//remove index of the bacteria to process
		int index = bacteria_queue->dequeue();
		printf("Creating Stochatic Table for index: %d on threadId %d\n", index, threadID);
		b[index]->stochastic_compute();
		//once processed, add it to the next for comparations
		stoch_queue->enqueue(index);

	}
	return 0;
}

DWORD WINAPI COMPARATOR_WORKER(LPVOID lpThreadParameter)
{
	ThreadDataComparatorWorker* data = static_cast<ThreadDataComparatorWorker*>(lpThreadParameter);
	int threadID = data->threadId;
	Bacteria**& b = data->b;
	Matrix& correlationMatix = data->m;
	ThreadSafeQueue<int>* stoch_queue = data->stochQueue;

	while (!stoch_queue->isEmpty())
	{
		int index = stoch_queue->dequeue();
		printf("Comparing with bacteria: %d on threadId %d\n", index, threadID);
		for (int j = 0; j < index; j++)
		{
			printf("%2d %2d -> ", index, j);
			double correlation = CompareBacteria(b[index], b[j]);
			correlationMatix.setValue(index, j, correlation);
			printf("%.20lf\n", correlation);
		}
	}
	return 0;
}


void CompareAllBacteria()
{
	Bacteria** b = new Bacteria * [number_bacteria];
	//The number of THREADS is set earlier	
	HANDLE* handles = new HANDLE[NUMBER_THREADS_CREATOR];

	Matrix correlationMatrix(number_bacteria, number_bacteria);

	DWORD dwEvent;


	ThreadDataCreator** threadDataArray = new ThreadDataCreator * [NUMBER_THREADS_CREATOR];

	for (int thread = 0; thread < NUMBER_THREADS_CREATOR; thread++)
	{
		threadDataArray[thread] = new ThreadDataCreator(thread, b);
		//uses bloc to create the bacteria

		handles[thread] = CreateThread(NULL, 0, CREATOR, threadDataArray[thread], 0, NULL);
		if (handles[thread] == NULL) {
			printf("Error: Could not create thread %d. Error code: %d\n", thread, GetLastError());
			return;  // Or handle the error appropriately
		}
	}

	dwEvent = WaitForMultipleObjects(NUMBER_THREADS_CREATOR, handles, TRUE, INFINITE);
	printf("dwEvent: %lu\n", dwEvent);
	switch (dwEvent) {

	case WAIT_TIMEOUT:
		printf("Wait timed out.\n");
		break;

	case WAIT_FAILED:
		printf("WaitForMultipleObjects failed. Error code: %lu\n", GetLastError());
		break;
	default:
		printf("All threads have completed. \n");
		break;
	}

	// Close event handles

	for (int i = 0; i < NUMBER_THREADS_CREATOR; i++)
	{
		CloseHandle(handles[i]);

	}

	printf("Bacteria Creation Done:");

	int tolal_comparations = combination(number_bacteria, 2);
	printf("tolal_comparaions: %d.\n", tolal_comparations);

	int bloc_size = tolal_comparations / NUMBER_THREADS_COMPARATOR;


	int start = 0;
	int end = 0;
	int partial_count;
	int total_count = 0;
	ThreadData** data = new ThreadData * [NUMBER_THREADS_COMPARATOR];
	printf("Thread data created. bloc size %d  \n", bloc_size);
	for (int thread = 0; thread < NUMBER_THREADS_COMPARATOR; thread++)
	{
		partial_count = 0;
		end = start;
		while (partial_count < bloc_size && total_count < tolal_comparations)
		{
			partial_count += (end);
			total_count += (end);
			end += 1;
			printf("total count:%d, partial_count: %d\n", total_count, partial_count);
		}

		printf("i_end: %d, for thread %d \n", end, thread);
		data[thread] = new ThreadData(thread, start, end, b, correlationMatrix);
		start = end;
	}

	for (int thread = 0; thread < NUMBER_THREADS_COMPARATOR; thread++)
	{
		printf("datathread data: %d %d\n", (void*)data[thread]->i_end, (void*)data[thread]->i_start);
		//uses bloc to create the bacteria
		handles[thread] = CreateThread(NULL, 0, COMPARATOR, data[thread], 0, NULL);
		if (handles[thread] == NULL) {
			printf("Error: Could not create thread %d. Error code: %d\n", thread, GetLastError());
			return;  // Or handle the error appropriately
		}
	}

	dwEvent = WaitForMultipleObjects(NUMBER_THREADS_COMPARATOR, handles, TRUE, INFINITE);

	printf("dwEvent: %lu\n", dwEvent);
	switch (dwEvent) {

	case WAIT_TIMEOUT:
		printf("Wait timed out.\n");
		break;

	case WAIT_FAILED:
		printf("WaitForMultipleObjects failed. Error code: %lu\n", GetLastError());
		break;
	default:
		printf("All threads have completed successfully.\n");
		break;
	}

	for (int thread = 0; thread < NUMBER_THREADS_COMPARATOR; ++thread) {
		CloseHandle(handles[thread]);
	}
	delete[] handles;
};

void CompareAllBacteriaCycle()
{
	Bacteria** b = new Bacteria * [number_bacteria];
	//The number of THREADS is set earlier	
	HANDLE* handles = new HANDLE[NUMBER_THREADS_CREATOR];
	HANDLE* stochHandles = new HANDLE[NUMBER_THREADS_STOCHASTOR];

	Matrix correlationMatrix(number_bacteria, number_bacteria);
	correlationMatrix.createMatrix();
	
	DWORD dwEvent;

	ThreadSafeQueue<int> intQueue(41);
	ThreadSafeQueue<int> bactQueue(41);
	ThreadSafeQueue<int> stochVectorQueue(41);

	for (int i = 0; i < number_bacteria; i++) 
		{
			intQueue.enqueue(i);
		}

	ThreadDataCreatorWorker** threadDataArray = new ThreadDataCreatorWorker * [NUMBER_THREADS_CREATOR];

	ThreadStochComputeWorker ** threadStochArray = new ThreadStochComputeWorker * [NUMBER_THREADS_CREATOR];

	//first file the thread hanler for the creator 
	for (int thread = 0; thread < NUMBER_THREADS_CREATOR; thread++)
	{
		threadDataArray[thread] = new ThreadDataCreatorWorker(thread, b, &intQueue, &bactQueue);
		//uses bloc to create the bacteria

		handles[thread] = CreateThread(NULL, 0, CREATOR_WORKER, threadDataArray[thread], 0, NULL);
		if (handles[thread] == NULL) {
			printf("Error: Could not create thread %d. Error code: %d\n", thread, GetLastError());
			return;  // Or handle the error appropriately
		}
	}



	//next, create the thread handler for the stoch compute
	for (int thread = 0; thread < NUMBER_THREADS_STOCHASTOR; thread++)
	{
		//Create data stru to be passed to the thread
		threadStochArray[thread] = new ThreadStochComputeWorker(thread, b, &bactQueue, &stochVectorQueue);
		
		//Add the handles to the handlers
		stochHandles[thread] = CreateThread(NULL, 0, STOCHASTOR_WORKER, threadStochArray[thread], 0, NULL);
		if (stochHandles[thread] == NULL) {
			printf("Error: Could not create thread %d. Error code: %d\n", thread, GetLastError());
			return;  // Or handle the error appropriately
		}
	}
	
	dwEvent = WaitForMultipleObjects(NUMBER_THREADS_CREATOR, handles, TRUE, INFINITE);
	printf("dwEvent: %lu\n", dwEvent);
	switch (dwEvent) {
	case  WAIT_OBJECT_0:
		printf("All threads have completed successfully.\n");
		break;

	case WAIT_TIMEOUT:
		printf("Wait timed out.\n");
		break;

	case WAIT_FAILED:
		printf("WaitForMultipleObjects failed. Error code: %lu\n", GetLastError());
		break;
	default:
		printf("Unknown event result: %lu\n", dwEvent);
		ExitProcess(0);
	}

	for (int i = 0; i < NUMBER_THREADS_CREATOR; i++)
	{
		CloseHandle(handles[i]);

	}

	dwEvent = WaitForMultipleObjects(NUMBER_THREADS_STOCHASTOR, stochHandles, TRUE, INFINITE);
	// Close event handles
	for (int i = 0; i < NUMBER_THREADS_STOCHASTOR; i++)
	{
		CloseHandle(stochHandles[i]);

	}
	

	printf("Bacteria Creation Done:");


	ThreadDataComparatorWorker** threadDataComparArray = new ThreadDataComparatorWorker * [NUMBER_THREADS_COMPARATOR];
	
	for (int thread = 0; thread < NUMBER_THREADS_COMPARATOR; thread++)
	{
		threadDataComparArray[thread] = new ThreadDataComparatorWorker(thread, b,&stochVectorQueue, correlationMatrix);
	}

	for (int thread = 0; thread < NUMBER_THREADS_COMPARATOR; thread++)
	{
	
		handles[thread] = CreateThread(NULL, 0, COMPARATOR_WORKER, threadDataComparArray[thread], 0, NULL);
		if (handles[thread] == NULL) {
			printf("Error: Could not create thread %d. Error code: %d\n", thread, GetLastError());
			return;  // Or handle the error appropriately
		}
	}

	dwEvent = WaitForMultipleObjects(NUMBER_THREADS_COMPARATOR, handles, TRUE, INFINITE);

	printf("dwEvent: %lu\n", dwEvent);
	switch (dwEvent) {
	case  WAIT_OBJECT_0:
		printf("All threads have completed successfully.\n");
		break;

	case WAIT_TIMEOUT:
		printf("Wait timed out.\n");
		break;

	case WAIT_FAILED:
		printf("WaitForMultipleObjects failed. Error code: %lu\n", GetLastError());
		break;
	default:
		printf("Unknown event result: %lu\n", dwEvent);
		ExitProcess(0);
	}

	for (int thread = 0; thread < NUMBER_THREADS_COMPARATOR; ++thread) {
		CloseHandle(handles[thread]);
	}


	delete[] handles;
};


