#include <vector>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <omp.h>

/**
 * explains how to use the program upon wrong input
 * Program implements the sieve of eratosthenes algorithm
 */
void usage(char *program) {
    std::cout << "Usage " << program << " T N" << std::endl;
    std::cout << "Where  T is the number of threads and N is the maximum number for the list." << std::endl;
    exit(1);
}

/**
 * Creates a list using std::vector to store the numbers for the list
 */
std::vector<int> createList(int min, int max) {
    std::vector<int> numbers;
    for (int i = min; i <= max; ++i) {
        numbers.push_back(i);
    }
    return numbers;
} 

/**
 * Checks if a number is divisible with a certain denominator
 */
bool isDivisible(int num, int denominator) {
    return num % denominator == 0;
}

/**
 * Performs the iterative step of sieve of eratosthenes for one number
 * PARAM numbers: the list to perform the algorithm on
 * PARAM denominator: the number to check the multiples of
 */
void sieveAlgorithm(std::vector<int>* numbers, int denominator) {
    int startValue = denominator * denominator;
    //find start value
    auto startIt = std::find_if(numbers->begin(), numbers->end(), [startValue](int num) {
            return num >= startValue;
        });

    //erase all numbers divisible with denominator
    numbers->erase(
            std::remove_if(startIt, numbers->end(),
                           [denominator](int num) { return isDivisible(num, denominator); }),
            numbers->end()
        );
}

/**
 * Main function for the parallel sieve of eratosthenes program
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage(argv[0]);
    }

    int threads = atoi(argv[1]);
    int max = atoi(argv[2]);

    if (max <= 1) { //make sure max is larger than 1
        std::cout << "max must be larger than 1" << std::endl;
        exit(1);
    }

    // Step 1: Sequentially compute primes up to sqrt(max)
    int sqrtmax = sqrt(max);
    std::vector<int> seed = createList(2, sqrtmax);

    int i = 2; //Start searching at 2 the first prime number
    while(i*i <= sqrtmax) {
        sieveAlgorithm(&seed, i);
        auto it = std::find(seed.begin(), seed.end(), i);
        i = *(it + 1);
    }

    // Step 2: Divide sqrt(max) + 1 to max into chunks and allocate to threads

    std::vector<std::vector<int>> chunkCollection(threads); //Vector to hold the chunk vectors

    int chunkSize = (max - sqrtmax) / threads;
    int remainder = (max - sqrtmax) % threads;

    int current = sqrtmax + 1;
    for (int i = 0; i < threads; ++i) {
        // Calculate how many elements should go in this chunk
        int currentChunkSize = chunkSize;
        if (remainder > 0) {
            currentChunkSize += 1;
            --remainder;
        }
        chunkCollection[i] = createList(current, current+currentChunkSize-1); // -1 Because current is already included
        current = current + currentChunkSize;

    }

        /*** Start Timing ***/
        auto start_time = std::chrono::system_clock::now();

    // Step 3: Threads compute using the seed.
   #pragma omp parallel num_threads(threads)
    {
        int thread_id = omp_get_thread_num();  // Get thread number
        std::vector<int>& chunk = chunkCollection[thread_id];  // Reference to the thread's chunk

        // Each thread processes its chunk using the seed primes
        for (int prime : seed) {
            sieveAlgorithm(&chunk, prime);
        }
    }

    std::chrono::duration<double> duration =
        (std::chrono::system_clock::now() - start_time);
        /*** Stop Timing***/



   // Step 4: Collect results from all threads
    for (int i = 0; i < threads; ++i) {
        seed.insert(seed.end(), chunkCollection[i].begin(), chunkCollection[i].end());
    }

    // Step 5: Print results
    std::sort(seed.begin(), seed.end());  // Sort the final list of primes
    std::cout << "The prime numbers between 1 and " << max << " are:" << std::endl;
    for (int num : seed) {
            std::cout << num << " ";
        }
    std::cout << std::endl;
    std::cout << "With " << threads << " threads this took " << duration.count() << " seconds" << std::endl;


    return 0;
}