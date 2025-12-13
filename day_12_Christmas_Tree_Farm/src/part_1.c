#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRESENT_DIMENSIONS 3
#define NUM_PRESENT_SHAPES 6
#define MAX_NUM_REGIONS 1000
#define NUM_ORIENTATIONS 8

#define MAX_NUM_DISTINCT_PRESENTS NUM_PRESENT_SHAPES * 99

#define MAX_LINE_SIZE 32

typedef struct
{
    bool occupiedSpaces[PRESENT_DIMENSIONS][PRESENT_DIMENSIONS];
} Present;

typedef struct
{
    unsigned int numOrientations;
    Present orientations[NUM_ORIENTATIONS];
} PresentOrientations;

typedef struct
{
    unsigned int x, y;
    unsigned int numDistinctPresents;
    unsigned int distinctRequiredPresents[MAX_NUM_DISTINCT_PRESENTS];
    unsigned int requiredPresentsPerIndex[NUM_PRESENT_SHAPES];
} Region;

typedef struct Node
{
    struct Node* left;
    struct Node* right;
    struct Node* up;
    struct Node* down;
    struct ColumnHeader* columnHeader;
} Node;

typedef struct ColumnHeader
{
    char name[12];
    unsigned int size;
    Node* columnHeaderNode;
    unsigned int columnIndex;
} ColumnHeader;

typedef struct
{
    Node headerNode;
    ColumnHeader headerNodeHeader;
    Node* nodes;
    ColumnHeader* columnHeaders;
} DLXGrid;

void GetOccupancyMaskForShape(bool* occupancyMask, unsigned int regionSizeX, unsigned int regionSizeY, Present present, unsigned int posX, unsigned int posY)
{
    for (int y = 0; y < PRESENT_DIMENSIONS; ++y)
    {
        for (int x = 0; x < PRESENT_DIMENSIONS; ++x)
        {
            occupancyMask[((y + posY) * regionSizeX) + x + posX] = present.occupiedSpaces[y][x];
        }
    }
}

Node* ChooseColumn(const DLXGrid* dlxGrid)
{
    unsigned int maxSize = UINT_MAX;
    Node* chosenNode = dlxGrid->headerNode.right;

    for (Node* node = dlxGrid->headerNode.right; node != dlxGrid->headerNode.right; node = node->right)
    {
        if (node->columnHeader->size < maxSize)
        {
            chosenNode = node;
            maxSize = node->columnHeader->size;
        }
    }

    return chosenNode;
}

void RemoveColumnNode(Node* nodeToRemove)
{
    //printf("Removing column node %s\n", nodeToRemove->columnHeader->name);
    nodeToRemove->left->right = nodeToRemove->right;
    //printf("Set right node of %s to %s\n", nodeToRemove->left->columnHeader->name, nodeToRemove->left->right->columnHeader->name);
    nodeToRemove->right->left = nodeToRemove->left;
    //printf("Set left node of %s to %s\n", nodeToRemove->right->columnHeader->name, nodeToRemove->right->left->columnHeader->name);

    //printf("Right node of %s is %s\n", nodeToRemove->left->columnHeader->name, nodeToRemove->left->right->columnHeader->name);
}

void RemoveRowNode(Node* nodeToRemove)
{
    //printf("Removing row node %s\n", nodeToRemove->columnHeader->name);
    nodeToRemove->up->down = nodeToRemove->down;
    nodeToRemove->down->up = nodeToRemove->up;
}

void RestoreColumnNode(Node* nodeToRestore)
{
    //printf("Restoring column node %s\n", nodeToRestore->columnHeader->name);
    nodeToRestore->left->right = nodeToRestore;
    nodeToRestore->right->left = nodeToRestore;
}

void RestoreRowNode(Node* nodeToRestore)
{
    //printf("Restoring row node %s\n", nodeToRestore->columnHeader->name);
    nodeToRestore->down->up = nodeToRestore;
    nodeToRestore->up->down = nodeToRestore;
}

