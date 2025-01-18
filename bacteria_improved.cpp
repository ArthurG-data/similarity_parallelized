/*
 * Class: BacteriaImprovedParallel
 * Description:
 * This class processes a file containing bacteria data to generate vectors
 * representing specific frequency counts. The process involves initializing
 * vectors, parsing input files, and performing computations to identify
 * significant values based on a stochastic model. The computations are
 * parallelized using OpenMP to improve performance, particularly during
 * the creation of the `t` vector and computation of non-null entries.
 *
 * Key Features:
 * 1. **OpenMP Parallelization**: Utilizes OpenMP to divide the workload of
 *    calculating the stochastic values and vector entries among multiple
 *    threads, significantly reducing execution time.
 * 2. **Dynamic Memory Allocation**: Allocates memory dynamically for vectors
 *    to adapt to varying sizes of input data.
 * 3. **File Parsing**: Reads and parses a bacteria file efficiently, skipping
 *    unnecessary sections and focusing only on relevant data.
 * 4. **Computation of Non-Null Entries**: Identifies and stores positions
 *    of non-null values for further analysis or storage.
 *
 * Note:
 * The class assumes the use of the OpenMP library for parallelization, which
 * needs to be enabled during compilation using appropriate flags (e.g., `-fopenmp`
 * for GCC/Clang).
 */

class BacteriaImproved
{
private:
	long* vector;
	long* second;
	long one_l[AA_NUMBER];
	long indexs;
	long total;
	long total_l;
	long complement;

	void InitVectors()
	{
		vector = new long[M];
		second = new long[M1];

		memset(vector, 0, M * sizeof(long));
		memset(second, 0, M1 * sizeof(long));
		memset(one_l, 0, AA_NUMBER * sizeof(long));
		total = 0;
		total_l = 0;
		complement = 0;
	}

	void init_buffer(char* buffer)
	{
		complement++;
		indexs = 0;
		for (int i = 0; i < LEN - 1; i++)
		{
			short enc = encode(buffer[i]);
			one_l[enc]++;
			total_l++;
			indexs = indexs * AA_NUMBER + enc;
		}
		second[indexs]++;
	}

	void cont_buffer(char ch)
	{
		short enc = encode(ch);
		one_l[enc]++;
		total_l++;
		long index = indexs * AA_NUMBER + enc;
		vector[index]++;
		total++;
		indexs = (indexs % M2) * AA_NUMBER + enc;
		second[indexs]++;
	}

public:
	long count;
	double* tv;
	long* ti;

	BacteriaImproved(char* filename)
	{
		FILE* bacteria_file = fopen(filename, "r");



		if (bacteria_file == NULL)
		{
			fprintf(stderr, "Error: failed to open file %s\n", filename);
			perror("Error opening file");
			exit(1);
		}

		InitVectors();

		char ch;
		while ((ch = fgetc(bacteria_file)) != EOF)
		{

			if (ch == '>')
			{

				while (fgetc(bacteria_file) != '\n'); // skip rest of line

				char buffer[LEN - 1];
				fread(buffer, sizeof(char), LEN - 1, bacteria_file);
				init_buffer(buffer);
			}
			else if (ch != '\n' && ch != '\r')
				cont_buffer(ch);
		}

		long total_plus_complement = total + complement;
		double total_div_2 = total * 0.5;
		double one_l_div_total[AA_NUMBER];
		for (int i = 0; i < AA_NUMBER; i++)
			one_l_div_total[i] = (double)one_l[i] / total_l;

		double* second_div_total = new double[M1];
		for (int i = 0; i < M1; i++)
			second_div_total[i] = (double)second[i] / total_plus_complement;

		count = 0;
		int total_count = 0;
		double* t = new double[M];
		//array for the position of non null array
		long* non_null = new long[M];

#pragma omp parallel num_threads(1) reduction(+:total_count)  shared(t)
		{
			int thread_id = omp_get_thread_num();
			int num_threads = omp_get_num_threads();

			long chunk_size = M / (num_threads);

			long starting_id = chunk_size * (thread_id);
			long end_id = (thread_id == num_threads - 1) ? M : starting_id + chunk_size;
			long i_mod_aa_number = starting_id % (AA_NUMBER);
			long i_div_aa_number = starting_id / (AA_NUMBER);
			long i_mod_M1 = starting_id % M1;
			long i_div_M1 = starting_id / M1;

			for (long i = starting_id; i < end_id; i++)
			{
				double p1 = second_div_total[i_div_aa_number];
				double p2 = one_l_div_total[i_mod_aa_number];
				double p3 = second_div_total[i_mod_M1];
				double p4 = one_l_div_total[i_div_M1];
				double stochastic = (p1 * p2 + p3 * p4) * total_div_2;
				if (i_mod_aa_number == AA_NUMBER - 1)
				{
					i_mod_aa_number = 0;
					i_div_aa_number++;
				}

				else
					i_mod_aa_number++;

				if (i_mod_M1 == M1 - 1)
				{
					i_mod_M1 = 0;
					i_div_M1++;
				}
				else
					i_mod_M1++;

				if (stochastic > EPSILON)
				{
					t[i] = (vector[i] - stochastic) / stochastic;

					total_count++;
				}
				else
					t[i] = 0;
			}

		}
		count = total_count;
		delete second_div_total;
		delete vector;
		delete second;

		tv = new double[count];
		ti = new long[count];

		int pos = 0;

		for (long i = 0; i < M; i++)
		{
			if (t[i] != 0)
			{
				tv[pos] = t[i];
				ti[pos] = i;
				pos++;
			}
		}
		delete t;

		fclose(bacteria_file);
	}
};