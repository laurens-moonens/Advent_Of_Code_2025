#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char* filePath = "../resources/input.txt";
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at path '%s'\n", filePath);
        exit(1);
    }

    uint64_t rangeStart = 1;
    uint64_t rangeEnd = 1;

    char numberToCheck[32] = {0};

    uint64_t sum = 0;

    while (fscanf(file, "%lu-%lu,", &rangeStart, &rangeEnd) != EOF)
    {
        printf("[%lu - %lu]\n", rangeStart, rangeEnd);

        for (uint64_t i = rangeStart; i <= rangeEnd; ++i)
        {
            memset(numberToCheck, 0, sizeof(numberToCheck) / sizeof(numberToCheck[0]));
            sprintf(numberToCheck, "%lu", i);
            int numberOfChars = strlen(numberToCheck);

            bool hasSubSequenceOfDigits = false;

            for (int divider = 2; divider <= numberOfChars; ++divider)
            {
                if (numberOfChars % divider != 0)
                    continue;

                int subStringLength = numberOfChars / divider;
                char subString[subStringLength];
                memcpy(subString, numberToCheck, subStringLength);

                bool allSubSequencesAreTheSame = true;

                for (int subStringIndex = 0; subStringIndex < divider; ++subStringIndex)
                {
                    if (memcmp(subString, numberToCheck + (subStringLength * subStringIndex), subStringLength) != 0)
                    {
                        allSubSequencesAreTheSame = false;
                        break;
                    }
                }

                if (allSubSequencesAreTheSame)
                {
                    hasSubSequenceOfDigits = true;
                    break;
                }
            }

            if (hasSubSequenceOfDigits)
            {
                sum += i;
                printf("%s has sub sequence\n", numberToCheck);
            }
        }
    }

    printf("SUM = %lu\n", sum);

    fclose(file);
}
