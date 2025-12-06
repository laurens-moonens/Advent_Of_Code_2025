#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define maxLineSize 4096
#define maxNumLines 5
#define maxNumberSize 16

typedef enum
{
    Add,
    Multiply
} Operation;

int main()
{
    char* filePath = "../resources/input.txt";
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at filepath '%s'\n", filePath);
        exit(1);
    }

    char lineBuffer[maxLineSize] = {0};
    char reversedLines[maxNumLines][maxLineSize] = {0};
    unsigned int columnWidths[maxLineSize] = {0};
    unsigned int numLines = 0;
    unsigned int numColumns = 0;

    while (fgets(lineBuffer, maxLineSize, file) != NULL)
    {
        numColumns = 0;
        int columnWidth = 0;

        unsigned int lineLength = strlen(lineBuffer) - 1;
        assert(lineLength < maxLineSize);
        printf("%u\n", lineLength);
        for (unsigned int i = 0; i < lineLength; ++i)
        {
            char c = lineBuffer[lineLength - (i + 1)];

            if (c == '\n')
            {
                continue;
            }

            columnWidth++;

            if (c == '*' || c == '+')
            {
                columnWidths[numColumns] = columnWidth;
                numColumns++;
                // set to -1 to account for the space in between columns
                columnWidth = -1;
            }

            reversedLines[numLines][i] = c;
        }
        printf("\n");

        numLines++;
    }

    fclose(file);

    unsigned int numberCount = numLines - 1;
    uint64_t sum = 0;
    unsigned int characterIndex = 0;

    for (unsigned int c = 0; c < numColumns; ++c)
    {
        unsigned int columnWidth = columnWidths[c];

        char operationChar = reversedLines[numLines - 1][characterIndex + columnWidth - 1];
        Operation operation = operationChar == '+' ? Add : Multiply;
        uint64_t result = operation == Add ? 0 : 1;

        for (unsigned int w = 0; w < columnWidth; ++w)
        {
            char numberString[numLines];
            memset(numberString, 0, sizeof(numberString));

            printf("STRING = [");

            for (unsigned int n = 0; n < numberCount; ++n)
            {
                char c = reversedLines[n][characterIndex + w];
                numberString[n] = c;
                printf("%c", c);
            }
            printf("]\n");

            unsigned int number = atoi(numberString);
            printf("NUMBER = %u\n", number);
            switch (operation)
            {
                case Add:
                    result += number;
                    break;
                case Multiply:
                    result *= number;
                    break;
            }
        }

        sum += result;
        characterIndex += columnWidth + 1;
        printf("\n");
    }

    printf("%lu\n", sum);

    return 0;
}
