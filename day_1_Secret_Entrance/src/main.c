#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MIN_DIAL_NUMBER = 0;
const int MAX_DIAL_NUMBER = 99;

int main()
{
    int currentDialNumber = 50;
    int numberOfZeros = 0;

    FILE* file;
    char* filePath = "../resources/input.txt";
    file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at location '%s'\n", filePath);
    }

    char line[6] = {0};

    while (fgets(line, sizeof(line), file))
    {
        assert(line[0] == 'L' || line[0] == 'R');

        int dialChangeSign = line[0] == 'L' ? -1 : 1;
        int dialChange = atoi(&line[1]);
        dialChange *= dialChangeSign;

        currentDialNumber += dialChange;

        while (currentDialNumber > MAX_DIAL_NUMBER || currentDialNumber < MIN_DIAL_NUMBER)
        {
            currentDialNumber -= ((MAX_DIAL_NUMBER - MIN_DIAL_NUMBER) + 1) * dialChangeSign;
        }

        if (currentDialNumber == 0)
        {
            numberOfZeros++;
        }
    }

    fclose(file);

    printf("%d\n", numberOfZeros);

    return 0;
}
