#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define PRESENT_DIMENSIONS 3
#define NUM_PRESENT_SHAPES 6
#define MAX_NUM_REGIONS 1000
#define NUM_ORIENTATIONS 8

#define MAX_NUM_DESTINCT_PRESENTS NUM_PRESENT_SHAPES * 99

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
    unsigned int numDestinctPresents;
    unsigned int destinctRequiredPresents[MAX_NUM_DESTINCT_PRESENTS];
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

Node* ChooseColumn(DLXGrid dlxGrid)
{
    return dlxGrid.headerNode.right;
}

void RemoveColumnNode(Node* nodeToRemove)
{
    nodeToRemove->left->right = nodeToRemove->right;
    nodeToRemove->right->left = nodeToRemove->left;
}

void RemoveRowNode(Node* nodeToRemove)
{
    nodeToRemove->up->down = nodeToRemove->down;
    nodeToRemove->down->up = nodeToRemove->up;
}

void RestoreColumnNode(Node* nodeToRestore)
{
    nodeToRestore->left->right = nodeToRestore;
    nodeToRestore->right->left = nodeToRestore;
}

void RestoreRowNode(Node* nodeToRestore)
{
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

void CoverColumn(Node* columnHeaderNode)
{
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

void UncoverColumn(Node* columnHeaderNode)
{
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

void SolveDLX(DLXGrid dlxGrid, Node** chosenNodes, unsigned int numChoseNodes)
{
    unsigned int columnIndex = 0;
    Node* chosenColumnNode = ChooseColumn(dlxGrid);

    CoverColumn(chosenColumnNode);

    for (Node* chosenRowNode = chosenColumnNode->down; chosenRowNode != chosenColumnNode; chosenRowNode = chosenRowNode->down)
    {
        chosenNodes[numChoseNodes++] = chosenRowNode;

        for (Node* affectedColumnRight = chosenRowNode->right; affectedColumnRight != chosenRowNode; affectedColumnRight = affectedColumnRight->right)
        {
            CoverColumn(affectedColumnRight);
        }

        SolveDLX(dlxGrid, chosenNodes, numChoseNodes);

        for (Node* affectedColumnLeft = chosenRowNode->left; affectedColumnLeft != chosenRowNode; affectedColumnLeft = affectedColumnLeft->left)
        {
            UncoverColumn(affectedColumnLeft);
        }
    }

    UncoverColumn(chosenColumnNode);
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

        presents[i].orientations[presents[i].numOrientations++] = defaultOrientation;

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
                        region.destinctRequiredPresents[region.numDestinctPresents++] = numParsedPresents;
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

    for (int i = 0; i < NUM_PRESENT_SHAPES; ++i)
    {
        printf("%d\n", i);

        PresentOrientations present = presents[i];

        for (int y = 0; y < PRESENT_DIMENSIONS; ++y)
        {
            for (int x = 0; x < PRESENT_DIMENSIONS; ++x)
            {
                printf("%c", present.orientations[0].occupiedSpaces[y][x] ? '#' : '.');
            }

            printf("\n");
        }
        printf("\n");
    }

    //for (int r = 0; r < numRegions; ++r)
    for (int r = 0; r < 1; ++r)
    {
        Region region = regions[r];

        printf("%dX%d -> ", region.x, region.y);

        for (int s = 0; s < NUM_PRESENT_SHAPES; ++s)
        {
            printf("%d ", region.requiredPresentsPerIndex[s]);
        }
        printf(" => ");
        for (int p = 0; p < region.numDestinctPresents; ++p)
        {
            printf("%d ", region.destinctRequiredPresents[p]);
        }

        printf("\n");

        uint64_t numItems = region.numDestinctPresents + (region.x * region.y);
        unsigned int destinctPresents[MAX_NUM_DESTINCT_PRESENTS];

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

        //                //for (int y = 0; y < region.y; ++y)
        //                //{
        //                //    for (int x = 0; x < region.x; ++x)
        //                //    {
        //                //        printf("%c", occupancyMask[((y)*region.x) + x] ? '#' : '.');
        //                //    }
        //                //    printf("\n");
        //                //}
        //                //printf("\n");
        //            }
        //        }
        //    }
        //}

        uint64_t numDestinctPresentPositions = ((region.x - PRESENT_DIMENSIONS) + 1) * ((region.y - PRESENT_DIMENSIONS) + 1);
        uint64_t numOptions = region.numDestinctPresents * numDestinctPresentPositions;

        printf("%lu items\n", numItems);
        printf("%lu options\n", numOptions);
        printf("Allocating %zu bytes\n", numItems * numOptions * sizeof(Node));

        DLXGrid dlxGrid = {0};
        uint64_t numNodes = 1;
        Node* nodes = calloc((numOptions * numItems) + 1, sizeof(Node));
        ColumnHeader* columnHeaders = calloc(numItems, sizeof(ColumnHeader));

        dlxGrid.headerNode.right = &dlxGrid.headerNode;
        dlxGrid.headerNode.left = &dlxGrid.headerNode;
        Node* node = dlxGrid.headerNode.right;
        Node* prevNode = &dlxGrid.headerNode;

        for (int c = 0; c < region.numDestinctPresents; ++c, node = node->right)
        {
            //int nextC = c + 1;
            //node->right = &nodes[nextC];

            //int prevC = c - 1;
            //if (prevC <= 0)
            //{
            //    node->left = &dlxGrid.headerNode;
            //}
            //else
            //{
            //    node->left = &nodes[prevC];
            //}

            ColumnHeader* columnHeader = &columnHeaders[c];
            //node->columnHeader = columnHeader;
            columnHeader->size = 0;
            columnHeader->columnIndex = c;
            sprintf(columnHeader->name, "%c [%d]", 'A' + c, region.destinctRequiredPresents[c]);

            Node* newNode = &nodes[numNodes++];
            InsertNode(newNode, prevNode, &dlxGrid.headerNode, newNode, newNode, columnHeader);
            prevNode = newNode;
        }

        for (int y = 0; y < region.y; ++y)
        {
            for (int x = 0; x < region.x; ++x)
            {
                int index = (y * region.x) + x + region.numDestinctPresents;

                //int nextIndex = index + 1;
                //if (nextIndex >= numItems)
                //{
                //    node->right = &dlxGrid.headerNode;
                //}
                //else
                //{
                //    node->right = &nodes[nextIndex];
                //}

                //int prevIndex = index - 1;

                //node->left = &nodes[prevIndex];

                ColumnHeader* columnHeader = &columnHeaders[index];
                //node->columnHeader = columnHeader;
                columnHeader->size = 0;
                columnHeader->columnIndex = index;
                sprintf(columnHeader->name, "(%d, %d)", x, y);

                Node* newNode = &nodes[numNodes++];
                InsertNode(newNode, prevNode, &dlxGrid.headerNode, newNode, newNode, columnHeader);
                prevNode = newNode;
            }
        }

        for (Node* columnHeaderNode = dlxGrid.headerNode.right; columnHeaderNode != &dlxGrid.headerNode && columnHeaderNode->columnHeader->columnIndex < region.numDestinctPresents; columnHeaderNode = columnHeaderNode->right)
        {
            Node* upNode = columnHeaderNode;
            PresentOrientations presentOrientation = presents[columnHeaderNode->columnHeader->columnIndex];

            for (int posY = 0; posY < (region.y - PRESENT_DIMENSIONS) + 1; ++posY)
            {
                for (int posX = 0; posX < (region.x - PRESENT_DIMENSIONS) + 1; ++posX)
                {
                    for (int o = 0; o < presentOrientation.numOrientations; ++o)
                    {
                        Node* rowNode = &nodes[numNodes++];
                        ++columnHeaderNode->columnHeader->size;
                        rowNode->columnHeader = upNode->columnHeader;

                        bool* occupancyMask = calloc(region.y * region.x, sizeof(bool));
                        GetOccupancyMaskForShape(occupancyMask, region.x, region.y, presentOrientation.orientations[0], posX, posY);

                        upNode->down = rowNode;
                        rowNode->up = upNode;

                        Node* occupancyNode = rowNode;

                        for (int y = 0; y < region.y; ++y)
                        {
                            for (int x = 0; x < region.x; ++x)
                            {
                                if (occupancyMask[((y + posY) * region.x) + x + posX] == false)
                                    continue;

                                Node* prevNode = occupancyNode;

                                occupancyNode = &nodes[numNodes++];
                                occupancyNode->left = prevNode;
                                prevNode->right = occupancyNode;
                            }
                        }

                        occupancyNode->right = rowNode;

                        upNode = rowNode;
                    }
                }
            }

            upNode->down = columnHeaderNode;
            columnHeaderNode->up = upNode;
        }

        for (node = dlxGrid.headerNode.right; node != &dlxGrid.headerNode; node = node->right)
        {
            printf("%s (%d options)\n", node->columnHeader->name, node->columnHeader->size);
        }

        printf("\n");

        free(nodes);
    }

    return 0;
}
