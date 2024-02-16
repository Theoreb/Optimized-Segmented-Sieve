#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h> // Portable PRIu64 macro

uint64_t run(uint64_t max) {
    uint64_t byteSize = ((max >> 4) + 1) / 3 + 1;
    printf("Attempting to allocate %" PRIu64 " bytes.\n", byteSize);

    unsigned char* mem = (unsigned char*)malloc(byteSize);
    if (!mem) {
        printf("Failed to allocate memory.\n");
        return 0;
    }
    for (uint64_t i = 0; i < byteSize; i += 3) {
        mem[i] = 0b01001001;
        mem[i + 1] = 0b10010010;
        mem[i + 2] = 0b00100100;
    }

    uint64_t mm = max >> 1;
    uint64_t sq = (uint64_t)sqrt(max) | 1;
    uint64_t total = 2;

    for (uint64_t n = 5; n <= sq; n += 2) {
        uint64_t idx = (n - 2) >> 1;
        if (!(mem[idx >> 3] & (1 << (idx & 7)))) {
            total++;
            printf("%" PRIu64 "\t\t", n);
            for (uint64_t k = (n * n - 2) >> 1; k <= mm; k += n) {
                mem[k >> 3] |= (1 << (k & 7));
            }
        }
    }
    
    uint64_t cacheidx = (sq - 2) >> 1;
    unsigned char bit = 1 << (cacheidx & 7);
    cacheidx >>= 3;
    unsigned char cache = mem[cacheidx];

    for (uint64_t n = sq + 2; n <= max; n += 2) {
        if (!(cache & bit)) {
            printf("%" PRIu64 "\t\t", n);
            total++;
        }

        bit <<= 1;
        if (bit & 256) {
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

    printf("Allocated %" PRIu64 " bytes (%" PRIu64 " MB) for the operation.\n", ((max_value >> 4) + 1) / 3 + 1, (((max_value >> 4) + 1) / 3 + 1) >> 20);
    printf("Found %" PRIu64 " primes from 2 to %" PRIu64 " in %f seconds\n", total, max_value, time_spent);

    return 0;
}
