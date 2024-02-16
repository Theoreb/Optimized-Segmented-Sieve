#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

uint64_t run(uint64_t max) {
    if (max < 5) {
        printf("Max value must be at least 5.\n");
        return -1;
    }
    uint64_t byteSize = ((max - 5) >> 4) + 1;
    byteSize += 3 - (byteSize % 3); // Bit overflow for optimization
    printf("Attempting to allocate %lu bytes.\n", byteSize);

    unsigned char* mem = (unsigned char*)malloc(byteSize);
    if (!mem) {
        printf("Failed to allocate memory.\n");
        return -1;
    }
    for (uint64_t i = 0; i < byteSize; i += 3) {
        mem[i] = 0b00100100; // if (i + 1 < byteSize)
        mem[i + 1] = 0b01001001;
        mem[i + 2] = 0b10010010;
    }

    printf("Memory allocated.\n");

    uint64_t halfMax = max >> 1;
    uint64_t sqrtMax = (uint64_t)sqrt(max) | 1;
    uint64_t total = 2;

    if (sqrtMax < 5) { sqrtMax = 5; }

    for (uint64_t n = 5; n <= sqrtMax; n += 2) {
        uint64_t idx = (n - 5) >> 1;
        unsigned char byte = mem[idx >> 3];
        unsigned char bit = 1 << (idx & 7);

        if (!(byte & bit)) {
            total++;
            for (uint64_t k = (n * n - 5) >> 1; k <= halfMax; k += n) {
                mem[k >> 3] |= (1 << (k & 7));
            }
        }
    }

    uint64_t cacheidx = (sqrtMax - 5) >> 1;
    unsigned char bit = 1 << ((cacheidx & 7) - 1);
    cacheidx >>= 3;
    unsigned char cache = mem[cacheidx];

    for (uint64_t n = sqrtMax + 2; n <= max; n += 2) {
        if (!(cache & bit)) {
            total++;
        }

        if ((bit <<= 1) == 0) {
            bit = 1;
            cacheidx++;
            cache = mem[cacheidx];
        }
    }

    free(mem);
    return total;
}

int main(int argc, char *argv[]) {
    uint64_t max_value;
    if (argc == 2) {
        max_value = strtoull(argv[1], NULL, 10);
    } else {
        printf("Usage: %s <max value>\n", argv[0]);
        return 1;
    }

    clock_t start = clock();
    uint64_t total = run(max_value);
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Allocated %lu bytes (%lu MB) for the operation.\n", ((max_value >> 4) + 1) / 3 + 1, (((max_value >> 4) + 1) / 3 + 1) >> 20);
    printf("Found %lu primes from 2 to %lu in %f seconds\n", total, max_value, time_spent);

    return 0;
}
