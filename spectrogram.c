#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define PI 3.14159265359

typedef struct WAVE {
    char chunkID[4];
    int ChunkSize;
    char Format[4];
    char SubChunk1ID[4];
    int SubChunk1Size;
    short AudioFormat;
    short numChannels;
    int SampleRate;
    int ByteRate;
    short BlockAlign;
    short BitsPerSample;
    char SubChunk2ID[4];
    int SubChunk2Size;
} Headerwav;

struct Complex {
    double real;
    double imag;
};

void applyHammingWindow(double *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] *= 0.54 - 0.46 * cos(2 * PI * i / (size - 1));
    }
}
void computeDFT(double *input, struct Complex *output, int N) {
    for (int k = 0; k < N; k++) {
        output[k].real = 0;
        output[k].imag = 0;
        for (int n = 0; n < N; n++) {
            double angle = -2 * PI * k * n / N;
            output[k].real += input[n] * cos(angle);
            output[k].imag += input[n] * sin(angle);
        }
    }
}

void generateSpectrogram(const char *wavFile, const char *outputFile, int windowSize, int frameStep, int useHamming) {
    FILE *wav = fopen(wavFile, "rb");
    if (!wav) {
        fprintf(stderr, "Error opening WAV file: %s\n", wavFile);
        exit(1);
    }

    Headerwav header;
    fread(&header, sizeof(Headerwav), 1, wav);
    int numSamples = header.SubChunk2Size / (header.BitsPerSample / 8);
    double *samples = malloc(sizeof(double) * numSamples);

    if (!samples) {
        fprintf(stderr, "Error: Memory allocation failed for samples.\n");
        fclose(wav);
        return;
    }

    for (int i = 0; i < numSamples; i++) {
        short temp;
        fread(&temp, sizeof(short), 1, wav);
        samples[i] = temp;
    }
    fclose(wav);

    if (windowSize > numSamples) {
        fprintf(stderr, "Error: Window size (%d) is larger than input size (%d).\n", windowSize, numSamples);
        free(samples);
        return;
    }

    int numFrames = (numSamples - windowSize) / frameStep + 1;
    if (numFrames > 30000) { // 提高最大框架限制
        fprintf(stderr, "Warning: Too many frames (%d). This may take longer.\n", numFrames);
    }

    FILE *output = fopen(outputFile, "w");
    if (!output) {
        fprintf(stderr, "Error opening output file: %s\n", outputFile);
        free(samples);
        return;
    }

    struct Complex *dftOutput = malloc(sizeof(struct Complex) * windowSize);
    if (!dftOutput) {
        fprintf(stderr, "Error: Memory allocation failed for DFT output.\n");
        free(samples);
        fclose(output);
        return;
    }

    for (int i = 0; i < numFrames; i++) {
        double *frame = &samples[i * frameStep];
        if (useHamming) {
            applyHammingWindow(frame, windowSize);
        }

        computeDFT(frame, dftOutput, windowSize);

        for (int k = 0; k < windowSize / 2; k++) {
            double magnitude = sqrt(dftOutput[k].real * dftOutput[k].real + dftOutput[k].imag * dftOutput[k].imag);
            fprintf(output, "%.6f ", 20 * log10(magnitude + 1e-6));
        }
        fprintf(output, "\n");
    }

    free(samples);
    free(dftOutput);
    fclose(output);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s <windowSize> <windowType> <dftSize> <frameStep> <wavFile> <outputFile>\n", argv[0]);
        return 1;
    }

    int windowSize = atoi(argv[1]);
    int frameStep = atoi(argv[4]);
    int useHamming = strcmp(argv[2], "hamming") == 0;

    generateSpectrogram(argv[5], argv[6], windowSize, frameStep, useHamming);
    return 0;
}


