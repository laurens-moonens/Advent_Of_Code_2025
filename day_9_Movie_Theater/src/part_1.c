#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int x, y;
} Tile;

int main()
{
    char* filePath = "../resources/input.txt";
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Unable to open file at filepath '%s'\n", filePath);
        exit(1);
    }

    Tile tiles[512] = {0};
    unsigned int numTiles = 0;
    char lineBuffer[32] = {0};

    while (fgets(lineBuffer, 32, file) != NULL)
    {
        Tile tile = {0};

        int* coordinateToSet = &tile.x;

        char numberString[8] = {0};
        unsigned int numberSize = 0;
        char c = 0;

        for (int i = 0; i < strlen(lineBuffer); ++i)
        {
            c = lineBuffer[i];

            if (c == '\n')
            {
                *coordinateToSet = atoi(numberString);
                tiles[numTiles++] = tile;
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
            numberString[numberSize++] = c;
        }
    }

    fclose(file);

    uint64_t maxArea = 0;

    Tile tile1 = tiles[0];
    Tile tile2 = tile1;

    for (int i = 0; i < numTiles; ++i)
    {
        Tile tile1ToCheck = tiles[i];

        for (int j = i + 1; j < numTiles; ++j)
        {
            Tile tile2ToCheck = tiles[j];

            uint64_t area = (llabs(tile1ToCheck.x - tile2ToCheck.x) + 1) * (llabs(tile1ToCheck.y - tile2ToCheck.y) + 1);

            if (area > maxArea)
            {
                tile1 = tile1ToCheck;
                tile2 = tile2ToCheck;
                maxArea = area;
            }
        }
    }

    printf("Largest area is %lu between (%d, %d) and (%d, %d)\n", maxArea, tile1.x, tile1.y, tile2.x, tile2.y);

    return 0;
}
