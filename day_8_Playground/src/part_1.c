#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAIRS_TO_COUNT 1000
#define MAX_BOXES 1024
#define NUMBER_OF_CIRCUITS_TO_MULTIPLY 3

typedef struct JunctionBox
{
    int x, y, z;
    unsigned int connectedBoxesSize;
    struct JunctionBox* connectedBoxes[MAX_BOXES];
} JunctionBox;

typedef struct
{
    unsigned int boxCount;
    const JunctionBox* rootBox;
} BoxCircuit;

bool CircuitContainsBox(const JunctionBox* parentBoxes[MAX_BOXES], unsigned int parentBoxesSize, const JunctionBox* const boxToFind)
{
    const JunctionBox* parentBox = parentBoxes[parentBoxesSize - 1];

    for (int i = 0; i < parentBox->connectedBoxesSize; ++i)
    {
        const JunctionBox* const nextBox = parentBox->connectedBoxes[i];

        bool alreadyChecked = false;

        for (int j = 0; j < parentBoxesSize; ++j)
        {
            if (nextBox == parentBoxes[j])
            {
                alreadyChecked = true;
            }
        }

        if (alreadyChecked)
            continue;

        if (nextBox == boxToFind)
        {
            printf("Box (%d, %d, %d), is part of circuit with root (%d, %d, %d), and direct parent (%d, %d, %d)\n", nextBox->x, nextBox->y, nextBox->z, parentBoxes[0]->x, parentBoxes[0]->y, parentBoxes[0]->z, parentBoxes[parentBoxesSize]->x, parentBoxes[parentBoxesSize]->y, parentBoxes[parentBoxesSize]->z);
            return true;
        }

        parentBoxes[parentBoxesSize] = nextBox;

        if (CircuitContainsBox(parentBoxes, parentBoxesSize + 1, boxToFind))
        {
            return true;
        }
    }

    return false;
}

