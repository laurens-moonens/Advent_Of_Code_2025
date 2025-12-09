#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAIRS_TO_COUNT 10000
#define MAX_BOXES 1024
#define NUMBER_OF_CIRCUITS_TO_MULTIPLY 3

typedef struct JunctionBox
{
    int x, y, z;
    int circuitID;
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

void PopulateCircuit(BoxCircuit* circuit, JunctionBox* parent)
{
    if (circuit->rootBox == NULL)
    {
        printf("Circuit needs root box when populating it.\n");
        exit(1);
    }

    if (circuit->rootBox->circuitID == -1)
    {
        printf("Root box needs a circuit ID when populating its circuit.\n");
        exit(1);
    }

    for (int i = 0; i < parent->connectedBoxesSize; ++i)
    {
        JunctionBox* child = parent->connectedBoxes[i];

        if (child->circuitID == circuit->rootBox->circuitID)
        {
            //already part of this circuit
            continue;
        }

        // If the child has a circuit, it HAS to be the circuit we're currently populating
        assert(child->circuitID == -1);

        child->circuitID = circuit->rootBox->circuitID;
        ++(circuit->boxCount);

        PopulateCircuit(circuit, child);
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
        box.circuitID = -1;

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
    //while (true)
    {
        float minDistance = FLT_MAX;

        JunctionBox* box1 = &boxes[0];
        JunctionBox* box2 = box1;

        for (int i = 0; i < boxesSize; ++i)
        {
            JunctionBox* box1ToCheck = &boxes[i];

            box1ToCheck->circuitID = -1;

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
        unsigned int numCircuits = 0;
        BoxCircuit circuits[MAX_BOXES] = {0};
        const JunctionBox* previouslyCheckedBoxes[MAX_BOXES] = {0};
        unsigned int biggestCircuitSizes[NUMBER_OF_CIRCUITS_TO_MULTIPLY] = {0};

        printf("\n");

        for (int i = 0; i < boxesSize; ++i)
        {
            JunctionBox* box = &boxes[i];

            if (box->circuitID != -1)
            {
                printf("Box (%d, %d, %d), is part of circuit [%d]\n", box->x, box->y, box->z, box->circuitID);
                continue;
            }

            BoxCircuit* circuit = &circuits[numCircuits];
            circuit->rootBox = box;
            circuit->boxCount = 1;
            box->circuitID = numCircuits;
            printf("NEW circuit [numCircuits] with root (%d, %d, %d)\n", box->x, box->y, box->z);
            PopulateCircuit(circuit, box);
            ++numCircuits;
        }

        printf("Populated %d circuits\n", numCircuits);

        if (numCircuits == 1)
        {
            printf("Found 1 big circuit after joining (%d, %d, %d), and (%d, %d, %d)\n", box1->x, box1->y, box1->z, box2->x, box2->y, box2->z);
            printf("Multiplying those X coordinates = %d\n", box1->x * box2->x);
            break;
        }
    }

    free(boxes);

    return 0;
}
