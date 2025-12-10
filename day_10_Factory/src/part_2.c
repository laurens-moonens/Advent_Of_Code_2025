// Failed! Algorithm is too slow to run.
// I check every permutation of buttons to press.
// I already filter out duplicate permutations, but this is not enough
// Instead I should start checking permutations using the buttons with the biggest impact (buttons with the most connection)
// Only when I overshoot should I start checking buttons with fewer connections
// buttons with equal connections should be checked together

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUM_LIGHTS 16
#define MAX_NUM_BUTTONS 16
#define MAX_NUM_JOLTAGES 16
#define MAX_NUM_MACHINES 256
#define MAX_LINE_LENGTH 256
#define PERMUTATIONS_SIZE MAX_NUM_BUTTONS * MAX_NUM_BUTTONS * MAX_NUM_BUTTONS * MAX_NUM_BUTTONS * MAX_NUM_BUTTONS* MAX_NUM_BUTTONS
#define MAX_BUTTONS_IN_PERMUTATION MAX_NUM_BUTTONS* MAX_NUM_BUTTONS

typedef struct
{
    unsigned int connectedJoltages[MAX_NUM_LIGHTS];
    unsigned int numConnectedJoltages;
} Button;

typedef struct
{
    bool lightsRequiredState[MAX_NUM_LIGHTS];
    unsigned int numLights;
    Button buttons[MAX_NUM_BUTTONS];
    unsigned int numButtons;
    int joltages[MAX_NUM_JOLTAGES];
    unsigned int numJoltages;
} Machine;

typedef struct
{
    unsigned int buttonIndices[MAX_BUTTONS_IN_PERMUTATION];
    unsigned int numIndices;
} Permutation;

void ApplyButtonPressToJoltages(int (*joltage)[MAX_NUM_JOLTAGES], const Button button)
{
    for (int l = 0; l < button.numConnectedJoltages; ++l)
    {
        unsigned int joltageIndex = button.connectedJoltages[l];
        (*joltage)[joltageIndex]++;
    }
}

void AddNextButtonWithDuplicates(Permutation (*const currentPermutations)[PERMUTATIONS_SIZE], unsigned int numCurrentPermutations, Permutation (*nextPermutations)[PERMUTATIONS_SIZE], unsigned int* numNextPermutations, unsigned int startNumber, unsigned int numButtons, unsigned int currentButtonIndex)
{
    if (numCurrentPermutations == 0)
    {
        for (int b = 0; b < numButtons; ++b)
        {
            (*nextPermutations)[b].buttonIndices[0] = b;
            (*nextPermutations)[b].numIndices = currentButtonIndex + 1;
        }

        *numNextPermutations = numButtons;
        return;
    }

    for (int c = 0; c < numCurrentPermutations; ++c)
    {
        Permutation currentPermutation = (*currentPermutations)[c];
        //printf("PARENT permutation = ");
        //for (int i = 0; i < currentButtonIndex; ++i)
        //{
        //    printf("%d ", (*currentPermutation)[i]);
        //}
        //printf("\n");

        unsigned int lastNumber = currentPermutation.buttonIndices[currentButtonIndex - 1];

        for (unsigned int i = lastNumber; i < numButtons; ++i)
        {
            memcpy(&(*nextPermutations)[*numNextPermutations], &currentPermutation, sizeof(Permutation));
            (*nextPermutations)[(*numNextPermutations)].buttonIndices[currentButtonIndex] = i;
            (*nextPermutations)[(*numNextPermutations)++].numIndices = currentButtonIndex + 1;

            if (*numNextPermutations >= PERMUTATIONS_SIZE)
            {
                printf("ERROR: Too many permutations\n");
                exit(1);
            }
        }
    }
}

//bool IsPermutationInOvershooting(unsigned int permutation[MAX_NUM_BUTTONS], )

