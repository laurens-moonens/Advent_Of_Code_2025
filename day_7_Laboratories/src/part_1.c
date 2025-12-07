#include <stdio.h>
#include <stdlib.h>

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

    unsigned int numberOfSplitsEncountered = 0;

    for (unsigned int y = 0; y < sizeY; ++y)
    {
        for (unsigned int x = 0; x < sizeX; ++x)
        {
            Cell* c = &grid[y][x];

            if (y > 0)
            {
                Cell cellAbove = grid[y - 1][x];
                if (cellAbove == Source || cellAbove == Beam)
                {
                    if (*c == Empty)
                    {
                        *c = Beam;
                    }
                    else if (*c == Split)
                    {
                        numberOfSplitsEncountered++;
                        grid[y][x - 1] = Beam;
                        grid[y][x + 1] = Beam;
                    }
                }
            }
        }
    }

    for (unsigned int y = 0; y < sizeY; ++y)
    {
        for (unsigned int x = 0; x < sizeX; ++x)
        {
            Cell c = grid[y][x];

            switch (c)
            {
                case Empty:
                    printf("%c", '.');
                    break;
                case Source:
                    printf("%c", 'S');
                    break;
                case Split:
                    printf("%c", '^');
                    break;
                case Beam:
                    printf("%c", '|');
                    break;
            }
        }

        printf("\n");
    }

    printf("%u\n", numberOfSplitsEncountered);

    return 0;
}
