# spectshow.py - 用於顯示波形和頻譜圖的腳本
import sys
import matplotlib.pyplot as plt
import numpy as np

# 檢查命令行參數的數量是否正確
if len(sys.argv) != 4:
    print("Usage: python3 spectshow.py <in_wav> <in_txt> <out_pdf>")
    sys.exit(1)

# 接收命令行參數
in_wav = sys.argv[1]  # 輸入的 WAV 文件
in_txt = sys.argv[2]  # 輸入的 ASCII 頻譜數據文件
out_pdf = sys.argv[3]  # 輸出的 PDF 文件

# 讀取波形數據
wav_data = []
with open(in_wav, "rb") as f:
    f.seek(44)  # 跳過 WAV 文件的頭部（44 字節）
    wav_data = np.frombuffer(f.read(), dtype=np.int16)  # 讀取音訊樣本數據，16 位整數格式

# 讀取頻譜數據
spec_data = []
with open(in_txt, "r") as f:
    for line in f:
        spec_data.append([float(val) for val in line.strip().split()])  # 將每行的頻譜數據轉換為浮點數列表

# 將頻譜數據轉換為 NumPy 陣列並進行轉置
spec_data = np.array(spec_data).T

# 繪製波形
plt.figure(figsize=(10, 6))  # 設置圖形大小
plt.subplot(2, 1, 1)  # 在第一個子圖中繪製波形
plt.plot(wav_data, color='blue')  # 使用藍色繪製波形
plt.title("Waveform")  # 設置標題
plt.xlabel("Sample")  # X 軸標籤為樣本數
plt.ylabel("Amplitude")  # Y 軸標籤為振幅

# 繪製頻譜圖
plt.subplot(2, 1, 2)  # 在第二個子圖中繪製頻譜圖
plt.imshow(20 * np.log10(spec_data + 1e-12), aspect='auto', origin='lower', 
           extent=[0, spec_data.shape[1], 0, spec_data.shape[0]])  # 將頻譜數據轉換為 dB 並顯示
plt.colorbar(label="Magnitude (dB)")  # 添加顏色條，用於顯示振幅大小
plt.title("Spectrogram")  # 設置標題
plt.xlabel("Time (frames)")  # X 軸標籤為時間（幀）
plt.ylabel("Frequency Bin")  # Y 軸標籤為頻率桶（離散頻率單位）

# 自動調整子圖之間的佈局，避免重疊
plt.tight_layout()

# 將繪製的圖形保存為 PDF 文件
plt.savefig(out_pdf)

# 關閉圖形窗口，釋放記憶體
plt.close()