void InsertNode(Node* newNode, Node* leftNode, Node* rightNode, Node* upNode, Node* downNode, ColumnHeader* columnHeader)
{
    newNode->left = leftNode;
    newNode->right = rightNode;
    newNode->up = upNode;
    newNode->down = downNode;

    leftNode->right = newNode;
    rightNode->left = newNode;
    upNode->down = newNode;
    downNode->up = newNode;

    newNode->columnHeader = columnHeader;

    if (newNode != upNode || newNode != downNode)
    {
        columnHeader->size++;
    }
}

void CoverColumn(Node* node)
{
    Node* columnHeaderNode = node->columnHeader->columnHeaderNode;
    //printf("Cover column %s\n", columnHeaderNode->columnHeader->name);
    RemoveColumnNode(columnHeaderNode);
    Node* nodeDown = columnHeaderNode->down;

    for (Node* nodeDown = columnHeaderNode->down; nodeDown != columnHeaderNode; nodeDown = nodeDown->down)
    {
        for (Node* nodeRight = nodeDown->right; nodeRight != nodeDown; nodeRight = nodeRight->right)
        {
            RemoveRowNode(nodeRight);
            nodeRight->columnHeader->size--;
        }
    }
}

void UncoverColumn(Node* node)
{
    Node* columnHeaderNode = node->columnHeader->columnHeaderNode;
    for (Node* upNode = columnHeaderNode->up; upNode != columnHeaderNode; upNode = upNode->up)
    {
        for (Node* leftNode = upNode->left; leftNode != upNode; leftNode = leftNode->left)
        {
            leftNode->columnHeader->size++;
            RestoreRowNode(leftNode);
        }
    }

    RestoreColumnNode(columnHeaderNode);
}

void PrintDLXState(const DLXGrid* dlxGrid, Node** const chosenNodes, uint64_t numChosenNodes, const Region* region)
{
    char grid[region->y * region->x];
    memset(grid, '.', sizeof(grid));

    for (int n = 0; n < numChosenNodes; ++n)
    {
        const Node* presentNode = chosenNodes[n];
        for (Node* occupancyNode = presentNode->right; occupancyNode != presentNode; occupancyNode = occupancyNode->right)
        {
            int gridIndex = occupancyNode->columnHeader->columnIndex - region->numDistinctPresents;

            if (grid[gridIndex] != '.')
            {
                grid[gridIndex] = '!' + n;
            }
            else
            {
                grid[gridIndex] = 'A' + n;
            }
        }
    }

    for (int y = 0; y < region->y; ++y)
    {
        for (int x = 0; x < region->x; ++x)
        {
            printf("%c", grid[(y * region->x) + x]);
        }
        printf("\n");
    }
}

bool SolveDLX(const DLXGrid* dlxGrid, Node** chosenNodes, uint64_t* numChosenNodes, const uint64_t numMandatoryItems, uint64_t* numTries, const Region* region)
{
    if (*numTries % 100000 == 0)
    {
        printf("%lu tries\n", *numTries);
        PrintDLXState(dlxGrid, chosenNodes, *numChosenNodes, region);
        printf("\n");
    }

    (*numTries)++;

    if (dlxGrid->headerNode.right == &dlxGrid->headerNode || dlxGrid->headerNode.right->columnHeader->columnIndex >= numMandatoryItems)
    {
        return true;
    }

    Node* chosenColumnNode = ChooseColumn(dlxGrid);

    CoverColumn(chosenColumnNode);

    for (Node* chosenRowNode = chosenColumnNode->down; chosenRowNode != chosenColumnNode; chosenRowNode = chosenRowNode->down)
    {
        chosenNodes[(*numChosenNodes)++] = chosenRowNode;

        for (Node* affectedColumnRight = chosenRowNode->right; affectedColumnRight != chosenRowNode; affectedColumnRight = affectedColumnRight->right)
        {
            CoverColumn(affectedColumnRight);
        }

        //return false;
        bool foundSolution = SolveDLX(dlxGrid, chosenNodes, numChosenNodes, numMandatoryItems, numTries, region);
        if (foundSolution)
            return true;

        (*numChosenNodes)--;

        for (Node* affectedColumnLeft = chosenRowNode->left; affectedColumnLeft != chosenRowNode; affectedColumnLeft = affectedColumnLeft->left)
        {
            UncoverColumn(affectedColumnLeft);
        }
    }

    UncoverColumn(chosenColumnNode);

    return false;
}

