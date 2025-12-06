#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define maxLineSize 2048
#define maxNumLines 4

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

    uint64_t numberMatrix[maxNumLines][maxLineSize] = {0};
    Operation operations[maxLineSize];
    char charBuffer[16];
    unsigned int charBufferSize = 0;
    char c;

    unsigned int numLines = 0;
    unsigned int numberCount = 0;
    unsigned int maxNumberCount = 0;

    while ((c = fgetc(file)) != EOF)
    {
        if (c == ' ')
        {
            if (charBufferSize > 0)
            {
                if (charBufferSize == 1 && (charBuffer[0] == '+' || charBuffer[0] == '*'))
                {
                    operations[numberCount] = charBuffer[0] == '+' ? Add : Multiply;
                    numberCount++;
                }
                else
                {
                    numberMatrix[numLines][numberCount] = atoi(charBuffer);
                    numberCount++;
                }
            }

            memset(charBuffer, 0, sizeof(charBuffer));
            charBufferSize = 0;

            continue;
        }
        else if (c == '\n')
        {
            if (charBufferSize > 0)
            {
                if (charBufferSize == 1 && (charBuffer[0] == '+' || charBuffer[0] == '*'))
                {
                    operations[numberCount] = charBuffer[0] == '+' ? Add : Multiply;
                    numberCount++;
                }
                else
                {
                    numberMatrix[numLines][numberCount] = atoi(charBuffer);
                    numberCount++;
                }
            }

            memset(charBuffer, 0, sizeof(charBuffer));
            charBufferSize = 0;

            numLines++;
            assert(maxNumberCount == 0 || maxNumberCount == numberCount);
            maxNumberCount = numberCount;
            numberCount = 0;
            continue;
        }

        charBuffer[charBufferSize] = c;
        charBufferSize++;
    }

    fclose(file);

    uint64_t sum = 0;

    for (int n = 0; n < maxNumberCount; ++n)
    {
        Operation operation = operations[n];

        for (int l = 0; l < numLines - 1; ++l)
        {
            if (l < numLines - 1)
            {
                printf("[%lu] ", numberMatrix[l][n]);
            }
            else
            {
                char* operation = operations[n] == Add ? "A " : "M ";
                printf("%s", operation);
            }
        }
        printf("\n");
    }

    for (int n = 0; n < maxNumberCount; ++n)
    {
        Operation operation = operations[n];
        uint64_t result = operation == Add ? 0 : 1;

        char* opString = operation == Add ? "A " : "M ";
        printf("%s: ", opString);

        for (int l = 0; l < numLines - 1; ++l)
        {
            printf("[%lu] ", numberMatrix[l][n]);
            switch (operation)
            {
                case Add:
                    printf("ADD");
                    result += numberMatrix[l][n];
                    break;
                case Multiply:
                    printf("MULT");
                    result *= numberMatrix[l][n];
                    break;
            }
        }
        printf(" = %lu\n", result);
        sum += result;
    }

    printf("%lu\n", sum);

    return 0;
}
