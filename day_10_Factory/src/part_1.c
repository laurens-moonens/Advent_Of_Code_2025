#include <assert.h>
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
    unsigned int joltages[MAX_NUM_JOLTAGES];
    unsigned int numJoltages;
} Machine;

typedef struct
{
    bool lightStates[MAX_NUM_LIGHTS];
    bool buttonsPressed[MAX_NUM_BUTTONS];
} Permutation;

void ApplyButtonPress(bool (*lights)[MAX_NUM_LIGHTS], const Button button)
{
    for (int l = 0; l < button.numConnectedJoltages; ++l)
    {
        unsigned int lightIndex = button.connectedJoltages[l];
        (*lights)[lightIndex] = !((*lights)[lightIndex]);
    }
}

void GenerateButtonCombination(unsigned int (*permutations)[PERMUTATIONS_SIZE][MAX_NUM_BUTTONS], unsigned int currentPermutation[MAX_NUM_BUTTONS], unsigned int startButtonNumber, unsigned int* numPermutations, unsigned int numButtons, unsigned int numButtonsToPress, unsigned int currentButtonIndex)
{
    for (int i = startButtonNumber; i < numButtons + 1; ++i)
    {
        currentPermutation[currentButtonIndex] = i;

        if (currentButtonIndex >= numButtonsToPress)
        {
            memcpy((*permutations)[(*numPermutations)++], currentPermutation, sizeof(unsigned int) * MAX_NUM_BUTTONS);
            return;
        }
        GenerateButtonCombination(permutations, currentPermutation, ++startButtonNumber, numPermutations, numButtons, numButtonsToPress, currentButtonIndex + 1);
    }
}

int main()
{
    char* filePath = "../resources/example.txt";
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

    for (int m = 0; m < numMachines; ++m)
    {
        Machine machine = machines[m];

        bool foundCorrectLightState = false;
        unsigned int numButtonPresses = 0;

        for (numButtonPresses = 1; foundCorrectLightState == false; ++numButtonPresses)
        {
            unsigned int (*buttonIndices)[PERMUTATIONS_SIZE][MAX_NUM_BUTTONS] = calloc(PERMUTATIONS_SIZE * MAX_NUM_BUTTONS, sizeof(unsigned int));
            unsigned int numPermutations = 0;
            unsigned int currentPermutation[MAX_NUM_BUTTONS] = {0};

            GenerateButtonCombination(buttonIndices, currentPermutation, 0, &numPermutations, machine.numButtons, numButtonPresses, 0);

            for (int i = 0; i < numPermutations; ++i)
            {
                bool lights[MAX_NUM_LIGHTS] = {0};

                for (int b = 0; b < numButtonPresses; ++b)
                {
                    Button button = machine.buttons[(*buttonIndices)[i][b]];
                    ApplyButtonPress(&lights, button);
                    printf("%u ", (*buttonIndices)[i][b]);
                }

                printf(" -> ");

                bool isCorrectLightState = true;

                for (int l = 0; l < machine.numLights; ++l)
                {
                    printf("%c", lights[l] ? '#' : '.');
                    if (lights[l] != machine.lightsRequiredState[l])
                    {
                        isCorrectLightState = false;
                    }
                }

                if (isCorrectLightState)
                {
                    printf(" -> CORRECT\n");
                    printf("After %d button presses\n---------------------------------------\n", numButtonPresses);
                    totalButtonPressesForLights += numButtonPresses;
                    foundCorrectLightState = true;
                    break;
                }

                printf("\n");
            }

            printf("\n");
        }
    }

    printf("%u\n", totalButtonPressesForLights);

    fclose(file);

    return 0;
}
