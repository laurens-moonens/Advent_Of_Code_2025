#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUM_TILES 512

typedef struct
{
    int x, y;

} Tile;

typedef struct
{
    Tile corner1, corner2;
} Rectangle;

int sign(const int i)
{
    return (i > 0) - (i < 0);
}

bool IsTileConcave(const Tile tileToCheck, const Tile prevTile, const Tile nextTile)
{
    int diffXPrev = tileToCheck.x - prevTile.x;
    int diffYPrev = tileToCheck.y - prevTile.y;

    int diffXNext = nextTile.x - tileToCheck.x;
    int diffYNext = nextTile.y - tileToCheck.y;

    if (diffXPrev == 0)
    {
        return (sign(diffXNext) == sign(diffYPrev));
        printf("DiffYPrev = %d; DiffXNext = %d\n", diffYPrev, diffXNext);
        return (diffYPrev < 0 && diffXNext < 0) || (diffYPrev > 0 && diffXNext > 0);
    }
    else if (diffYPrev == 0)
    {
        return (sign(diffYNext) != sign(diffXPrev));
        printf("DiffXPrev = %d; DiffYNext = %d\n", diffXPrev, diffYNext);
        return (diffXPrev < 0 && diffYNext < 0) || (diffXPrev > 0 && diffYNext > 0);
    }
    else
    {
        printf("ERROR: Tile can only be checked for concaveness with adjacent tiles that share a coordinate!\n");
    }

    return false;
}

bool IsTileOnEdgeOfRectangle(Tile tileToCheck, Tile rectCorner1, Tile rectCorner2)
{
    bool betweenX = (tileToCheck.x > rectCorner1.x && tileToCheck.x < rectCorner2.x) ||
                    (tileToCheck.x > rectCorner2.x && tileToCheck.x < rectCorner1.x);

    bool betweenY = (tileToCheck.y > rectCorner1.y && tileToCheck.y < rectCorner2.y) ||
                    (tileToCheck.y > rectCorner2.y && tileToCheck.y < rectCorner1.y);

    bool onXEdge = (tileToCheck.x == rectCorner1.x || tileToCheck.x == rectCorner2.x) && betweenY;
    bool onYEdge = (tileToCheck.y == rectCorner1.y || tileToCheck.y == rectCorner2.y) && betweenX;

    return (onXEdge || onYEdge);
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

    Tile tiles[MAX_NUM_TILES] = {0};
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

    Rectangle(*rectanglesWithoutEmptySpace)[MAX_NUM_TILES * MAX_NUM_TILES] = malloc(sizeof(Rectangle) * MAX_NUM_TILES * MAX_NUM_TILES);
    uint64_t rectanglesSize = 0;

    for (int i = 0; i < numTiles; ++i)
    {
        Tile tile1ToCheck = tiles[i];

        for (int j = i + 1; j < numTiles; ++j)
        {
            Tile tile2ToCheck = tiles[j];

            bool rectHasTilesRedInside = false;

            for (int k = i + 1; k < numTiles; ++k)
            {
                if (k == j)
                    continue;

                Tile tileInRect = tiles[k];

                bool betweenX = (tileInRect.x > tile1ToCheck.x && tileInRect.x < tile2ToCheck.x) ||
                                (tileInRect.x > tile2ToCheck.x && tileInRect.x < tile1ToCheck.x);

                bool betweenY = (tileInRect.y > tile1ToCheck.y && tileInRect.y < tile2ToCheck.y) ||
                                (tileInRect.y > tile2ToCheck.y && tileInRect.y < tile1ToCheck.y);

                if (betweenX && betweenY)
                {
                    rectHasTilesRedInside = true;
                }
            }

            if (rectHasTilesRedInside)
                continue;

            (*rectanglesWithoutEmptySpace)[rectanglesSize].corner1 = tile1ToCheck;
            (*rectanglesWithoutEmptySpace)[rectanglesSize].corner2 = tile2ToCheck;
            ++rectanglesSize;

            //uint64_t area = (llabs(tile1ToCheck.x - tile2ToCheck.x) + 1) * (llabs(tile1ToCheck.y - tile2ToCheck.y) + 1);

            //if (area > maxArea)
            //{
            //    tile1 = tile1ToCheck;
            //    tile2 = tile2ToCheck;
            //    maxArea = area;
            //}
        }
    }

    Tile tilesOnEdge[MAX_NUM_TILES];

    for (int i = 0; i < numTiles; ++i)
    {
        Tile tileToCheck = tiles[i];

        bool isTileOnEdgeOfRect = IsTileOnEdgeOfRectangle(tileToCheck, tile1, tile2);

        int prevTileIndex = i - 1;
        if (prevTileIndex < 0)
            prevTileIndex += numTiles;

        int nextTileIndex = i + 1;
        if (nextTileIndex >= numTiles)
            nextTileIndex -= numTiles;

        Tile prevTile = tiles[prevTileIndex];
        Tile nextTile = tiles[nextTileIndex];

        bool nextTileOnEdge = false;

        if (isTileOnEdgeOfRect && IsTileConcave(tileToCheck, prevTile, nextTile))
        {
            printf("(%d, %d) is concave\n", tileToCheck.x, tileToCheck.y);
        }
    }

    printf("Largest area is %lu between (%d, %d) and (%d, %d)\n", maxArea, tile1.x, tile1.y, tile2.x, tile2.y);

    return 0;
}
