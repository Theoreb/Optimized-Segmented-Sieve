#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define ln(x) log(x)

uint64_t* simpleSieve(uint64_t limit, uint64_t* count);
void segmentedSieve(uint64_t max);

long getL1CacheSize() {
    FILE* file = fopen("/sys/devices/system/cpu/cpu0/cache/index0/size", "r");
    if (!file) {
        perror("fopen");
        return -1;
    }
    
    long l1CacheSize = 0;
    fscanf(file, "%ld", &l1CacheSize);
    fclose(file);

    printf("L1 Cache Size: %ld KB\n", l1CacheSize);

    return l1CacheSize * 1024;
}

static void updateProgressBar(int progress, int total) {
    static int lastPercent = -1;
    int currentPercent = progress * 100 / total;

    if (currentPercent > lastPercent) {
        lastPercent = currentPercent;
        
        const int barWidth = 50;
        int filledWidth = barWidth * progress / total;

        printf("[");
        for (int i = 0; i < barWidth; i++) {
            if (i < filledWidth) {
                printf("=");
            } else {
                printf(" ");
            }
        }
        printf("] %d%%\r", currentPercent);
        fflush(stdout);
    }
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
    segmentedSieve(max_value);
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\nCalculated primes up to %lu in %f seconds\n", max_value, time_spent);

    return 0;
}


void segmentedSieve(uint64_t max) {
    const uint64_t limit = sqrt(max);
    uint64_t count;
    const uint64_t* basePrimes = simpleSieve(limit, &count);
    printf("Found %lu primes from 2 to %lu\n", count, limit);

    if (!basePrimes) {
        printf("Simple sieve failed.\n");
        return;
    }

    // Precompute all square of primes
    uint64_t* precomputedPrimeSquares = (uint64_t*)malloc(count * sizeof(uint64_t));
    for (uint64_t i = 0; i < count; i++) {
        precomputedPrimeSquares[i] = basePrimes[i] * basePrimes[i];
    }

    const uint64_t segmentSize = getL1CacheSize()-1000;
    char* segment = malloc(segmentSize * sizeof(char));
    if (!segment) {
        printf("Memory allocation failed for the segment.\n");
        return;
    }

    printf("Sieve using Segment size of %lu bytes (%lu KB)\n", segmentSize, segmentSize >> 10);

    uint64_t totalCount = count;

    for (uint64_t low = limit; low <= max; low += segmentSize) {
        
        const uint64_t high = fmin(low + segmentSize - 1, max);
        memset(segment, 0, segmentSize);

        double progress = (double)(low - limit) / (max - limit);
        updateProgressBar((int)(progress * 100), 100);

        for (uint64_t i = 0; i < count; i++) {
            const uint64_t prime = basePrimes[i];
            const uint64_t primeSquare = precomputedPrimeSquares[i];

            uint64_t minMultiple = (low + prime - 1) / prime * prime;
            if (minMultiple < primeSquare)
                minMultiple = primeSquare;
            for (uint64_t j = minMultiple; j <= high; j += prime) {
                segment[j - low] = 1;
            }
        }

        for (uint64_t n = low; n <= high; n++) {
            if (!segment[n - low]) {
                totalCount++;
            }
        }
    }
    printf("\n Found %lu primes.\n", totalCount);

    free(segment);
    free((uint64_t*)basePrimes);
    free(precomputedPrimeSquares);
}

uint64_t *simpleSieve(uint64_t limit, uint64_t* count) {
    if (limit < 5) {
        printf("Max value must be at least 5.\n");
        return NULL;
    }
    uint64_t byteSize = ((limit - 5) >> 4) + 1;
    byteSize += 3 - (byteSize % 3); // Make it a multiple of 3
    printf("Attempting to allocate %lu bytes (%lu KB).\n", byteSize, byteSize >> 10);

    unsigned char* mem = (unsigned char*)malloc(byteSize);
    if (!mem) {
        printf("Failed to allocate memory.\n");
        return NULL;
    }

    const uint64_t primesApproximation = limit / (ln(limit) - 1.1);
    uint64_t* primes = (uint64_t*)malloc(primesApproximation * sizeof(uint64_t));
    if (!primes) {
        printf("Failed to allocate memory.\n");
        return NULL;
    }
    primes[0] = 2;
    primes[1] = 3;
    *count = 2;

    for (uint64_t i = 0; i < byteSize; i += 3) {
        mem[i] = 0b00100100;
        mem[i + 1] = 0b01001001;
        mem[i + 2] = 0b10010010;
    }

    printf("Memory allocated.\n");

    const uint64_t halfMax = limit >> 1;
    uint64_t sqrtMax = (uint64_t)sqrt(limit); // Have to be odd !
    if (sqrtMax < 5) { sqrtMax = 5; }

    for (uint64_t n = 5; n <= sqrtMax; n += 2) {
        const uint64_t idx = (n - 5) >> 1;
        const unsigned char byte = mem[idx >> 3];
        const unsigned char bit = 1 << (idx & 7);

        if (!(byte & bit)) {
            primes[(*count)++] = n;
            const int startPos = (n * n - 5) >> 1;
            for (uint64_t k = (n * n - 5) >> 1; k <= halfMax; k += n) {
                mem[k >> 3] |= (1 << (k & 7));
            }
        }
    }

    uint64_t cacheidx = (sqrtMax + 1 - 5) >> 1;
    unsigned char bit = 1 << (cacheidx & 7);
    cacheidx >>= 3;
    unsigned char cache = mem[cacheidx];

    for (uint64_t n = sqrtMax + 1; n <= limit; n += 2) {
        if (!(cache & bit)) {
            primes[(*count)++] = n;
        }

        if ((bit <<= 1) == 0) {
            bit = 1;
            cacheidx++;
            cache = mem[cacheidx];
        }
    }

    printf("[SimpleSieve]: Found %lu primes from 2 to %lu and approximated %lu\n", *count, limit, primesApproximation);

    free(mem);
    return primes;
}
