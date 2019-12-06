///usr/bin/gcc test.cpp && ./a.out && rm a.out && exit
#include <x86intrin.h>
#include <stdio.h>

void run(long long int multiplier) {
    int item_dist= 30;
    int sum_dist = 100;
    int dist_int = (int) ((double)item_dist* (double)multiplier + 0.5);
    int sum_int = (int)((double)sum_dist* (double)multiplier + 0.5);
    double distance_percent = (double)dist_int/(double)sum_int;
    printf("%20lld %.20f\n",multiplier,  distance_percent); 
}

int main() {
    int start_cycles= _rdtsc();

    run(1);
    run(10);
    run(100);
    run(1000);
    run(100000);
    run(10000000);

    int dt_cycles = _rdtsc() - start_cycles;
    printf("cycles %d\n", dt_cycles);
    return 0;
}
