#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define PI 3.14159265358979

// 定義 WAV 檔案的標頭結構
typedef struct AudioHeader {
    char riffID[4];        // "RIFF" 標識符
    int fileSize;          // 檔案總大小（不包括 riffID 和 fileSize）
    char waveFormat[4];    // "WAVE" 格式
    char fmtID[4];         // "fmt " 子塊標識符
    int fmtChunkSize;      // 格式子塊大小，通常為 16
    short audioFormat;     // 音訊格式（PCM 為 1）
    short numChannels;     // 通道數（單聲道為 1，立體聲為 2）
    int sampleRate;        // 採樣率（例如 44100 Hz）
    int byteRate;          // 每秒的位元組數（sampleRate * numChannels * bitsPerSample / 8）
    short blockAlign;      // 區塊對齊（numChannels * bitsPerSample / 8）
    short bitsPerSample;   // 每個樣本的位數（例如 16 位）
    char dataID[4];        // "data" 子塊標識符
    int dataSize;          // 音訊資料的大小（以位元組為單位）
} AudioHeader;

int main(int argc, char *argv[]) {
    // 檢查參數數量是否正確
    if (argc != 8) {
        fprintf(stderr, "Usage: %s <sampleRate> <bitDepth> <waveType> <frequency> <amplitude> <duration> <outputFile>\n", argv[0]);
        return 1;
    }

    // 解析輸入參數並驗證其有效性
    int sampleRate = atoi(argv[1]);      // 採樣率
    int bitDepth = atoi(argv[2]);        // 位元深度
    char *waveType = argv[3];            // 波形類型（例如 sine, square）
    int frequency = atoi(argv[4]);       // 頻率（單位：Hz）
    double amplitude = atof(argv[5]);    // 振幅（範圍：0.0 至 1.0）
    double duration = atof(argv[6]);     // 時長（單位：秒）
    char *filename = argv[7];            // 輸出檔案名稱

    if (sampleRate <= 0 || bitDepth <= 0 || frequency < 0 || amplitude <= 0 || duration <= 0) {
        fprintf(stderr, "Error: Invalid input parameters. Ensure all values are positive numbers.\n");
        return 1;
    }

    // 計算總樣本數
    size_t sampleCount = (size_t)(duration * sampleRate);
    if (sampleCount > 1000000000) { // 避免過大的記憶體分配
        fprintf(stderr, "Error: sampleCount too large. Check your input values.\n");
        return 1;
    }

    // 分配記憶體以存儲樣本資料
    short *samples16Bit = (short *)malloc(sizeof(short) * sampleCount);
    if (!samples16Bit) {
        fprintf(stderr, "Error: Memory allocation failed for samples16Bit.\n");
        return 1;
    }

    // 根據波形類型生成波形
    for (size_t i = 0; i < sampleCount; i++) {
        double time = (double)i / sampleRate; // 當前樣本的時間點
        double value = 0.0;

        // 判斷波形類型並計算對應的振幅值
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

        // 將振幅值轉換為 16 位整數格式
        samples16Bit[i] = (short)(value * 32767);
    }

    // 開啟輸出檔案
    FILE *outputFile = fopen(filename, "wb");
    if (!outputFile) {
        fprintf(stderr, "Error: Cannot open output file %s.\n", filename);
        free(samples16Bit);
        return 1;
    }

    // 初始化 WAV 標頭
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

    // 寫入 WAV 標頭
    if (fwrite(&header, sizeof(header), 1, outputFile) != 1) {
        fprintf(stderr, "Error: Failed to write WAV header.\n");
        free(samples16Bit);
        fclose(outputFile);
        return 1;
    }

    // 寫入音訊樣本
    if (fwrite(samples16Bit, sizeof(short), sampleCount, outputFile) != sampleCount) {
        fprintf(stderr, "Error: Failed to write audio samples.\n");
        free(samples16Bit);
        fclose(outputFile);
        return 1;
    }

    // 釋放資源並關閉檔案
    fclose(outputFile);
    free(samples16Bit);

    printf("WAV file successfully generated: %s\n", filename);
    return 0;
}
