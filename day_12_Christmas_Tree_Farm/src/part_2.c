#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LABEL_SIZE 4
#define MAX_NUM_DEVICES 650
#define MAX_LINE_LENGTH 128
#define MAX_NUM_OUTPUTS 32

#define ROOT_LABEL "svr"
#define TARGET_LABEL "out"

typedef struct Device
{
    char label[4];
    struct Device* outputDevices[MAX_NUM_OUTPUTS];
    uint64_t numOutputDevices;
    uint64_t numPathsToTarget;
    uint64_t numPathsThroughDacAndFftDownstream;
    uint64_t numPathsThroughDacDownstream;
    uint64_t numPathsThroughFftDownstream;
} Device;

void TraverseTree(Device* const parentDevice, const Device* const targetDevice, Device* parents[MAX_NUM_DEVICES], unsigned int numParents)
{
    for (int i = 0; i < numParents; ++i)
    {
        Device* alreadyCheckedParent = parents[i];
        if (alreadyCheckedParent == parentDevice)
        {
            printf("\nERROR: Found looping device %s\n", parentDevice->label);
            exit(1);
        }
        printf("%s - ", alreadyCheckedParent->label);
    }
    printf("%s\n", parentDevice->label);

    parents[numParents++] = parentDevice;

    for (int i = 0; i < parentDevice->numOutputDevices; ++i)
    {
        Device* childDevice = parentDevice->outputDevices[i];

        if (childDevice == targetDevice)
        {
            parentDevice->numPathsToTarget++;
            printf("Found target [%s]\n", TARGET_LABEL);
            continue;
        }

        if (childDevice->numPathsToTarget > 0)
        {
            printf("Already checked %s before. It has %lu paths to %s and %lu of those pass DAC and FFT downstream\n", childDevice->label, childDevice->numPathsToTarget, TARGET_LABEL, childDevice->numPathsThroughDacAndFftDownstream);
        }
        else
        {
            TraverseTree(childDevice, targetDevice, parents, numParents);
        }

        if (strcmp(childDevice->label, "dac") == 0)
        {
            printf("DAC has %lu paths to target\n", childDevice->numPathsToTarget);
            printf("---> and %lu run through fft downstream\n", childDevice->numPathsThroughFftDownstream);
            parentDevice->numPathsThroughDacDownstream = childDevice->numPathsToTarget;
            parentDevice->numPathsThroughDacAndFftDownstream += childDevice->numPathsThroughFftDownstream;
        }
        else if (strcmp(childDevice->label, "fft") == 0)
        {
            printf("FFT has %lu paths to target\n", childDevice->numPathsToTarget);
            printf("---> and %lu run through dac downstream\n", childDevice->numPathsThroughDacDownstream);
            parentDevice->numPathsThroughFftDownstream = childDevice->numPathsToTarget;
            parentDevice->numPathsThroughDacAndFftDownstream += childDevice->numPathsThroughDacDownstream;
        }

        parentDevice->numPathsToTarget += childDevice->numPathsToTarget;
        parentDevice->numPathsThroughDacAndFftDownstream += childDevice->numPathsThroughDacAndFftDownstream;
        parentDevice->numPathsThroughDacDownstream += childDevice->numPathsThroughDacDownstream;
        parentDevice->numPathsThroughFftDownstream += childDevice->numPathsThroughFftDownstream;
    }

    printf("[%s] has %lu paths to %s and %lu of those pass DAC and FFT downstream\n\n", parentDevice->label, parentDevice->numPathsToTarget, TARGET_LABEL, parentDevice->numPathsThroughDacAndFftDownstream);
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

    Device* parents[MAX_NUM_DEVICES];

    TraverseTree(rootDevice, targetDevice, parents, 0);

    printf("%lu\n", rootDevice->numPathsToTarget);
    printf("%lu\n", rootDevice->numPathsThroughDacAndFftDownstream);

    return 0;
}
