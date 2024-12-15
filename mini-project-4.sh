#!/bin/bash

set -e  # 遇到錯誤立即停止
set -x  # 顯示執行過程

# =============================
# 清理舊文件
# =============================
echo "清理舊文件..."
rm -rf scp scp_8k                     # 刪除舊的 scp 和 scp_8k 目錄
rm -f sinegen cascade spectrogram     # 刪除舊的執行檔
rm -f scp_16k.txt scp_8k.txt          # 刪除舊的 SCP 文件
rm -f s-16k.wav s-8k.wav              # 刪除舊的 WAV 文件
rm -f *.txt *.pdf                     # 刪除舊的 TXT 和 PDF 文件
mkdir -p scp scp_8k                   # 創建新的目錄

# =============================
# 編譯 C 程式
# =============================
echo "編譯 C 程式..."
gcc -o sinegen sinegen.c -lm || { echo "錯誤：編譯 sinegen 失敗"; exit 1; }
gcc -o cascade cascade.c -lm || { echo "錯誤：編譯 cascade 失敗"; exit 1; }
gcc -o spectrogram spectrogram.c -lm || { echo "錯誤：編譯 spectrogram 失敗"; exit 1; }

# =============================
# 生成 16kHz 和 8kHz WAV 文件
# =============================
types=(sine sawtooth square triangle) # 波形類型
frequencies=(0 31.25 500 2000 4000 44 220 440 1760 3960) # 頻率設置
amplitudes=(100 2000 1000 500 250 100 2000 1000 500 250) # 振幅設置

# 生成 16kHz WAV 文件
echo "生成 16kHz WAV 文件..."
> scp_16k.txt                         # 清空或創建 SCP 文件
index=1
for type in "${types[@]}"; do
    for i in "${!frequencies[@]}"; do
        freq="${frequencies[i]}"
        amp="${amplitudes[i]}"
        output="s16k_${index}.wav"    # 輸出文件名
        ./sinegen 16000 16 $type $freq $amp 0.1 "$output" # 使用 sinegen 生成波形
        mv "$output" scp/            # 移動文件到 SCP 目錄
        echo "scp/$output" >> scp_16k.txt # 添加到 SCP 列表
        index=$((index + 1))         # 更新索引
    done
done
./cascade scp_16k.txt s-16k.wav || { echo "錯誤：生成 s-16k.wav 失敗"; exit 1; }

# 生成 8kHz WAV 文件
echo "生成 8kHz WAV 文件..."
> scp_8k.txt                         # 清空或創建 SCP 文件
index=1
for type in "${types[@]}"; do
    for i in "${!frequencies[@]}"; do
        freq="${frequencies[i]}"
        amp="${amplitudes[i]}"
        output="s8k_${index}.wav"    # 輸出文件名
        ./sinegen 8000 16 $type $freq $amp 0.1 "$output" # 使用 sinegen 生成波形
        mv "$output" scp_8k/         # 移動文件到 SCP_8k 目錄
        echo "scp_8k/$output" >> scp_8k.txt # 添加到 SCP 列表
        index=$((index + 1))         # 更新索引
    done
done
./cascade scp_8k.txt s-8k.wav || { echo "錯誤：生成 s-8k.wav 失敗"; exit 1; }

# =============================
# 設置頻譜參數並生成 TXT 文件
# =============================
echo "生成頻譜 TXT 文件..."
settings=(                              # 頻譜設置參數
  "32ms rectangular 32ms 10ms"
  "32ms hamming 32ms 10ms"
  "30ms rectangular 32ms 10ms"
  "30ms hamming 32ms 10ms"
)
wav_files=("s-16k.wav" "s-8k.wav" "aeueo-16kHz.wav" "aeueo-8kHz.wav") # 處理的 WAV 文件
for wav_file in "${wav_files[@]}"; do
    for i in {0..3}; do
        IFS=' ' read -r w_size w_type dft_size f_itv <<< "${settings[$i]}"
        output_txt="${wav_file%.wav}.Set$((i+1)).txt" # 設定輸出文件名
        echo "處理 $wav_file 使用 $w_size, $w_type, $dft_size, $f_itv -> $output_txt"
        ./spectrogram "$w_size" "$w_type" "$dft_size" "$f_itv" "$wav_file" "$output_txt" || {
            echo "錯誤：處理 $wav_file 使用 $w_size, $w_type 失敗"
            exit 1
        }
    done
done

# =============================
# 結果輸出
# =============================
echo "生成的 TXT 文件列表:"
ls -l *.txt                             # 列出所有 TXT 文件
echo "所有任務成功完成！"