void FindButtonToPressForJoltage(int joltages[MAX_NUM_JOLTAGES], unsigned int numJoltages, Button buttons[MAX_NUM_BUTTONS], unsigned int numButtons, unsigned int* buttonIndexToPress, unsigned int* numberOfTimesToPress)
{
    unsigned int minJoltage = UINT_MAX;
    unsigned int minJoltageIndex = 0;

    for (int j = 0; j < numJoltages; ++j)
    {
        if (joltages[j] < minJoltage)
        {
            minJoltage = joltages[j];
            minJoltageIndex = j;
        }
    }

    unsigned int possibleButtons[MAX_NUM_BUTTONS] = {0};
    unsigned int numPossibleButtons = 0;

    for (unsigned int b = 0; b < numButtons; ++b)
    {
        Button button = buttons[b];
        for (unsigned int i = 0; i < button.numConnectedJoltages; ++i)
        {
            if (button.connectedJoltages[i] == minJoltageIndex)
            {
                possibleButtons[numPossibleButtons++] = b;
                printf("%d is possible\n", b);
                continue;
            }
        }
    }

    unsigned int maxNumConnectedJoltages = 0;
    unsigned int bestButtonIndex = 0;

    for (unsigned int b = 0; b < numPossibleButtons; ++b)
    {
        Button button = buttons[b];
        if (button.numConnectedJoltages > maxNumConnectedJoltages)
        {
            bestButtonIndex = possibleButtons[b];
            maxNumConnectedJoltages = button.numConnectedJoltages;
        }
    }

    *buttonIndexToPress = bestButtonIndex;
    *numberOfTimesToPress = minJoltage;
}

bool DoesPermutationEndWithInvalidPermutation(Permutation permutation, Permutation (*invalidPermutations)[PERMUTATIONS_SIZE], unsigned int numInvalidPermutations)
{
    for (int i = 0; i < numInvalidPermutations; ++i)
    {
        Permutation invalidPermutation = (*invalidPermutations)[i];
        bool endsWithThisPermutation = true;

        for (int j = 0; j < invalidPermutation.numIndices; ++j)
        {
            int indexInPermutation = permutation.numIndices - (j + 1);
            int indexInInvalidPermutation = invalidPermutation.numIndices - (j + 1);
            if (permutation.buttonIndices[indexInPermutation] != invalidPermutation.buttonIndices[indexInInvalidPermutation])
            {
                endsWithThisPermutation = false;
                break;
            }
        }

        if (endsWithThisPermutation)
        {
            return true;
        }
    }

    return false;
}

