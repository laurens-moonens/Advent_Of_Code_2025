#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Range
{
    uint64_t min;
    uint64_t max;
} Range;

int main()
{
    char* filePath = "../resources/input.txt";
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at path '%s'\n", filePath);
        exit(1);
    }

    Range ranges[1024];
    unsigned int rangesSize = 0;
    uint64_t min, max;
    int numMatches = 0;
    char buffer[64];

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
    }

    uint64_t ids[1024];
    uint64_t id;
    unsigned int idsSize = 0;

    for (idsSize = 0; fscanf(file, "%lu\n", &id) == 1; ++idsSize)
    {
        if (idsSize >= sizeof(ids) / sizeof(ids[0]))
        {
            printf("ID's array size is too small.\n");
            exit(1);
        }

        ids[idsSize] = id;
    }

    unsigned int numFreshIds = 0;

    for (unsigned int i = 0; i < idsSize; ++i)
    {
        for (unsigned int r = 0; r < rangesSize; ++r)
        {
            Range range = ranges[r];
            id = ids[i];
            if (range.min <= id && range.max >= id)
            {
                printf("%lu is fresh\n", id);
                numFreshIds++;
                break;
            }
        }
    }

    printf("%d\n", numFreshIds);

    fclose(file);

    return 0;
}
