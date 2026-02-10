# ⚡ Auto VJ - Real-time Audio Visualizer

An aggressive, shader-based video manipulation engine built with **openFrameworks**. This tool transforms standard video libraries into high-energy visuals by mapping real-time FFT frequency data to complex GLSL mosh patterns and geometric displacements.

---

## 🚀 Getting Started

### 1. Select Video Folder
* Click **"Select Video Folder"** and choose the directory containing your `.mp4` or `.mov` files.
* **Performance Tip:** If using an external drive (like Transcend), the app uses `loadAsync` to prevent UI freezing during file swaps.

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

## 🛠 Setup & Installation

### MacOS Setup (Verified)
- [ ] Install **Visual Studio Code** & **Xcode**.
- [ ] Download **openFrameworks** and unpack.
- [ ] Move `ProjectGenerator` out of its folder and back in (to reset relative paths).
- [ ] Install Addons: `ofxFft`, `ofxGui`.
- [ ] Import project via **projectGenerator** and open in IDE.
- [ ] **Crucial:** Copy `shader.vert` and `shader.frag` to `bin/data/`.
- [ ] **Permissions:** Grant Xcode/App "Full Disk Access" in System Settings for external drive read access.

### Windows TODO
- [ ] Install **Visual Studio 2022**.
- [ ] Change audio API to `MS_WASAPI`.
- [ ] Set `video.setPixelFormat(OF_PIXELS_NATIVE)`.

### Linux TODO
- [ ] Run `install_dependencies.sh`.
- [ ] Configure `ofSoundStream` for **ALSA** or **PulseAudio**.

---
*Built with openFrameworks and GLSL.*