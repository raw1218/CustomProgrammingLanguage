#include <stdio.h>
#include <omp.h>
#include <unistd.h>

void printNumbers() {
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (int i = 1; i <= 1000000000; i++) {
                
                int x = 4;
                x = 4 + 5;
            }
        }

        #pragma omp section
        {
            for (int i = 1000000000; i <= 20000000000; i++) {
             
               int x = 5;
               x = 5 + 8;
            }
        }
    }
}

int main() {
    printNumbers();
    return 0;
}
