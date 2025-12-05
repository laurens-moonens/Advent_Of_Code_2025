#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Range
{
    uint64_t min;
    uint64_t max;
} Range;

bool AreRangesOverlapping(const Range r1, const Range r2)
{
    return (r1.min >= r2.min && r1.min <= r2.max) ||
           (r1.max <= r2.max && r1.max >= r2.min) ||
           (r1.min <= r2.min && r1.max >= r2.max);
}

int main()
{
    char* filePath = "../resources/input.txt";
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at path '%s'\n", filePath);
        exit(1);
    }

    Range ranges[256];
    unsigned int rangesSize = 0;
    uint64_t min, max;
    int numMatches = 0;
    char buffer[64] = {0};

    for (rangesSize = 0; fgets(buffer, sizeof(buffer), file) != NULL; ++rangesSize)
    {
        numMatches = sscanf(buffer, "%lu-%lu\n", &min, &max);
        if (numMatches != 2)
        {
            break;
        }

        if (rangesSize >= sizeof(ranges) / sizeof(ranges[0]))
        {
            printf("Ranges array size is too small.\n");
            exit(1);
        }

        ranges[rangesSize].min = min;
        ranges[rangesSize].max = max;

        assert(min <= max);
    }

    Range nonOverlappingRanges[256] = {0};
    unsigned int nonOverlappingRangesSize = 0;
    unsigned int numberOfOverlaps = 0;

    do
    {
        nonOverlappingRangesSize = 0;
        numberOfOverlaps = 0;

        for (unsigned int i = 0; i < rangesSize; ++i)
        {
            Range range = ranges[i];

            bool overlapped = false;

            for (unsigned int j = 0; j < nonOverlappingRangesSize; ++j)
            {
                Range nonOverlappingRange = nonOverlappingRanges[j];

                if (AreRangesOverlapping(range, nonOverlappingRange) ||
                    AreRangesOverlapping(nonOverlappingRange, range))
                {
                    overlapped = true;
                    numberOfOverlaps++;
                    printf("[%lu - %lu] overlaps with [%lu - %lu]\n", range.min, range.max, nonOverlappingRange.min, nonOverlappingRange.max);
                    nonOverlappingRange.min = range.min < nonOverlappingRange.min ? range.min : nonOverlappingRange.min;
                    nonOverlappingRange.max = range.max > nonOverlappingRange.max ? range.max : nonOverlappingRange.max;
                    nonOverlappingRanges[j] = nonOverlappingRange;
                    break;
                }
            }

            if (!overlapped)
            {
                nonOverlappingRanges[nonOverlappingRangesSize] = range;
                nonOverlappingRangesSize++;
            }
        }

        memcpy(ranges, nonOverlappingRanges, sizeof(nonOverlappingRanges));
        rangesSize = nonOverlappingRangesSize;
    } while (numberOfOverlaps > 0);

    uint64_t numberOfFreshIds = 0;

    for (unsigned int i = 0; i < nonOverlappingRangesSize; ++i)
    {
        Range range = nonOverlappingRanges[i];
        numberOfFreshIds += (range.max - range.min) + 1;
        printf("%lu - %lu\n", nonOverlappingRanges[i].min, nonOverlappingRanges[i].max);
    }

    printf("%lu\n", numberOfFreshIds);

    fclose(file);

    return 0;
}