void PrintCircuit(const JunctionBox* (*parentBoxes)[MAX_BOXES], unsigned int* parentBoxesSize)
{
    const JunctionBox* parentBox = (*parentBoxes)[*parentBoxesSize - 1];
    printf("(%d, %d, %d) --- ", parentBox->x, parentBox->y, parentBox->z);

    for (int i = 0; i < parentBox->connectedBoxesSize; ++i)
    {
        const JunctionBox* const nextBox = parentBox->connectedBoxes[i];

        bool alreadyPrinted = false;

        for (int j = 0; j < *parentBoxesSize; ++j)
        {
            if (nextBox == (*parentBoxes)[j])
            {
                alreadyPrinted = true;
            }
        }

        if (alreadyPrinted)
            continue;

        (*parentBoxes)[*parentBoxesSize] = nextBox;
        (*parentBoxesSize) += 1;

        PrintCircuit(parentBoxes, parentBoxesSize);
    }
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

    JunctionBox* boxes = malloc(sizeof(JunctionBox) * MAX_BOXES);
    unsigned int boxesSize = 0;
    char lineBuffer[32] = {0};

    while (fgets(lineBuffer, 32, file) != NULL)
    {
        JunctionBox box = {0};

        int* coordinateToSet = &box.x;

        char numberString[8] = {0};
        unsigned int numberSize = 0;
        char c = 0;

        for (int i = 0; i < strlen(lineBuffer); ++i)
        {
            c = lineBuffer[i];

            if (c == '\n')
            {
                *coordinateToSet = atoi(numberString);
                boxes[boxesSize] = box;
                ++boxesSize;
                continue;
            }

            if (c == ',')
            {
                *coordinateToSet = atoi(numberString);
                memset(numberString, 0, sizeof(numberString));
                numberSize = 0;
                coordinateToSet += 1;
                continue;
            }

            assert(c >= '0' && c <= '9');
            numberString[numberSize] = c;
            ++numberSize;
        }
    }

    fclose(file);

    for (int pair = 0; pair < PAIRS_TO_COUNT; ++pair)
    {
        float minDistance = FLT_MAX;

        JunctionBox* box1 = &boxes[0];
        JunctionBox* box2 = box1;

        for (int i = 0; i < boxesSize; ++i)
        {
            JunctionBox* box1ToCheck = &boxes[i];

            for (int j = i + 1; j < boxesSize; ++j)
            {
                JunctionBox* box2ToCheck = &boxes[j];

                bool alreadyConnected = false;

                for (int c = 0; c < box1ToCheck->connectedBoxesSize; ++c)
                {
                    if (box1ToCheck->connectedBoxes[c] == box2ToCheck)
                    {
                        alreadyConnected = true;
                        break;
                    }
                }

                if (alreadyConnected)
                    continue;

                float distance = sqrtf(powf(box2ToCheck->x - box1ToCheck->x, 2.0f) + powf(box2ToCheck->y - box1ToCheck->y, 2.0f) + powf(box2ToCheck->z - box1ToCheck->z, 2.0f));

                //printf("Distance: box (%u, %u, %u)[%d] and box (%u, %u, %u)[%d] = %f\n", box1ToCheck->x, box1ToCheck->y, box1ToCheck->z, box1ToCheck.circuitId, box2ToCheck.x, box2ToCheck.y, box2ToCheck.z, box2ToCheck.circuitId, distance);

                if (distance < minDistance)
                {
                    box1 = box1ToCheck;
                    box2 = box2ToCheck;
                    minDistance = distance;
                }
            }
        }

        printf("Joining box (%u, %u, %u) and box (%u, %u, %u)\n", box1->x, box1->y, box1->z, box2->x, box2->y, box2->z);

        box1->connectedBoxes[box1->connectedBoxesSize++] = box2;
        box2->connectedBoxes[box2->connectedBoxesSize++] = box1;
    }

    unsigned int numCircuits = 0;
    BoxCircuit circuits[MAX_BOXES] = {0};
    const JunctionBox* previouslyCheckedBoxes[MAX_BOXES] = {0};
    unsigned int biggestCircuitSizes[NUMBER_OF_CIRCUITS_TO_MULTIPLY] = {0};

    printf("\n");

    //TODO: Optimize this. Just pick a root box, and generate the circuit that way.
    // -> assign each box a circuit ID to keep track if we already checked it.

    for (int i = 0; i < boxesSize; ++i)
    {
        const JunctionBox* box = &boxes[i];

        bool isBoxAlreadyPartOfCircuit = false;
        for (int c = 0; c < numCircuits; ++c)
        {
            previouslyCheckedBoxes[0] = circuits[c].rootBox;
            if (CircuitContainsBox(previouslyCheckedBoxes, 1, box))
            {
                isBoxAlreadyPartOfCircuit = true;
                circuits[c].boxCount++;
                break;
            }
        }

        if (!isBoxAlreadyPartOfCircuit)
        {
            printf("NEW circuit with root (%d, %d, %d)\n", box->x, box->y, box->z);
            BoxCircuit newCircuit = {.rootBox = box, .boxCount = 1};
            circuits[numCircuits++] = newCircuit;
        }
    }

    for (int c = 0; c < numCircuits; ++c)
    {
        BoxCircuit circuit = circuits[c];
        printf("Circuit: root = (%d, %d, %d) | BoxCount = %u\n", circuit.rootBox->x, circuit.rootBox->y, circuit.rootBox->z, circuit.boxCount);
        const JunctionBox* parentBoxes[MAX_BOXES] = {0};
        parentBoxes[0] = circuit.rootBox;
        unsigned int parentBoxesSize = 1;
        PrintCircuit(&parentBoxes, &parentBoxesSize);
        printf("\nFollowed connections (%d) equals circuit size (%d) ?\n", parentBoxesSize, circuit.boxCount);
        assert(parentBoxesSize == circuit.boxCount);
        printf("\n\n");

        bool isPartOfBigCircuits = false;
        unsigned int previousCircuitSize = 0;
        unsigned int tricklingDownCircuitSize = 0;

        for (int i = 0; i < NUMBER_OF_CIRCUITS_TO_MULTIPLY; ++i)
        {
            if (isPartOfBigCircuits)
            {
                previousCircuitSize = biggestCircuitSizes[i];
                biggestCircuitSizes[i] = tricklingDownCircuitSize;
                tricklingDownCircuitSize = previousCircuitSize;
            }
            else if (circuit.boxCount > biggestCircuitSizes[i])
            {
                isPartOfBigCircuits = true;
                tricklingDownCircuitSize = biggestCircuitSizes[i];
                biggestCircuitSizes[i] = circuit.boxCount;
            }
        }
    }

    uint64_t product = 1;

    for (int i = 0; i < NUMBER_OF_CIRCUITS_TO_MULTIPLY; ++i)
    {
        printf("%d * \n", biggestCircuitSizes[i]);
        product *= biggestCircuitSizes[i];
    }

    printf("%lu\n", product);

    free(boxes);

    return 0;
}
