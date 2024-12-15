#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_SIZE 44

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <scp> <output>\n", argv[0]);
        return 1;
    }

    FILE *scpFile = fopen(argv[1], "r");
    if (!scpFile) {
        perror("Error opening scp file");
        return 1;
    }

    FILE *outputFile = fopen(argv[2], "wb");
    if (!outputFile) {
        perror("Error opening output file");
        fclose(scpFile);
        return 1;
    }

    char waveFile[256];
    int isFirstFile = 1;
    while (fgets(waveFile, sizeof(waveFile), scpFile)) {
        // Remove newline character
        waveFile[strcspn(waveFile, "\n")] = '\0';

        FILE *inputFile = fopen(waveFile, "rb");
        if (!inputFile) {
            fprintf(stderr, "Error opening input file: %s\n", waveFile);
            fclose(outputFile);
            fclose(scpFile);
            return 1;
        }

        if (isFirstFile) {
            // Copy the header of the first file
            char header[HEADER_SIZE];
            if (fread(header, sizeof(char), HEADER_SIZE, inputFile) != HEADER_SIZE) {
                fprintf(stderr, "Error reading header from %s\n", waveFile);
                fclose(inputFile);
                fclose(outputFile);
                fclose(scpFile);
                return 1;
            }
            fwrite(header, sizeof(char), HEADER_SIZE, outputFile);
            isFirstFile = 0;
        } else {
            // Skip the header for subsequent files
            fseek(inputFile, HEADER_SIZE, SEEK_SET);
        }
        // Copy audio data
        char buffer[1024];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, sizeof(char), sizeof(buffer), inputFile)) > 0) {
            if (fwrite(buffer, sizeof(char), bytesRead, outputFile) != bytesRead) {
                fprintf(stderr, "Error writing to output file\n");
                fclose(inputFile);
                fclose(outputFile);
                fclose(scpFile);
                return 1;
            }
        }
        fclose(inputFile);
    }
    fclose(outputFile);
    fclose(scpFile);

    printf("WAV files successfully merged into %s\n", argv[2]);
    return 0;
}


