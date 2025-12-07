#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MaxX 256
#define MaxY 256

typedef enum
{
    Empty,
    Source,
    Split,
    Beam
} Cell;

int main()
{
    char* filePath = "../resources/input.txt";
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at filepath '%s'\n", filePath);
        exit(1);
    }

    Cell grid[MaxY][MaxX];
    unsigned int sizeX;
    unsigned int sizeY;

    char lineBuffer[MaxX];

    for (sizeY = 0; fgets(lineBuffer, MaxX, file) != NULL && sizeY < MaxY; ++sizeY)
    {
        for (sizeX = 0; sizeX < MaxX; ++sizeX)
        {
            char c = lineBuffer[sizeX];
            if (c == 0 || c == '\n')
                break;

            switch (c)
            {
                case '.':
                    grid[sizeY][sizeX] = Empty;
                    break;
                case 'S':
                    grid[sizeY][sizeX] = Source;
                    break;
                case '^':
                    grid[sizeY][sizeX] = Split;
                    break;
            }
        }
    }

    uint64_t possiblePathsPerCell[sizeY][sizeX];
    memset(possiblePathsPerCell, 0, sizeof(possiblePathsPerCell));

    uint64_t numberOfPossiblePaths = 0;

    for (unsigned int y = 0; y < sizeY; ++y)
    {
        unsigned int splitsInLayer = 0;
        for (unsigned int x = 0; x < sizeX; ++x)
        {
            Cell* c = &grid[y][x];
            uint64_t* possiblePathCount = &possiblePathsPerCell[y][x];

            if (*c == Source)
            {
                *possiblePathCount = 1;
            }

            if (y > 0)
            {
                Cell cellAbove = grid[y - 1][x];
                uint64_t pathCountAbove = possiblePathsPerCell[y - 1][x];
                if (cellAbove == Source || cellAbove == Beam)
                {
                    if (*c == Empty || *c == Beam)
                    {
                        *c = Beam;
                        *possiblePathCount += pathCountAbove;
                    }
                    else if (*c == Split)
                    {
                        splitsInLayer++;
                        //left
                        grid[y][x - 1] = Beam;
                        possiblePathsPerCell[y][x - 1] += pathCountAbove;
                        //right
                        grid[y][x + 1] = Beam;
                        possiblePathsPerCell[y][x + 1] += pathCountAbove;
                    }
                }
            }
        }
    }

    for (int i = 0; i < sizeX; ++i)
    {
        numberOfPossiblePaths += possiblePathsPerCell[sizeY - 1][i];
    }

    for (unsigned int y = 0; y < sizeY; ++y)
    {
        for (unsigned int x = 0; x < sizeX; ++x)
        {
            Cell c = grid[y][x];
            unsigned int numPossiblePaths = possiblePathsPerCell[y][x];

            if (numPossiblePaths == 0)
            {
                switch (c)
                {
                    case Empty:
                        printf(".");
                        break;
                    case Source:
                        printf("S");
                        break;
                    case Split:
                        printf("^");
                        break;
                    case Beam:
                        printf("|");
                        break;
                }
            }
            else
            {
                printf("%u", numPossiblePaths);
            }
        }

        printf("\n");
    }

    printf("%lu\n", numberOfPossiblePaths);

    return 0;
}
