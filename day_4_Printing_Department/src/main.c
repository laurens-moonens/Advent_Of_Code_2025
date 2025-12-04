#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define roll '@'
#define empty '.'
#define rollWithFewNeigbours 'x'

#define neighbourThreshold 4

bool CanRollBeRemoved(unsigned int x, unsigned int y, unsigned int sizeX, unsigned int sizeY, const char (*const bufferToRead)[sizeY][sizeX])
{
    unsigned int numberOfNeighbours = 0;

    for (int ny = -1; ny <= 1; ++ny)
    {
        for (int nx = -1; nx <= 1; ++nx)
        {
            if (ny == 0 && nx == 0)
            {
                continue;
            }

            int neighbourY = y + ny;
            int neighbourX = x + nx;

            if (neighbourX < 0 || neighbourX >= sizeX || neighbourY < 0 || neighbourY >= sizeY)
            {
                continue;
            }

            char neighbour = (*bufferToRead)[neighbourY][neighbourX];

            if (neighbour != empty)
            {
                numberOfNeighbours++;
            }
        }
    }

    return numberOfNeighbours < neighbourThreshold;
}

int RemoveAllAccessibleRolls(int sizeX, int sizeY, char (*const bufferToRead)[sizeY][sizeX], char (*const bufferToWrite)[sizeY][sizeX])
{
    unsigned int numberOfRollsToRemove = 0;

    for (unsigned int y = 0; y < sizeY; ++y)
    {
        for (unsigned int x = 0; x < sizeX; ++x)
        {
            char current = (*bufferToRead)[y][x];
            if (current == empty)
            {
                (*bufferToWrite)[y][x] = current;
                printf("%c", current);
                continue;
            }

            if (CanRollBeRemoved(x, y, sizeX, sizeY, bufferToRead))
            {
                numberOfRollsToRemove++;
                (*bufferToRead)[y][x] = rollWithFewNeigbours;
                (*bufferToWrite)[y][x] = empty;
            }

            printf("%c", (*bufferToRead)[y][x]);
        }
        printf("\n");
    }

    return numberOfRollsToRemove;
}

int main()
{
    char* filePath = "../resources/input.txt";
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at filepath '%s'\n", filePath);
        exit(-1);
    }

    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int sizeX = 0;
    unsigned int sizeY = 0;

    char charToCheckLineSize;
    while ((charToCheckLineSize = fgetc(file)) != '\n')
    {
        sizeX++;
    }
    rewind(file);

    char c = fgetc(file);

    //        y    x
    char grid[256][sizeX];

    for (y = 0; c != EOF; ++y, c = fgetc(file))
    {
        for (x = 0; c != '\n'; ++x, c = fgetc(file))
        {
            grid[y][x] = c;
        }
    }

    fclose(file);

    sizeX = x;
    sizeY = y;

    char buffer1[sizeY][sizeX];
    char buffer2[sizeY][sizeX];

    memcpy(buffer1, grid, sizeY * sizeX);
    memcpy(buffer2, grid, sizeY * sizeX);

    char (*bufferToRead)[sizeY][sizeX] = &buffer1;
    char (*bufferToWrite)[sizeY][sizeX] = &buffer2;

    unsigned int numberOfRollsToRemove = 0;
    unsigned int sum = 0;

    do
    {
        numberOfRollsToRemove = RemoveAllAccessibleRolls(sizeX, sizeY, bufferToRead, bufferToWrite);

        printf("\n");
        printf("Removed %d rolls\n", numberOfRollsToRemove);
        printf("\n");

        char (*temp)[sizeY][sizeX] = bufferToRead;
        bufferToRead = bufferToWrite;
        bufferToWrite = temp;

        sum += numberOfRollsToRemove;

    } while (numberOfRollsToRemove > 0);

    printf("%d\n", sum);
}
