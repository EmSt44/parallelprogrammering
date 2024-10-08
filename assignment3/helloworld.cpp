#include <omp.h>
#include <iostream>
int main()
{
    #pragma omp parallel 
    {
        int id = 0;
        std::cout << "Hello"  << id << " World" << std::endl;
    }
    return 0;
}

