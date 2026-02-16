# ⚡ Cognitoni Auto VJ - Real-time Audio Visualizer



https://github.com/user-attachments/assets/3f1e9de5-d62e-4fac-959c-5831c84a2d86

🎵 _Cognitoni - Bait_  
🎬 _Bakemonogatari_



An aggressive, shader-based video manipulation engine built with **openFrameworks**. This tool transforms standard video libraries into high-energy visuals by mapping real-time FFT frequency data to complex GLSL mosh patterns and geometric displacements.

---

## 🚀 Getting Started

### 1. Select Video Folder
* Click **"Select Video Folder"** and choose the directory containing your `.mp4` or `.mov` files.

### 2. Audio Input Setup
* The GUI lists all detected hardware inputs.
* Select your primary input (Loopback, External Mic, or System Audio).
* **Mac Fix:** The engine automatically detects your hardware sample rate (44.1k/48k) to ensure the frequency bands stay perfectly calibrated.

### 3. Start VJ
* Once path and audio are set, hit **"START VJ"**.
* Use the **Gain Slider** in the HUD (bottom left) to tune sensitivity to the room volume.

---

## 🔊 The Audio Engine (FFT Bands)

The signal is split into five calibrated bands. Each band is smoothed and normalized to drive specific shader parameters:

| Band | Freq Range | Visual Effect |
| :--- | :--- | :--- |
| **Sub Bass** | 0 – 150Hz | Fluid wave distortion, RGB Shift, & Invert triggers. |
| **Low Mids** | 150 – 350Hz | Screen bounce (Impact Zoom) & Strobe triggers. |
| **Mids** | 350 – 1kHz | Pattern visibility (Shape Mask) & Rotation speed. |
| **High Mids** | 1kHz – 4kHz | High-frequency jitter & HSB brightness pulse. |
| **Treble** | 4kHz+ | Pixelation size & digital noise interference. |

---

## 🎨 Visual Styles (8-Style Mega Evolution)

The app cycles through these modes automatically to keep the visuals evolving:

1.  **Liquid Silk**: Organic, wavy interference lines.
2.  **The Vortex**: Spiraling swirls that pull pixels inward.
3.  **Plasma Blobs**: Fluid, cloud-like organic blobs.
4.  **Kaleidoscope**: 12-point rotational starburst symmetry.
5.  **The Tunnel**: Concentric rings with a pulsing zoom feel.
6.  **Waves of Grain**: Sharp, high-frequency digital interference.
7.  **Rings of Saturn**: Clean, steady concentric circular ripples.
8.  **Geometric Fractal**: Boxy, folded space with sharp angles.

---
## 🛠 Setup & Installation (TODO)

<details>
<summary><b>🍎 MacOS Setup (Verified)</b></summary>

* **IDEs:** Install **Visual Studio Code** & **Xcode**.
* **Framework:** Download **openFrameworks** (macOS release) and unpack.
* **Path Fix:** If the Project Generator fails, move it out of its folder and back in to reset relative paths.
* **Required Addons:** * `ofxGui` (Core - usually auto-added by Project Generator)
    * `ofxFft` (External - requires `fftw` library)
    * `ofxPostProcessing` (External - used for GLSL stack effects)
* **Importing:** Use the **projectGenerator** to "Import" the folder, then open the generated `.xcodeproj` file.
* **Assets:** Ensure `shader.vert` and `shader.frag` are inside the `bin/data/` folder.
* **Permissions:** Grant Xcode (and the final exported App) **Full Disk Access** in *System Settings > Privacy & Security* to allow the app to read video files from external drives or protected folders.
</details>

<details>
<summary><b>🪟 Windows Setup (TODO)</b></summary>

* **IDE:** Install **Visual Studio 2022** (Community Edition).
* **Audio API:** Change the setup logic to use `ofSoundDevice::Api::MS_WASAPI`.
* **Performance:** Use `video.setPixelFormat(OF_PIXELS_NATIVE)` to allow the GPU to handle decoding more efficiently.
* **FFT:** Ensure the `fftw` dll files are present in the `bin` folder for the app to launch.
</details>

<details>
<summary><b>🐧 Linux Setup (TODO)</b></summary>

* **Dependencies:** Run the `scripts/linux/ubuntu/install_dependencies.sh` script provided in the oF root.
* **Audio:** Configure `ofSoundStream` for **ALSA** or **PulseAudio** depending on your distribution.
* **Codecs:** Install `gstreamer-plugins-good`, `base`, and `ugly` to ensure MP4/MOV hardware acceleration.
</details>

---
*Built with openFrameworks and GLSL.*
