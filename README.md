[img](assets/img/banner.webp)

# Bacteria Comparison Using k-mer Frequencies

## Introduction

This application aims to compare bacteria based on protein frequency sequences. It uses **k-mer amino acids frequency counts**, where the complexity of the problem scales exponentially with the value of `k` (i.e., `20^k`). A phylogenetic tree is created to represent the evolutionary proximity and relationships of bacteria based on their proteome similarity.

With billions of bacterial species on Earth and proteome sizes ranging from 500 (e.g., *Mycoplasma*) to 9000 proteins (e.g., *Pseudomonas*), efficiently processing and comparing such large datasets is challenging. Brute-force approaches are computationally infeasible, but parallelism provides a solution to handle this complexity.

---
### Technologies Used

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-00599C?logo=c%2B%2B&logoColor=white" alt="C++"/>
</p>
---

## High-Level Structure

### Overview

The application processes files containing proteome content data to:

1. Create a **Bacteria** object from each file, which stores counts of:
   - Individual amino acids (1-mer)
   - 5-mer frequencies
   - 6-mer frequencies
2. Compare two bacteria at a time to evaluate their similarity.

For each combination of six amino acids (6-mers), the observed probabilities are compared to the expected probabilities derived from the corresponding 5-mers. This approach highlights similarities and dissimilarities based on stochastic probability relationships. The results are printed to the console.

---

## Detailed Workflow

### Input Data

The input data for each bacterium is read from a `.txt` file, where:
- Each protein sequence starts with a `>` character (header line).
- The sequence of amino acids follows the header.

### Steps

1. **Data Initialization**:
   - Amino acid counts are updated as sequences are read.
   - Frequencies for 5-mers and 6-mers are calculated using a buffer.

2. **Probability Calculation**:
   - Sparse arrays are used for storing the counts due to the high dimensionality of the data.
   - Observed and expected probabilities for each 6-mer are calculated based on 5-mer and amino acid frequencies.

3. **Index Management**:
   - Efficient increment logic is implemented to avoid expensive division or modulo operations during probability computations.

4. **Sparse Array Conversion**:
   - Non-zero probabilities are stored in a sparse array format for memory efficiency.

5. **Bacteria Comparison**:
   - Two bacteria objects are compared by iterating through their sparse probability arrays.
   - Correlations are computed and normalized by the vector lengths.

---

## Parallelization Approach

The computational complexity of comparing bacteria based on their proteome sequences makes parallelism essential for efficient processing. The parallelization was applied in multiple ways, both using **explicit** and **implicit threading** techniques, leveraging **Windows threads** and **OpenMP** to optimize performance.

### Implicit Threading (OpenMP)

OpenMP was used for implicit parallelism to distribute the workload of comparing bacteria across multiple cores automatically. Key areas where OpenMP parallelism is applied include:

- **6-mer Probability Calculation**: The probability computation for each 6-mer across multiple bacteria is performed concurrently. OpenMP automatically splits the iterations of the loop into multiple threads, allowing the probability calculations to be carried out simultaneously for different k-mers.
- **Sparse Array Operations**: Operations that convert large arrays into sparse arrays are parallelized to reduce the time spent on memory management.

The main benefit of implicit threading is that it requires minimal modification to the existing code, as OpenMP handles the threading and task scheduling automatically. This helps scale the application efficiently on multi-core processors.

### Explicit Threading (Windows Threads)

Explicit parallelism is used in parts of the application where fine control over the threading is required. **Windows threads** are utilized for managing the concurrent comparison of multiple pairs of bacteria. The application creates a new thread for each comparison task, which runs in parallel to others, optimizing the use of system resources.

- **Bacteria Comparison**: Each comparison between two bacteria is assigned to a separate thread. This allows the system to process multiple comparisons at once, rather than sequentially.
- **Vector Operations**: Certain vector operations, such as calculating the correlation between two bacteria, are handled by separate threads to speed up the overall process.

By managing the threading explicitly, the application has full control over the number of threads and their scheduling, enabling fine-tuning for optimal performance.

### Combining OpenMP and Windows Threads

By combining **OpenMP** and **Windows threads**, the application achieves both high-level parallelism (automatic parallelization of loops) and low-level parallelism (explicit control over thread creation). This hybrid approach ensures that the system makes the best use of the available cores while also providing more precise control where necessary.

- **Hybrid Parallelism**: For example, while OpenMP handles the internal loops, Windows threads manage the bacteria comparison tasks in parallel.
- **Scalability**: The parallelism is scalable to handle larger datasets with minimal increase in computation time, making the application robust even as the number of bacterial comparisons grows.

---

## Performance Results

The speedup curve of the application was obtained by running the parallelized code on a **High-Performance Computing (HPC)** cluster, requesting a varying number of cores for the computation. The results demonstrate the efficiency gains achieved by parallelism, highlighting how increasing the number of cores leads to faster computation times, especially when dealing with large datasets.

The application shows significant performance improvements, making it feasible to process larger sets of bacterial comparisons in a fraction of the time required by a single-core approach.

![Screenshot 2025-01-18 181540](https://github.com/user-attachments/assets/8a271dfd-5012-4f49-9534-f7dcd0153ddf)
---

## Technical Details

### Classes

1. **Bacteria Class**:
   - Handles amino acid counting, k-mer frequency vector creation, and stochastic probability computation.
   - Contains methods for:
     - Initializing buffers (`init_buffer`)
     - Continuously updating counts (`cont_buffer`)
     - Converting counts to sparse arrays

2. **Main Class**:
   - Manages the creation and comparison of bacteria objects.

### Comparison Logic

- For each 6-mer, if the observed probability exceeds a threshold:
  - The difference between the observed and expected probabilities is added to a result array.
- Similarity is updated only when matching entries exist for a 6-mer in both bacteria.
- The final correlation is normalized and printed.

---

## Example Input and Output

### Input Format

Each input `.txt` file should be structured as:

>gi|10140927|ref|NP_065504.1| A1 [Alcelaphine herpesvirus 1] MRLHFLFYRSIEFMYYKKSRHKIFCITLSLYFLTAHKRPSARNPITDFFSRIWGPCFYKWSRHQLLLVNQ LMLSPCRNKCSLTSADQCITANEDTTK
