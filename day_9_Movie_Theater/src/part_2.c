#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TILES 512

typedef enum
{
    Empty,
    Corner,
} TileType;

typedef struct
{
    int x, y;
} Tile;

bool IsTileInPolygon(const Tile tile, const Tile corners[MAX_TILES], const unsigned int numCorners)
{
    bool insidePolygon = false;

    for (int i = 0; i < numCorners; ++i)
    {
        Tile corner = corners[i];

        int nextCornerIndex = i + 1;
        if (nextCornerIndex >= numCorners)
        {
            nextCornerIndex -= numCorners;
        }

        Tile nextCorner = corners[nextCornerIndex];

        // check how many vertical lines to our left we cross

        unsigned int edgeMinX = corner.x < nextCorner.x ? corner.x : nextCorner.x;
        unsigned int edgeMaxX = corner.x > nextCorner.x ? corner.x : nextCorner.x;
        unsigned int edgeMinY = corner.y < nextCorner.y ? corner.y : nextCorner.y;
        unsigned int edgeMaxY = corner.y > nextCorner.y ? corner.y : nextCorner.y;

        if ((tile.x == corner.x && tile.x == nextCorner.x && (tile.y >= edgeMinY && tile.y <= edgeMaxY)) ||
            (tile.y == corner.y && tile.y == nextCorner.y && (tile.x >= edgeMinX && tile.x <= edgeMaxX)))
        {
            // On an edge -> we're inside
            return true;
        }

        if (corner.y < tile.y == nextCorner.y < tile.y)
        {
            // Never going to cross -> ignore
            continue;
        }

        if (corner.x < tile.x && nextCorner.x < tile.x)
        {
            insidePolygon = !insidePolygon;
        }
    }

    return insidePolygon;
}

bool DoesLineCrossRectangle(const Tile lineStart, const Tile lineEnd, const Tile rectLowerCorner, const Tile rectHigherCorner)
{
    if (lineStart.y == lineEnd.y) // horizontal line
    {
        if (lineStart.y <= rectLowerCorner.y || lineStart.y >= rectHigherCorner.y)
            return false;

        unsigned int lineMaxX = lineStart.x > lineEnd.x ? lineStart.x : lineEnd.x;
        unsigned int lineMinX = lineStart.x < lineEnd.x ? lineStart.x : lineEnd.x;

        return lineMinX < rectHigherCorner.x && lineMaxX > rectLowerCorner.x;
    }
    else // vertical
    {
        if (lineStart.x <= rectLowerCorner.x || lineStart.x >= rectHigherCorner.x)
            return false;

        unsigned int lineMaxY = lineStart.y > lineEnd.y ? lineStart.y : lineEnd.y;
        unsigned int lineMinY = lineStart.y < lineEnd.y ? lineStart.y : lineEnd.y;

        return lineMinY < rectHigherCorner.y && lineMaxY > rectLowerCorner.y;
    }
}

bool IsRectangleInsidePolygon(const Tile corner1, const Tile corner2, const Tile corners[MAX_TILES], unsigned int numCorners)
{
    Tile lowerCorner = {.x = corner1.x < corner2.x ? corner1.x : corner2.x, .y = corner1.y < corner2.y ? corner1.y : corner2.y};
    Tile higherCorner = {.x = corner1.x > corner2.x ? corner1.x : corner2.x, .y = corner1.y > corner2.y ? corner1.y : corner2.y};

    Tile rectCorners[] =
        {
            {.x = corner1.x, .y = corner1.y},
            {.x = corner2.x, .y = corner2.y},
            {.x = corner1.x, .y = corner2.y},
            {.x = corner2.x, .y = corner1.y}};

    for (int i = 0; i < 4; ++i)
    {
        Tile rectCorner = rectCorners[i];
        if (IsTileInPolygon(rectCorner, corners, numCorners) == false)
        {
            return false;
        }
    }

    for (int i = 0; i < numCorners; ++i)
    {
        Tile corner = corners[i];
        int nextCornerIndex = i + 1;
        if (nextCornerIndex >= numCorners)
        {
            nextCornerIndex -= numCorners;
        }

        Tile nextCorner = corners[nextCornerIndex];

        if (DoesLineCrossRectangle(corner, nextCorner, lowerCorner, higherCorner))
        {
            return false;
        }
    }

    return true;
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

    Tile cornerTiles[MAX_TILES] = {0};
    unsigned int numCornerTiles = 0;
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
                cornerTiles[numCornerTiles++] = tile;
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

    Tile tile1 = cornerTiles[0];
    Tile tile2 = tile1;

    for (int i = 0; i < numCornerTiles; ++i)
    {
        Tile tile1ToCheck = cornerTiles[i];

        for (int j = i + 1; j < numCornerTiles; ++j)
        {
            Tile tile2ToCheck = cornerTiles[j];

            uint64_t area = (llabs(tile1ToCheck.x - tile2ToCheck.x) + 1) * (llabs(tile1ToCheck.y - tile2ToCheck.y) + 1);

            if (area > maxArea)
            {
                if (IsRectangleInsidePolygon(tile1ToCheck, tile2ToCheck, cornerTiles, numCornerTiles) == false)
                    continue;

                tile1 = tile1ToCheck;
                tile2 = tile2ToCheck;
                maxArea = area;
            }
        }
    }

    printf("Largest area is %lu between (%d, %d) and (%d, %d)\n", maxArea, tile1.x, tile1.y, tile2.x, tile2.y);

    return 0;
}
