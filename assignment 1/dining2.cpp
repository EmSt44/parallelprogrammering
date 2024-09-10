#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex out;

void philosopher(int n, std::mutex* lower, std::mutex* higher)
{
    while (true)
    {
        out.lock();
        std::cout << "Philosopher " << n << " is thinking." << std::endl;
        out.unlock();

        lower->lock();
        out.lock();
        std::cout << "Philosopher " << n << " picked up her lower fork." << std::endl;
        out.unlock();

        higher->lock();
        out.lock();
        std::cout << "Philosopher " << n << " picked up her higher fork." << std::endl;
        out.unlock();

        out.lock();
        std::cout << "Philosopher " << n << " is eating." << std::endl;
        out.unlock();

        out.lock();
        std::cout << "Philosopher " << n << " is putting down her higher fork." << std::endl;
        out.unlock();
        higher->unlock();

        out.lock();
        std::cout << "Philosopher " << n << " is putting down her lower fork." << std::endl;
        out.unlock();
        lower->unlock();
    }
}

void usage(char* program)
{
    std::cout << "Usage: " << program << " N  (where 2<=N<=10)" << std::endl;
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // philosophers = argv[1]
    int philosophers;
    try
    {
        philosophers = std::stoi(argv[1]);
    }
    catch (std::exception const&)
    {
        usage(argv[0]);
    }

    if (philosophers < 2 || philosophers > 10)
    {
        usage(argv[0]);
    }

    // forks
    std::vector<std::mutex> forks(philosophers);

    // philosophers
    std::vector<std::thread> ph;
    for (int i = 0; i < philosophers; ++i)
    {
        int left = i;                   // left fork
        int right = (i + 1) % philosophers; // right fork

        // Determine which fork is lower and which is higher
        std::mutex* lower = &forks[std::min(left, right)];
        std::mutex* higher = &forks[std::max(left, right)];

        // Spawn philosopher thread with ordered resource acquisition (lower -> higher)
        ph.emplace_back(philosopher, i, lower, higher);
    }

    for (auto& p : ph)
    {
        p.join();
    }

    return 0;
}
