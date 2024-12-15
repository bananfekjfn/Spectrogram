#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define PI 3.14159265359

// WAV 檔案的標頭資訊結構
typedef struct WAVE {
    char chunkID[4];        // "RIFF"標識符
    int ChunkSize;          // 檔案大小
    char Format[4];         // "WAVE"格式
    char SubChunk1ID[4];    // "fmt "子塊標識符
    int SubChunk1Size;      // 格式塊大小
    short AudioFormat;      // 音訊格式
    short numChannels;      // 通道數
    int SampleRate;         // 採樣率
    int ByteRate;           // 每秒位元組數
    short BlockAlign;       // 區塊對齊
    short BitsPerSample;    // 每個樣本的位數
    char SubChunk2ID[4];    // "data"子塊標識符
    int SubChunk2Size;      // 資料塊大小
} Headerwav;

// 複數結構，用於存儲 DFT 計算結果
struct Complex {
    double real;  // 實部
    double imag;  // 虛部
};

// 套用 Hamming 窗函數
void applyHammingWindow(double *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] *= 0.54 - 0.46 * cos(2 * PI * i / (size - 1));
    }
}

// 計算離散傅立葉變換 (DFT)
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

// 生成頻譜圖
void generateSpectrogram(const char *wavFile, const char *outputFile, int windowSize, int frameStep, int useHamming) {
    // 開啟輸入的 WAV 檔案
    FILE *wav = fopen(wavFile, "rb");
    if (!wav) {
        fprintf(stderr, "無法開啟 WAV 檔案: %s\n", wavFile);
        exit(1);
    }

    // 讀取 WAV 標頭
    Headerwav header;
    fread(&header, sizeof(Headerwav), 1, wav);
    int numSamples = header.SubChunk2Size / (header.BitsPerSample / 8); // 計算樣本數
    double *samples = malloc(sizeof(double) * numSamples); // 分配記憶體以存儲樣本

    if (!samples) {
        fprintf(stderr, "記憶體分配失敗。\n");
        fclose(wav);
        return;
    }

    // 讀取音訊樣本資料
    for (int i = 0; i < numSamples; i++) {
        short temp;
        fread(&temp, sizeof(short), 1, wav);
        samples[i] = temp;
    }
    fclose(wav);

    // 檢查窗口大小是否有效
    if (windowSize > numSamples) {
        fprintf(stderr, "窗口大小 (%d) 大於輸入大小 (%d)。\n", windowSize, numSamples);
        free(samples);
        return;
    }

    // 計算總幀數
    int numFrames = (numSamples - windowSize) / frameStep + 1;
    if (numFrames > 30000) { // 提醒過多幀數可能影響效能
        fprintf(stderr, "警告: 幀數過多 (%d)。可能耗時較長。\n", numFrames);
    }

    // 開啟輸出檔案
    FILE *output = fopen(outputFile, "w");
    if (!output) {
        fprintf(stderr, "無法開啟輸出檔案: %s\n", outputFile);
        free(samples);
        return;
    }

    // 分配記憶體存儲 DFT 輸出
    struct Complex *dftOutput = malloc(sizeof(struct Complex) * windowSize);
    if (!dftOutput) {
        fprintf(stderr, "記憶體分配失敗 (DFT 輸出)。\n");
        free(samples);
        fclose(output);
        return;
    }

    // 計算每個幀的頻譜
    for (int i = 0; i < numFrames; i++) {
        double *frame = &samples[i * frameStep]; // 提取幀
        if (useHamming) {
            applyHammingWindow(frame, windowSize); // 套用 Hamming 窗
        }

        computeDFT(frame, dftOutput, windowSize); // 計算 DFT

        for (int k = 0; k < windowSize / 2; k++) {
            double magnitude = sqrt(dftOutput[k].real * dftOutput[k].real + dftOutput[k].imag * dftOutput[k].imag); // 計算幅度
            fprintf(output, "%.6f ", 20 * log10(magnitude + 1e-6)); // 轉換為分貝並輸出
        }
        fprintf(output, "\n");
    }

    // 釋放記憶體並關閉檔案
    free(samples);
    free(dftOutput);
    fclose(output);
}

// 主程式
int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "用法: %s <窗口大小> <窗類型> <DFT大小> <幀步長> <WAV檔案> <輸出檔案>\n", argv[0]);
        return 1;
    }

    int windowSize = atoi(argv[1]); // 窗口大小
    int frameStep = atoi(argv[4]); // 幀步長
    int useHamming = strcmp(argv[2], "hamming") == 0; // 判斷是否使用 Hamming 窗

    generateSpectrogram(argv[5], argv[6], windowSize, frameStep, useHamming);
    return 0;
}
