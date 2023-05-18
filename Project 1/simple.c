#include <stdio.h>
#include <math.h>

int main() {
    double nums[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    int i;
    double sumSqRt = 0;
        
    for (i=0; i <= 10; i++) {
        sumSqRt += sqrt(nums[i]);
    }
    
    printf("Average of the square roots = %lf", (sumSqRt/10));
    return 0;
}
