#!/bin/bash

# 清理舊文件
echo "Cleaning up old files..."
rm -f sinegen cascade spectrogram scp.txt s-16k.wav s-8k.wav *.wav *.txt *.pdf

# 編譯 C 程式
echo "Compiling C programs..."
gcc -o sinegen sinegen.c -lm || { echo "Error: Failed to compile sinegen"; exit 1; }
gcc -o cascade cascade.c -lm || { echo "Error: Failed to compile cascade"; exit 1; }
gcc -o spectrogram spectrogram.c -lm || { echo "Error: Failed to compile spectrogram"; exit 1; }

# 生成 40 個 WAV 文件
echo "Generating 40 WAV files..."
types=(sine sawtooth square triangle)
frequencies=("0 100" "31.25 2000" "500 1000" "2000 500" "4000 250" "44 100" "220 2000" "440 1000" "1760 500" "3960 250")
index=1
> scp.txt

for type in "${types[@]}"; do
    for freq in "${frequencies[@]}"; do
        output="s${index}.wav"
        ./sinegen 16000 16 $type $freq 0.1 "$output" || { echo "Error: Failed to generate $output"; exit 1; }
        mv "$output" scp
        echo "scp/$output" >> scp.txt
        index=$((index + 1))
    done
done

# 串接 WAV 文件為 s-16k.wav 和 s-8k.wav
./cascade scp.txt s-16k.wav || { echo "Error: Failed to generate s-16k.wav"; exit 1; }

# 更新採樣率為 8k 的檔案
echo "Resampling to 8kHz..."
index=1
> scp_8k.txt
for file in scp/s*.wav; do
    output="s8k_${index}.wav"
    sox "$file" -r 8000 "$output" || { echo "Error: Failed to resample $file to $output"; exit 1; }
    mv "$output" scp
    echo "scp/$output" >> scp_8k.txt
    index=$((index + 1))
done

./cascade scp_8k.txt s-8k.wav || { echo "Error: Failed to generate s-8k.wav"; exit 1; }

# 確保所有 WAV 文件生成成功
if [ ! -f "s-16k.wav" ] || [ ! -f "s-8k.wav" ]; then
    echo "Error: Output WAV files not found"
    exit 1
fi

echo "WAV files successfully generated and merged!"