int main()
{
    char* filePath = "../resources/input.txt";
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at filepath '%s'\n", filePath);
        exit(1);
    }

    Machine machines[MAX_NUM_MACHINES] = {0};
    unsigned int numMachines = 0;

    char lineBuffer[MAX_LINE_LENGTH] = {0};

    while (fgets(lineBuffer, MAX_LINE_LENGTH, file) != NULL)
    {
        Button b = {0};
        Machine machine = {0};
        bool parsingJoltages = false;

        char numberString[5] = {0};
        unsigned int numberStringSize = 0;

        for (int i = 0; i < strlen(lineBuffer); ++i)
        {
            char c = lineBuffer[i];

            switch (c)
            {
                case '.':
                    machine.lightsRequiredState[machine.numLights++] = false;
                    break;
                case '#':
                    machine.lightsRequiredState[machine.numLights++] = true;
                    break;
                case '(':
                    assert(parsingJoltages == false);
                    memset(&b, 0, sizeof(b));
                    break;
                case ')':
                    assert(parsingJoltages == false);
                    b.connectedJoltages[b.numConnectedJoltages++] = atoi(numberString);
                    machine.buttons[machine.numButtons++] = b;
                    memset(numberString, 0, sizeof(numberString));
                    numberStringSize = 0;
                    break;
                case '{':
                    parsingJoltages = true;
                    break;
                case '}':
                    assert(parsingJoltages);
                    machine.joltages[machine.numJoltages++] = atoi(numberString);
                    memset(numberString, 0, sizeof(numberString));
                    numberStringSize = 0;
                    break;
                case ',':
                    if (parsingJoltages)
                    {
                        machine.joltages[machine.numJoltages++] = atoi(numberString);
                        memset(numberString, 0, sizeof(numberString));
                        numberStringSize = 0;
                    }
                    else
                    {
                        b.connectedJoltages[b.numConnectedJoltages++] = atoi(numberString);
                        memset(numberString, 0, sizeof(numberString));
                        numberStringSize = 0;
                    }
                    break;
                case '[':
                case ']':
                case '\n':
                case ' ':
                case 0:
                    // ignore
                    break;
                default:
                    assert(c >= '0' && c <= '9');
                    numberString[numberStringSize++] = c;
                    break;
            }
        }

        machines[numMachines++] = machine;
    }

    unsigned int totalButtonPressesForLights = 0;
    uint64_t totalButtonPressesForJoltage = 0;

    for (int m = 0; m < numMachines; ++m)
    //for (int m = 0; m < 1; ++m)
    {
        Machine machine = machines[m];

        printf("\nMACHINE [%d]\n\n", m);

        bool foundCorrectLightState = false;
        bool foundCorrectJoltageLevels = false;
        unsigned int numButtonPresses = 0;

        //unsigned int buttonIndexToPress = 0;
        //unsigned int timesToPress = 0;
        //FindButtonToPressForJoltage(machine.joltages, machine.numJoltages, machine.buttons, machine.numButtons, &buttonIndexToPress, &timesToPress);
        //printf("PRESS %d for %d times\n", buttonIndexToPress, timesToPress);
        //for (int i = 0; i < timesToPress; ++i)
        //{
        //    ApplyButtonPressToJoltages(&machine.joltages, machine.buttons[buttonIndexToPress]);
        //}

        unsigned int numPermutations = 0;
        unsigned int numNextPermutations = 0;

        Permutation(*buttonIndexPermutations)[PERMUTATIONS_SIZE] = calloc(PERMUTATIONS_SIZE, sizeof(Permutation));
        Permutation(*nextButtonIndexPermutations)[PERMUTATIONS_SIZE] = calloc(PERMUTATIONS_SIZE, sizeof(Permutation));
        Permutation(*overshootingPermutations)[PERMUTATIONS_SIZE] = calloc(PERMUTATIONS_SIZE, sizeof(Permutation));
        uint64_t numOvershootingPermutations = 0;

        for (numButtonPresses = 1; foundCorrectJoltageLevels == false; ++numButtonPresses)
        //for (numButtonPresses = 1; numButtonPresses < 6; ++numButtonPresses)
        {
            unsigned int numNextPermutations = 0;

            AddNextButtonWithDuplicates(buttonIndexPermutations, numPermutations, nextButtonIndexPermutations, &numNextPermutations, 0, machine.numButtons, numButtonPresses - 1);

            printf("%d permutations\n", numNextPermutations);

            numPermutations = 0;

            //unsigned int (*temp)[PERMUTATIONS_SIZE][MAX_NUM_BUTTONS] = buttonIndexPermutations;
            //buttonIndexPermutations = nextButtonIndexPermutations;
            //nextButtonIndexPermutations = temp;

            //printf("NUM PERMUTATIONS = %d\n", numNextPermutations);

            for (int i = 0; i < numNextPermutations; ++i)
            {
                int joltages[MAX_NUM_JOLTAGES] = {0};

                Permutation permutation = (*nextButtonIndexPermutations)[i];

                for (int b = 0; b < numButtonPresses; ++b)
                {
                    Button button = machine.buttons[permutation.buttonIndices[b]];
                    ApplyButtonPressToJoltages(&joltages, button);
                    //printf("%u ", permutation.buttonIndices[b]);
                }

                //printf(" -> ");

                bool areJoltagesLevelsCorrect = true;
                bool isJoltagesOvershot = false;

                for (int j = 0; j < machine.numJoltages; ++j)
                {
                    //printf("%u ", joltages[j]);

                    if (joltages[j] > machine.joltages[j])
                    {
                        //printf("[OVERFLOW | exceeds %d] \n", machine.joltages[j]);
                        isJoltagesOvershot = true;
                        areJoltagesLevelsCorrect = false;
                        break;
                    }

                    if (joltages[j] != machine.joltages[j])
                    {
                        areJoltagesLevelsCorrect = false;
                        break;
                    }
                }

                if (areJoltagesLevelsCorrect)
                {
                    printf(" -> CORRECT\n");
                    printf("After %d button presses\n---------------------------------------\n", numButtonPresses);
                    totalButtonPressesForJoltage += numButtonPresses;
                    foundCorrectJoltageLevels = true;
                    break;
                }

                if (isJoltagesOvershot == false)
                {
                    (*buttonIndexPermutations)[numPermutations++] = permutation;
                }
                else
                {
                    (*overshootingPermutations)[numOvershootingPermutations++] = permutation;
                }

                //printf("\n");
            }

            //printf("\n");
        }

        free(buttonIndexPermutations);
        free(nextButtonIndexPermutations);
        free(overshootingPermutations);
    }

    printf("%lu\n", totalButtonPressesForJoltage);

    fclose(file);

    return 0;
}