PresentOrientations GenerateShapeOrientations(Present defaultOrientation)
{
    PresentOrientations result = {0};

    result.orientations[result.numOrientations++] = defaultOrientation;

    Present prevRotation = defaultOrientation;

    for (int r = 0; r < 4; ++r)
    {
        Present flip = {0};

        for (int y = 0; y < PRESENT_DIMENSIONS; ++y)
        {
            for (int x = 0; x < PRESENT_DIMENSIONS; ++x)
            {
                flip.occupiedSpaces[y][x] = prevRotation.occupiedSpaces[y][PRESENT_DIMENSIONS - (x + 1)];
            }
        }

        bool isDuplicate = false;
        for (int o = 0; o < result.numOrientations; ++o)
        {
            if (memcmp(flip.occupiedSpaces, result.orientations[o].occupiedSpaces, sizeof(flip.occupiedSpaces)) == 0)
            {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate)
        {
            result.orientations[result.numOrientations++] = flip;
        }

        for (int y = 0; y < PRESENT_DIMENSIONS; ++y)
        {
            for (int x = 0; x < PRESENT_DIMENSIONS; ++x)
            {
                flip.occupiedSpaces[y][x] = prevRotation.occupiedSpaces[PRESENT_DIMENSIONS - (y + 1)][x];
            }
        }

        isDuplicate = false;
        for (int o = 0; o < result.numOrientations; ++o)
        {
            if (memcmp(flip.occupiedSpaces, result.orientations[o].occupiedSpaces, sizeof(flip.occupiedSpaces)) == 0)
            {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate)
        {
            result.orientations[result.numOrientations++] = flip;
        }

        Present rotation = {0};
        for (int y = -1; y <= 1; ++y)
        {
            for (int x = -1; x <= 1; ++x)
            {
                rotation.occupiedSpaces[y + 1][x + 1] = prevRotation.occupiedSpaces[-x + 1][y + 1];
            }
        }

        isDuplicate = false;
        for (int o = 0; o < result.numOrientations; ++o)
        {
            if (memcmp(rotation.occupiedSpaces, result.orientations[o].occupiedSpaces, sizeof(rotation.occupiedSpaces)) == 0)
            {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate)
        {
            result.orientations[result.numOrientations++] = rotation;
            prevRotation = rotation;
        }
    }

    return result;
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

    char lineBuffer[MAX_LINE_SIZE] = {0};

    PresentOrientations presents[NUM_PRESENT_SHAPES] = {0};
    unsigned int numRegions = 0;
    Region regions[MAX_NUM_REGIONS] = {0};

    for (int i = 0; i < NUM_PRESENT_SHAPES; ++i)
    {
        char* str = fgets(lineBuffer, MAX_LINE_SIZE, file);
        assert(str != NULL);
        assert(atoi(lineBuffer) == i);

        Present defaultOrientation = {0};

        for (int y = 0; y < PRESENT_DIMENSIONS; ++y)
        {
            str = fgets(lineBuffer, MAX_LINE_SIZE, file);
            assert(str != NULL);

            char c = 0;
            for (int x = 0; x < PRESENT_DIMENSIONS; ++x)
            {
                c = lineBuffer[x];

                assert(c == '#' || c == '.');

                defaultOrientation.occupiedSpaces[y][x] = (c == '#');
            }
        }

        PresentOrientations orientations = GenerateShapeOrientations(defaultOrientation);
        presents[i] = orientations;

        str = fgets(lineBuffer, MAX_LINE_SIZE, file);
    }

    memset(lineBuffer, 0, sizeof(char) * MAX_LINE_SIZE);

    while (fgets(lineBuffer, MAX_LINE_SIZE, file) != NULL)
    {
        Region region = {0};
        unsigned int numParsedPresents = 0;

        char c = 0;

        bool parsedRegionDimensions = false;
        unsigned int numberStringSize = 0;
        char numberString[3];

        for (int i = 0; i < MAX_LINE_SIZE; ++i)
        {
            c = lineBuffer[i];

            if (c == 0)
                break;

            switch (c)
            {
                case ':':
                    region.y = atoi(numberString);
                    memset(numberString, 0, sizeof(char) * 3);
                    numberStringSize = 0;
                    parsedRegionDimensions = true;
                    // consume the space after ':'
                    ++i;
                    break;
                case 'x':
                    region.x = atoi(numberString);
                    memset(numberString, 0, sizeof(char) * 3);
                    numberStringSize = 0;
                    break;
                case ' ':
                case '\n':
                    assert(parsedRegionDimensions == true);
                    unsigned int numPresentsOfThisShape = atoi(numberString);
                    region.requiredPresentsPerIndex[numParsedPresents] = numPresentsOfThisShape;
                    for (unsigned int i = 0; i < numPresentsOfThisShape; ++i)
                    {
                        region.distinctRequiredPresents[region.numDistinctPresents++] = numParsedPresents;
                    }
                    numParsedPresents++;
                    memset(numberString, 0, sizeof(char) * 3);
                    numberStringSize = 0;
                    break;
                default:
                    assert(c >= '0' && c <= '9');
                    numberString[numberStringSize++] = c;
                    break;
            }
        }

        regions[numRegions++] = region;
        memset(lineBuffer, 0, sizeof(char) * MAX_LINE_SIZE);
    }

    fclose(file);

    //for (int i = 0; i < NUM_PRESENT_SHAPES; ++i)
    //{
    //    printf("%d\n", i);

    //    PresentOrientations present = presents[i];

    //    for (int o = 0; o < present.numOrientations; ++o)
    //    {
    //        printf("Orientation %d\n", o);
    //        for (int y = 0; y < PRESENT_DIMENSIONS; ++y)
    //        {
    //            for (int x = 0; x < PRESENT_DIMENSIONS; ++x)
    //            {
    //                printf("%c", present.orientations[o].occupiedSpaces[y][x] ? '#' : '.');
    //            }

    //            printf("\n");
    //        }
    //        printf("\n");
    //    }
    //    printf("\n");
    //}

    unsigned int numSolvableRegions = 0;

    for (int r = 0; r < numRegions; ++r)
    //for (int r = 0; r < 1; ++r)
    {
        Region region = regions[r];

        printf("%dX%d -> ", region.x, region.y);

        for (int s = 0; s < NUM_PRESENT_SHAPES; ++s)
        {
            printf("%d ", region.requiredPresentsPerIndex[s]);
        }
        printf(" => ");
        for (int p = 0; p < region.numDistinctPresents; ++p)
        {
            printf("%d ", region.distinctRequiredPresents[p]);
        }

        printf("\n");

        uint64_t numItems = region.numDistinctPresents + (region.x * region.y);
        unsigned int distinctPresents[MAX_NUM_DISTINCT_PRESENTS];

        //for (int s = 0; s < NUM_PRESENT_SHAPES; ++s)
        //{
        //    unsigned int numDestinctPresentsOfThisShape = region.requiredPresentsPerIndex[s];

        //    for (int p = 0; p < numDestinctPresentsOfThisShape; ++p)
        //    {
        //        Present present = presents[s].orientations[0];

        //        for (int posY = 0; posY < (region.y - PRESENT_DIMENSIONS) + 1; ++posY)
        //        {
        //            for (int posX = 0; posX < (region.x - PRESENT_DIMENSIONS) + 1; ++posX)
        //            {
        //                bool* occupancyMask = calloc(region.y * region.x, sizeof(bool));
        //                GetOccupancyMaskForShape(occupancyMask, region.x, region.y, present, posX, posY);

        //                for (int y = 0; y < region.y; ++y)
        //                {
        //                    for (int x = 0; x < region.x; ++x)
        //                    {
        //                        printf("%c", occupancyMask[(y * region.x) + x] ? '#' : '.');
        //                    }
        //                    printf("\n");
        //                }
        //                printf("\n");

        //                free(occupancyMask);
        //            }
        //        }
        //    }
        //}

        uint64_t numDistinctPresentPositions = ((region.x - PRESENT_DIMENSIONS) + 1) * ((region.y - PRESENT_DIMENSIONS) + 1);
        uint64_t numOptions = region.numDistinctPresents * numDistinctPresentPositions;

        printf("%lu items\n", numItems);
        printf("%lu options\n", numOptions);
        printf("%d distinct presents\n", region.numDistinctPresents);

        DLXGrid dlxGrid = {0};
        dlxGrid.headerNode.columnHeader = &dlxGrid.headerNodeHeader;
        char* headerName = "Header";
        memcpy(dlxGrid.headerNodeHeader.name, headerName, strlen(headerName));
        uint64_t numNodes = 0;
        uint64_t numColumns = 0;
        uint64_t numNodesToAllocate = numOptions + numItems + 1 + (numDistinctPresentPositions * region.numDistinctPresents * PRESENT_DIMENSIONS * PRESENT_DIMENSIONS * NUM_ORIENTATIONS);
        printf("Allocating %lu nodes = %zu bytes\n", numNodesToAllocate, numNodesToAllocate * sizeof(Node));
        dlxGrid.nodes = calloc(numNodesToAllocate, sizeof(Node));
        dlxGrid.columnHeaders = calloc(numItems, sizeof(ColumnHeader));

        dlxGrid.headerNode.right = &dlxGrid.headerNode;
        dlxGrid.headerNode.left = &dlxGrid.headerNode;
        Node* node = dlxGrid.headerNode.right;
        Node* prevNode = &dlxGrid.headerNode;

        // populate column headers for distinct presents
        for (int c = 0; c < region.numDistinctPresents; ++c, node = node->right)
        {
            PresentOrientations presentOrientations = presents[region.distinctRequiredPresents[c]];
            ColumnHeader* columnHeader = &dlxGrid.columnHeaders[numColumns++];
            columnHeader->size = 0;
            columnHeader->columnIndex = c;
            sprintf(columnHeader->name, "%c [%d]", 'A' + c, region.distinctRequiredPresents[c]);
            printf("%s [%d]\n", columnHeader->name, c);

            Node* newNode = &dlxGrid.nodes[numNodes++];
            columnHeader->columnHeaderNode = newNode;
            InsertNode(newNode, prevNode, &dlxGrid.headerNode, newNode, newNode, columnHeader);
            prevNode = newNode;
        }

        // populate column headers for each region grid cell
        for (int y = 0; y < region.y; ++y)
        {
            for (int x = 0; x < region.x; ++x)
            {
                int index = (y * region.x) + x + region.numDistinctPresents;

                ColumnHeader* columnHeader = &dlxGrid.columnHeaders[numColumns++];
                columnHeader->size = 0;
                columnHeader->columnIndex = index;
                sprintf(columnHeader->name, "(%d, %d)", x, y);
                printf("%s [%d]\n", columnHeader->name, index);

                Node* newNode = &dlxGrid.nodes[numNodes++];
                columnHeader->columnHeaderNode = newNode;
                InsertNode(newNode, prevNode, &dlxGrid.headerNode, newNode, newNode, columnHeader);
                prevNode = newNode;
            }
        }

        // for every present column
        for (Node* columnHeaderNode = dlxGrid.headerNode.right; columnHeaderNode != &dlxGrid.headerNode && columnHeaderNode->columnHeader->columnIndex < region.numDistinctPresents; columnHeaderNode = columnHeaderNode->right)
        {
            Node* prevUpNode = columnHeaderNode;
            printf("Column index [%d] -> shape %d\n", columnHeaderNode->columnHeader->columnIndex, region.distinctRequiredPresents[columnHeaderNode->columnHeader->columnIndex]);
            PresentOrientations presentOrientation = presents[region.distinctRequiredPresents[columnHeaderNode->columnHeader->columnIndex]];

            // for every possible position of that present
            // TODO: include different orientations
            for (int posY = 0; posY < (region.y - PRESENT_DIMENSIONS) + 1; ++posY)
            {
                for (int posX = 0; posX < (region.x - PRESENT_DIMENSIONS) + 1; ++posX)
                {
                    for (int o = 0; o < presentOrientation.numOrientations; ++o)
                    {
                        Node* newSelectedPresentNode = &dlxGrid.nodes[numNodes++];
                        InsertNode(newSelectedPresentNode, newSelectedPresentNode, newSelectedPresentNode, prevUpNode, columnHeaderNode, columnHeaderNode->columnHeader);

                        bool* occupancyMask = calloc(region.y * region.x, sizeof(bool));
                        GetOccupancyMaskForShape(occupancyMask, region.x, region.y, presentOrientation.orientations[o], posX, posY);

                        prevUpNode->down = newSelectedPresentNode;
                        newSelectedPresentNode->up = prevUpNode;

                        Node* prevLeftNode = newSelectedPresentNode;

                        for (int y = 0; y < PRESENT_DIMENSIONS; ++y)
                        {
                            for (int x = 0; x < PRESENT_DIMENSIONS; ++x)
                            {
                                uint64_t tileIndex = ((y + posY) * region.x) + x + posX;
                                uint64_t tileColumnNodeIndex = region.numDistinctPresents + tileIndex;

                                if (occupancyMask[tileIndex] == false)
                                    continue;

                                Node* tileColumnNode = dlxGrid.columnHeaders[tileColumnNodeIndex].columnHeaderNode;
                                assert(tileColumnNode->columnHeader->columnIndex == tileColumnNodeIndex);

                                Node* newOccupancyNode = &dlxGrid.nodes[numNodes++];
                                InsertNode(newOccupancyNode, prevLeftNode, newSelectedPresentNode, tileColumnNode->up, tileColumnNode, tileColumnNode->columnHeader);
                                prevLeftNode = newOccupancyNode;
                            }
                        }

                        free(occupancyMask);

                        prevUpNode = newSelectedPresentNode;
                    }
                }
            }

            prevUpNode->down = columnHeaderNode;
            columnHeaderNode->up = prevUpNode;
        }

        for (node = dlxGrid.headerNode.right; node != &dlxGrid.headerNode; node = node->right)
        {
            printf("%s (%d options) INDEX: %d\n", node->columnHeader->name, node->columnHeader->size, node->columnHeader->columnIndex);
            printf("\t LEFT = %s\n", node->left->columnHeader->name);
            printf("\t RIGHT = %s\n", node->right->columnHeader->name);
        }

        Node* chosenNodes[region.numDistinctPresents];
        uint64_t numChosenNodes = 0;
        uint64_t numTries = 0;

        bool foundSolution = SolveDLX(&dlxGrid, chosenNodes, &numChosenNodes, region.numDistinctPresents, &numTries, &region);
        if (foundSolution)
        {
            numSolvableRegions++;
            printf("\nFOUND SOLUTION\n");
            for (uint64_t i = 0; i < numChosenNodes; ++i)
            {
                Node* rowNode = chosenNodes[i];
                printf("%s | ", rowNode->columnHeader->name);
                for (rowNode = rowNode->right; rowNode != chosenNodes[i]; rowNode = rowNode->right)
                {
                    printf("%s ", rowNode->columnHeader->name);
                }

                printf("\n");
            }
        }
        else
        {
            printf("FOUND NOOOOOO SOLUTION\n");
        }

        printf("AFTER SOLVE -----------------------\n");

        PrintDLXState(&dlxGrid, chosenNodes, numChosenNodes, &region);

        //for (node = dlxGrid.headerNode.right; node != &dlxGrid.headerNode; node = node->right)
        //{
        //    printf("%s (%d options) INDEX: %d\n", node->columnHeader->name, node->columnHeader->size, node->columnHeader->columnIndex);
        //    printf("\t LEFT = %s\n", node->left->columnHeader->name);
        //    printf("\t RIGHT = %s\n", node->right->columnHeader->name);
        //}

        printf("\n");

        printf("%u regions are solvable\n", numSolvableRegions);

        free(dlxGrid.nodes);
        free(dlxGrid.columnHeaders);
    }

    return 0;
}
