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

    int contract_balance = 448389;

    int total_distance = 0;
    int distances[] = { 32, 71 };
    int distance_count = 2;
    for (int i = 0; i < distance_count; ++i) {
        total_distance += distances[i];
    }
    double total_percent;
    for (int i = 0; i < distance_count; ++i) {
        double percent = (double)distances[i] / total_distance * 100;
        total_percent += percent;
        double rounded = (int)(percent +0.5);
        printf("%d %.20f %.20f\n", distances[i], percent, rounded);
    }
    printf("total percent %20f\n", total_percent);
    printf("common\n");

    double percent = 0;
    printf("normal, 15, 30, 60 \n");
    for (int i = 0; i < 20; ++i) {
        percent += 5;
        printf("%.2f, %.15f, %.30f, %.60f \n", 
            percent, percent, percent, percent);
    }
    

#if 0
    run(1);
    run(10);
    run(100);
    run(1000);
    run(100000);
    run(10000000);
#endif

    int dt_cycles = _rdtsc() - start_cycles;
    printf("cycles %d\n", dt_cycles);
    return 0;
}
