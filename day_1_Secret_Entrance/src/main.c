#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MIN_DIAL_NUMBER = 0;
const int MAX_DIAL_NUMBER = 99;

int main()
{
    int currentDialNumber = 50;
    int numberOfZerosEnded = 0;
    int numberOfZerosPassed = 0;

    FILE* file;
    char* filePath = "../resources/input.txt";
    file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at location '%s'\n", filePath);
        exit(1);
    }

    char line[6] = {0};

    while (fgets(line, sizeof(line), file))
    {
        assert(line[0] == 'L' || line[0] == 'R');

        int dialChangeSign = line[0] == 'L' ? -1 : 1;
        int dialChange = atoi(&line[1]);
        dialChange *= dialChangeSign;

        if (dialChange == 0)
        {
            continue;
        }

        int prevNumberOfZerosEnded = numberOfZerosEnded;
        int prevNumberOfZerosPassed = numberOfZerosPassed;
        bool startedAtZero = currentDialNumber == 0;

        currentDialNumber += dialChange;

        while (currentDialNumber > MAX_DIAL_NUMBER || currentDialNumber < MIN_DIAL_NUMBER)
        {
            currentDialNumber -= ((MAX_DIAL_NUMBER - MIN_DIAL_NUMBER) + 1) * dialChangeSign;
            numberOfZerosPassed++;
        }

        if ((startedAtZero && dialChangeSign == -1) || (currentDialNumber == 0 && dialChangeSign == 1))
        {
            numberOfZerosPassed--;
        }

        if (currentDialNumber == 0)
        {
            numberOfZerosEnded++;
        }

        assert(prevNumberOfZerosEnded <= numberOfZerosEnded);
        assert(prevNumberOfZerosPassed <= numberOfZerosPassed);
    }

    fclose(file);

    printf("Number of times ended at zero = %d\n", numberOfZerosEnded);
    printf("Number of times passed by zero AND ended at zero = %d\n", numberOfZerosPassed + numberOfZerosEnded);

    return 0;
}
