#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define PI 3.14159265358979

typedef struct AudioHeader {
    char riffID[4];
    int fileSize;
    char waveFormat[4];
    char fmtID[4];
    int fmtChunkSize;
    short audioFormat;
    short numChannels;
    int sampleRate;
    int byteRate;
    short blockAlign;
    short bitsPerSample;
    char dataID[4];
    int dataSize;
} AudioHeader;

int main(int argc, char *argv[]) {
    if (argc != 8) {
        fprintf(stderr, "Usage: %s <sampleRate> <bitDepth> <waveType> <frequency> <amplitude> <duration> <outputFile>\n", argv[0]);
        return 1;
    }

    // Parse arguments and validate
    int sampleRate = atoi(argv[1]);
    int bitDepth = atoi(argv[2]);
    char *waveType = argv[3];
    int frequency = atoi(argv[4]);
    double amplitude = atof(argv[5]);
    double duration = atof(argv[6]);
    char *filename = argv[7];

    if (sampleRate <= 0 || bitDepth <= 0 || frequency < 0 || amplitude <= 0 || duration <= 0) {
        fprintf(stderr, "Error: Invalid input parameters. Ensure all values are positive numbers.\n");
        return 1;
    }

    // Calculate the total number of samples
    size_t sampleCount = (size_t)(duration * sampleRate);
    if (sampleCount > 1000000000) { // Limit to avoid excessive memory allocation
        fprintf(stderr, "Error: sampleCount too large. Check your input values.\n");
        return 1;
    }

    // Allocate memory for samples
    short *samples16Bit = (short *)malloc(sizeof(short) * sampleCount);
    if (!samples16Bit) {
        fprintf(stderr, "Error: Memory allocation failed for samples16Bit.\n");
        return 1;
    }

    // Generate waveform
    for (size_t i = 0; i < sampleCount; i++) {
        double time = (double)i / sampleRate;
        double value = 0.0;

        if (strcmp(waveType, "sine") == 0) {
            value = amplitude * sin(2 * PI * frequency * time);
        } else if (strcmp(waveType, "square") == 0) {
            value = amplitude * (fmod(frequency * time, 1.0) < 0.5 ? 1 : -1);
        } else if (strcmp(waveType, "triangle") == 0) {
            value = amplitude * (2 * fabs(2 * (time * frequency - floor(time * frequency + 0.5))) - 1);
        } else if (strcmp(waveType, "sawtooth") == 0) {
            value = amplitude * (2 * (time * frequency - floor(time * frequency + 0.5)));
        } else {
            fprintf(stderr, "Error: Unsupported wave type %s.\n", waveType);
            free(samples16Bit);
            return 1;
        }

        samples16Bit[i] = (short)(value * 32767);
    }

    // Open output file
    FILE *outputFile = fopen(filename, "wb");
    if (!outputFile) {
        fprintf(stderr, "Error: Cannot open output file %s.\n", filename);
        free(samples16Bit);
        return 1;
    }

    // Write WAV header
    AudioHeader header;
    memcpy(header.riffID, "RIFF", 4);
    header.fileSize = sampleCount * sizeof(short) + 36;
    memcpy(header.waveFormat, "WAVE", 4);
    memcpy(header.fmtID, "fmt ", 4);
    header.fmtChunkSize = 16;
    header.audioFormat = 1;
    header.numChannels = 1;
    header.sampleRate = sampleRate;
    header.byteRate = sampleRate * sizeof(short);
    header.blockAlign = sizeof(short);
    header.bitsPerSample = bitDepth;
    memcpy(header.dataID, "data", 4);
    header.dataSize = sampleCount * sizeof(short);

    if (fwrite(&header, sizeof(header), 1, outputFile) != 1) {
        fprintf(stderr, "Error: Failed to write WAV header.\n");
        free(samples16Bit);
        fclose(outputFile);
        return 1;
    }

    // Write audio samples
    if (fwrite(samples16Bit, sizeof(short), sampleCount, outputFile) != sampleCount) {
        fprintf(stderr, "Error: Failed to write audio samples.\n");
        free(samples16Bit);
        fclose(outputFile);
        return 1;
    }

    // Clean up
    fclose(outputFile);
    free(samples16Bit);

    printf("WAV file successfully generated: %s\n", filename);
    return 0;
}
