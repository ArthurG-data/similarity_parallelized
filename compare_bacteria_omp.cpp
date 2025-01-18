
/*
This program demonstrates the parallelized loading and comparison of bacterial data using OpenMP.

1. **Loading Bacteria Data:**
   - The `BacteriaImprovedBasic` objects are created and initialized in parallel.
   - OpenMP is used with the `#pragma omp parallel` directive to divide the work of loading bacteria among multiple threads.
   - Dynamic scheduling ensures balanced workload distribution even if loading times vary.

2. **Preparing Comparison Pairs:**
   - The `Pair` struct stores indices of bacterial pairs to compare.
   - A single thread generates all comparison pairs and stores them in an array.

3. **Comparing Bacteria:**
   - The comparison of bacterial pairs is parallelized using OpenMP.
   - Each thread processes a subset of pairs, with dynamic scheduling to balance workloads.
   - The `CompareBacteriaBasic` function calculates the correlation between two bacterial datasets.

4. **Key OpenMP Features:**
   - `#pragma omp parallel`: Creates a team of threads for concurrent execution.
   - `#pragma omp for schedule(dynamic)`: Distributes loop iterations dynamically among threads to handle varying workloads.
   - `num_threads`: Specifies the number of threads to use in parallel regions.
*/

int number_bacteria;
char** bacteria_name;
long M, M1, M2;
short code[27] = { 0, 2, 1, 2, 3, 4, 5, 6, 7, -1, 8, 9, 10, 11, -1, 12, 13, 14, 15, 16, 1, 17, 18, 5, 19, 3 };
#define encode(ch)		code[ch-'A']
#define LEN				6
#define AA_NUMBER		20
#define	EPSILON			1e-010
#define NUMBER_THREADS 10
#define OMP_NESTED TRUE

void Init()
{
	M2 = 1;
	for (int i = 0; i < LEN - 2; i++)	// M2 = AA_NUMBER ^ (LEN-2);
		M2 *= AA_NUMBER;
	M1 = M2 * AA_NUMBER;		// M1 = AA_NUMBER ^ (LEN-1);
	M = M1 * AA_NUMBER;			// M  = AA_NUMBER ^ (LEN);
}

double CompareBacteriaBasic(BacteriaImprovedBasic* b1, BacteriaImprovedBasic* b2)
{
	double correlation = 0;
	double vector_len1 = 0;
	double vector_len2 = 0;
	long p1 = 0;
	long p2 = 0;
	while (p1 < b1->count && p2 < b2->count)
	{
		long n1 = b1->ti[p1];
		long n2 = b2->ti[p2];
		if (n1 < n2)
		{
			double t1 = b1->tv[p1];
			vector_len1 += (t1 * t1);
			p1++;
		}
		else if (n2 < n1)
		{
			double t2 = b2->tv[p2];
			p2++;
			vector_len2 += (t2 * t2);
		}
		else
		{
			double t1 = b1->tv[p1++];
			double t2 = b2->tv[p2++];
			vector_len1 += (t1 * t1);
			vector_len2 += (t2 * t2);
			correlation += t1 * t2;

		}
	}
	while (p1 < b1->count)
	{
		long n1 = b1->ti[p1];
		double t1 = b1->tv[p1++];
		vector_len1 += (t1 * t1);
	}
	while (p2 < b2->count)
	{
		long n2 = b2->ti[p2];
		double t2 = b2->tv[p2++];
		vector_len2 += (t2 * t2);
	}

	return correlation / (sqrt(vector_len1) * sqrt(vector_len2));
}

struct Pair {
	int i;
	int j;
};


void CompareAllBacteriaImproved()
{

	BacteriaImprovedBasic** b = new BacteriaImprovedBasic * [number_bacteria];


#pragma omp parallel num_threads(15)  // Set the number of threads to 4
	{
#pragma omp for schedule(dynamic) 
		for (int i = 0; i < number_bacteria; i++)
		{
			int thread_id = omp_get_thread_num();
			int num_threads = omp_get_num_threads();
			printf("load %d of %d  \n", i + 1, number_bacteria);
			b[i] = new BacteriaImprovedBasic(bacteria_name[i]);

		}

	}
	int total_comparisons = (number_bacteria * (number_bacteria - 1)) / 2;
	Pair* comparisons = new Pair[total_comparisons];
	//fill the pairs
	int location = 0;
	for (int i = 0; i < number_bacteria - 1; i++) {
		for (int j = i + 1; j < number_bacteria; j++) {
			comparisons[location].i = i;
			comparisons[location].j = j;
			location++;

		}
	}
#pragma omp parallel num_threads(10)
	{
		int thread_id = omp_get_thread_num();
#pragma omp for schedule(dynamic) 
		for (int k = 0; k < total_comparisons; k++) {
			int i = comparisons[k].i;
			int j = comparisons[k].j;
			// Example comparison function (replace with actual function)
			double correlation = CompareBacteriaBasic(b[i], b[j]);
			printf("%03d %03d -> %.10lf on thread %d\n", i, j, correlation, thread_id);
		}
	}
}

void ReadInputFile(const char* input_name)
{
	FILE* input_file = fopen(input_name, "r");


	if (input_file == nullptr)
	{
		fprintf(stderr, "Error: failed to open file %s (Hint: check your working directory)\n", input_name);
		exit(1);
	}

	if (fscanf(input_file, "%d", &number_bacteria) != 1) {
		fprintf(stderr, "Error: failed to read the number of bacteria from %s\n", input_name);
		fclose(input_file);
		exit(1);
	}

	bacteria_name = new char* [number_bacteria];

	for (long i = 0; i < number_bacteria; i++)
	{
		char name[10];
		if (fscanf(input_file, "%9s", name) != 1) {
			fprintf(stderr, "Error: failed to read the bacteria name at index %ld\n", i);
			fclose(input_file);
			exit(1);
		}
		bacteria_name[i] = new char[20];
		snprintf(bacteria_name[i], 20, "data/%s.faa", name);
	}
	fclose(input_file);
}

int main(int argc, char* argv[])
{
	time_t t1 = time(NULL);

	Init();
	ReadInputFile("list.txt");
	CompareAllBacteriaImproved();

	time_t t2 = time(NULL);
	printf("time elapsed: %ld seconds\n", t2 - t1);
	return 0;
}