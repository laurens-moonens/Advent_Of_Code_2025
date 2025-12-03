#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define digitCount 12

int main()
{
    char* filePath = "../resources/input.txt";
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at location '%s'\n", filePath);
        exit(1);
    }

    char line[128];
    uint64_t sum = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        int numberStringSize = strlen(line) - 1;
        char largestNumberString[digitCount + 1] = {0};
        int prevDigitIndex = -1;

        for (int digit = 0; digit < digitCount; ++digit)
        {
            int startingIndex = digit > prevDigitIndex + 1 ? digit : prevDigitIndex + 1;
            for (int i = startingIndex; i < numberStringSize - (digitCount - (digit + 1)); ++i)
            {
                if (line[i] > largestNumberString[digit])
                {
                    largestNumberString[digit] = line[i];
                    prevDigitIndex = i;
                }
            }
        }

        uint64_t largestNumber = atoll(largestNumberString);
        sum += largestNumber;
        printf("%s\n", largestNumberString);
    }

    printf("%lu\n", sum);

    fclose(file);
}
