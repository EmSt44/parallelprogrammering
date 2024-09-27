#include <thread>
#include <chrono>
#include <iostream>
#include <string.h>
#include <mutex>
#include <vector>

void usage(char *program) {
    std::cout << "Usage: " << program << " T Z" << std::endl;
    std::cout << "  T: number of threads" << std::endl;
    std::cout << "  Z: number of trapezes" << std::endl;
    std::cout << std::endl;
    std::cout << "Or use: '" << program << " -h' for help." << std::endl;
    exit(1);
}

void help(char *program) {
    std::cout << "This program calculates numerical integration by parallelizing calculation." << std::endl;
    std::cout << "The program will return the result and the time it took to complete the calculation." << std::endl;
    std::cout << "To use the program correctly run it with: " << program << " T Z" << std::endl;
    std::cout << "Where T is the number of threads used to calculate." << std::endl;
    std::cout << "And Z is the amount of trapezes to be calculated" << std::endl;
    std::cout << "NOTE: if the number of threads exceeds the number of trapezoids the number of threads will be set to be equal to the number of trapezoids." << std::endl;
    exit(1);
}

/**
 * the function 4/(1+x²)
 */
double f(double x) {
     return 4.0 / (1.0 + x * x);
}

/**
 * Calculates the area of a trapezoid between two bounds in the function f.
 */
double trapezoid(double lower_bound, double upper_bound) {
    double h = upper_bound - lower_bound; // The width of the trapezoid
    double area = (h/2) * (f(lower_bound) + f(upper_bound));
    return area;
}

/**
 * Used to make sure final result is correct by disallowing simultaneous writing by threads.
 */
std::mutex mtx;

/**
 * Calculates integral over a range by dividing the range into a number of trapezoids
 */
void integrate_range(double a, double b, int num_trapezoids, double& result) {
    double h = (b - a) / num_trapezoids;  // Calculate width of each trapezoid
    double local_sum = 0.0;

    // Compute the integral for the local range
    for (int i = 0; i < num_trapezoids; ++i) {
        double x1 = a + i * h;      // Left point of the trapezoid
        double x2 = a + (i + 1) * h;  // Right point of the trapezoid
        local_sum += trapezoid(x1, x2);
    }

    // Lock the mutex and add the local result to the shared result
    std::lock_guard<std::mutex> guard(mtx);
    result += local_sum;
}

int main(int argc, char *argv[]) {

    if (strcmp(argv[1], "-h") == 0) {
        help(argv[0]);
    }
    if (argc != 3) {
        usage(argv[0]);
    }
    if (argc == 3) {
        int num_threads = atoi(argv[1]);
        int num_trapezoids = atoi(argv[2]);
        
        // If there are more threads than trapezoids set the number of threads to number of trapezoids to avoid threads with no work.
        if (num_threads > num_trapezoids) {
            num_threads = num_trapezoids;
        }

        double result = 0.0;  // Shared result

        // Vector to hold the threads
        std::vector<std::thread> threads;

        // Divide the trapezoids among the threads
        int trapezoids_per_thread = num_trapezoids / num_threads;
        int remainder = num_trapezoids % num_threads;  // Handle any leftover trapezoids

        double a = 0.0;
        double b = 1.0;
        double h = (b - a) / num_trapezoids;  // Total width divided by number of trapezoids


        /*** Start Timing ***/
        auto start_time = std::chrono::system_clock::now();


        // Launch the threads
        double current_a = a;
        for (int i = 0; i < num_threads; ++i) {
            // Calculate the number of trapezoids for this thread (distribute remainder)
            int num_for_this_thread = trapezoids_per_thread + (i < remainder ? 1 : 0);
            
            // Calculate the range for this thread
            double current_b = current_a + num_for_this_thread * h;
            
            // Create and run a thread to calculate the integration over this range
            threads.push_back(std::thread(integrate_range, current_a, current_b, num_for_this_thread, std::ref(result)));
            
            // Update the starting point for the next thread
            current_a = current_b;
        }

        // Wait for all threads to complete
        for (std::thread &t : threads) {
            t.join();
        }


        std::chrono::duration<double> duration =
        (std::chrono::system_clock::now() - start_time);
        /*** Stop Timing***/


        std::cout << "The approximate value of the integral is: " << result << std::endl;
        std::cout << "With " << num_threads << " threads calculating " << num_trapezoids << " trapezoids this took " << duration.count() << " seconds." <<std::endl;

    }

     return 0;
}