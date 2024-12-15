# spectshow.py
import sys
import matplotlib.pyplot as plt
import numpy as np

if len(sys.argv) != 4:
    print("Usage: python3 spectshow.py <in_wav> <in_txt> <out_pdf>")
    sys.exit(1)

in_wav = sys.argv[1]
in_txt = sys.argv[2]
out_pdf = sys.argv[3]

# Read waveform data
wav_data = []
with open(in_wav, "rb") as f:
    f.seek(44)  # Skip header
    wav_data = np.frombuffer(f.read(), dtype=np.int16)

# Read spectrogram data
spec_data = []
with open(in_txt, "r") as f:
    for line in f:
        spec_data.append([float(val) for val in line.strip().split()])

spec_data = np.array(spec_data).T

# Plot waveform
plt.figure(figsize=(10, 6))
plt.subplot(2, 1, 1)
plt.plot(wav_data, color='blue')
plt.title("Waveform")
plt.xlabel("Sample")
plt.ylabel("Amplitude")

# Plot spectrogram
plt.subplot(2, 1, 2)
plt.imshow(20 * np.log10(spec_data + 1e-12), aspect='auto', origin='lower', 
           extent=[0, spec_data.shape[1], 0, spec_data.shape[0]])
plt.colorbar(label="Magnitude (dB)")
plt.title("Spectrogram")
plt.xlabel("Time (frames)")
plt.ylabel("Frequency Bin")

plt.tight_layout()
plt.savefig(out_pdf)
plt.close()


