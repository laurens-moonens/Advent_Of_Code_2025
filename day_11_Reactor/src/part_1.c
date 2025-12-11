#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LABEL_SIZE 4
#define MAX_NUM_DEVICES 650
#define MAX_LINE_LENGTH 128
#define MAX_NUM_OUTPUTS 32

#define ROOT_LABEL "you"
#define TARGET_LABEL "out"

typedef struct Device
{
    char label[4];
    struct Device* outputDevices[MAX_NUM_OUTPUTS];
    unsigned int numOutputDevices;
} Device;

void TraverseTree(Device* const parentDevice, const Device* const targetDevice, unsigned int* numPaths)
{
    printf("[%u], checking %s\n", *numPaths, parentDevice->label);

    if (parentDevice == targetDevice)
    {
        ++(*numPaths);
        assert(parentDevice->numOutputDevices == 0);
        return;
    }

    for (int i = 0; i < parentDevice->numOutputDevices; ++i)
    {
        Device* childDevice = parentDevice->outputDevices[i];
        TraverseTree(childDevice, targetDevice, numPaths);
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

    Device devices[MAX_NUM_DEVICES] = {0};
    Device* rootDevice;
    Device* targetDevice;
    unsigned int numDevices = 0;

    char lineBuffer[MAX_LINE_LENGTH] = {0};

    // populate the devices
    while (fgets(lineBuffer, MAX_LINE_LENGTH, file) != NULL)
    {
        Device device = {0};

        char c = 0;
        unsigned int labelSize = 0;
        char label[LABEL_SIZE] = {0};

        bool setDeviceLabel = false;

        for (int i = 0; i < MAX_LINE_LENGTH; ++i)
        {
            if (setDeviceLabel)
                break;

            c = lineBuffer[i];

            switch (c)
            {
                case ':':
                    memcpy(device.label, label, LABEL_SIZE);
                    setDeviceLabel = true;
                    break;
                default:
                    assert(c >= 'a' && c <= 'z');
                    label[labelSize++] = c;
                    break;
            }
        }

        devices[numDevices++] = device;

        memset(lineBuffer, 0, MAX_LINE_LENGTH);
    }

    Device outDevice = {.label = TARGET_LABEL, .numOutputDevices = 0};
    devices[numDevices++] = outDevice;
    targetDevice = &devices[numDevices - 1];

    rewind(file);

    // link devices together
    for (int d = 0; d < numDevices && fgets(lineBuffer, MAX_LINE_LENGTH, file) != NULL; ++d)
    {
        Device* device = &devices[d];

        if (strcmp(device->label, ROOT_LABEL) == 0)
        {
            rootDevice = device;
        }

        char c = 0;
        unsigned int labelSize = 0;
        char label[LABEL_SIZE] = {0};

        printf("%s: ", device->label);

        bool skippedDeviceLabel = false;

        for (int i = 0; i < MAX_LINE_LENGTH; ++i)
        {
            c = lineBuffer[i];
            if (c == 0)
                continue;

            switch (c)
            {
                case ' ':
                case '\n':
                    if (skippedDeviceLabel == false)
                    {
                        skippedDeviceLabel = true;
                        break;
                    }

                    if (labelSize == 0)
                        break;

                    // make sure that a device is not connected to itself
                    assert(strcmp(label, device->label) != 0);

                    for (int o = 0; o < numDevices; ++o)
                    {
                        Device* otherDevice = &devices[o];

                        if (otherDevice == device)
                            continue;

                        if (strcmp(otherDevice->label, label) == 0)
                        {
                            printf("%s ", label);
                            device->outputDevices[device->numOutputDevices++] = otherDevice;
                        }
                    }

                    labelSize = 0;

                    break;

                default:
                    if (skippedDeviceLabel == false)
                    {
                        break;
                    }

                    assert(c >= 'a' && c <= 'z');
                    label[labelSize++] = c;
                    break;
            }
        }

        printf("\n");
        memset(lineBuffer, 0, MAX_LINE_LENGTH);
    }

    fclose(file);

    if (rootDevice == NULL || targetDevice == NULL)
    {
        printf("ERROR: no root or target found\n");
        exit(1);
    }

    unsigned int numPaths = 0;
    TraverseTree(rootDevice, targetDevice, &numPaths);

    printf("%u\n", numPaths);

    return 0;
}
