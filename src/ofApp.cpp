#include "ofApp.h"

void ofApp::setup() {
	ofSetBackgroundAuto(false); // Tell oF not to clear the screen automatically

	shader.load("shader.vert", "shader.frag");

	gui.setup("Auto VJ Setup");

	// Select Folder (Button)
	gui.add(btnSelectFolder.setup("1. Select Video Folder"));

	// Selected Path (Label)
	gui.add(lblFolderPath.setup("Selected path:", "None Selected"));

	//  *spacer*
	gui.add(lblSpacer.setup("", ""));

	// Input Device Header
	gui.add(lblDeviceHeader.setup("2. Select Input Device", ""));

	// **list devices here as radio buttons**
	#ifdef TARGET_OSX
		currentApi = ofSoundDevice::Api::APPLE_CORE_AUDIO;
	#else
		currentApi = ofSoundDevice::Api::MS_WASAPI;
	#endif

	sldAudioGain = 1.0f; // default value 1.0

	auto devices = soundStream.getDeviceList(currentApi);
	for (int i = 0; i < devices.size(); i++) {
		if (devices[i].inputChannels > 0) {
			ofxToggle * tgl = new ofxToggle();
			gui.add(tgl->setup(devices[i].name, false));
			tgl->addListener(this, &ofApp::deviceButtonPressed);
			deviceToggles.push_back(tgl);
		}
	}

	// *spacer*
	gui.add(lblSpacer.setup("", ""));

	// Start (Button) - NOW AT THE BOTTOM
	gui.add(btnStart.setup("START VJ"));

	// Handlers
	btnSelectFolder.addListener(this, &ofApp::selectFolderPressed);
	btnStart.addListener(this, &ofApp::startPressed);
}

void ofApp::selectFolderPressed() {
	ofFileDialogResult res = ofSystemLoadDialog("Select Video Folder", true);

	if (res.bSuccess) {
		// 1. STOP everything immediately to prevent a crash while clearing data
		isLive = false;
		video.stop();
		video.close();

		folderPath = res.getPath();
		lblFolderPath = folderPath;

		ofDirectory dir(folderPath);
		dir.allowExt("mp4");
		dir.allowExt("mov");
		dir.listDir();

		// 2. Clear and fill the vector safely
		videoFiles.clear();
		for (int i = 0; i < dir.size(); i++) {
			// getPath(i) is more stable than getFile().getAbsolutePath()
			videoFiles.push_back(dir.getPath(i));
		}

		ofLogNotice() << "Videos found: " << videoFiles.size();

		// 3. Only load if the folder isn't empty
		if (videoFiles.size() > 0) {
			loadRandomVideo();
		} else {
			ofSystemAlertDialog("Error: No videos found in that folder.");
		}
	}
}

void ofApp::deviceButtonPressed(bool & val) {
	if (isUpdatingGui) return; // Stop the loop!

	for (int i = 0; i < deviceToggles.size(); i++) {
		// If we found the toggle that is ON
		if (deviceToggles[i]->getParameter().cast<bool>().get()) {

			isUpdatingGui = true; // Start Silencing
			selectedDeviceID = i;

			string selectedName = deviceToggles[i]->getName();
			ofLogNotice() << ">>> NEW SELECTION: " << selectedName;

			// Turn off EVERY other toggle
			for (int j = 0; j < deviceToggles.size(); j++) {
				if (i != j) {
					deviceToggles[j]->getParameter().cast<bool>().set(false);
				}
			}
			isUpdatingGui = false; // Stop Silencing

			return;
		}
	}
}

void ofApp::startPressed() {
	if (videoFiles.empty()) {
		ofSystemAlertDialog("Select video folder first!");
		return;
	}
	if (selectedDeviceID == -1) {
		ofSystemAlertDialog("Select an audio device first!");
		return;
	}

	soundStream.stop();
	soundStream.close();

	auto devices = soundStream.getDeviceList(currentApi);
	vector<ofSoundDevice> inputs;
	for (auto & d : devices)
		if (d.inputChannels > 0) inputs.push_back(d);

	ofSoundStreamSettings settings;
	settings.setApi(currentApi);
	settings.setInDevice(inputs[selectedDeviceID]);
	settings.setInListener(this);
	settings.sampleRate = 44100;
	settings.numInputChannels = 1;
	settings.bufferSize = 1024;

	if (soundStream.setup(settings)) {
		loadRandomVideo();
		isLive = true;
	}
}

void ofApp::audioIn(ofSoundBuffer & input) {
	float s = 0, lm = 0, m = 0, hm = 0, t = 0;
	int numFrames = input.getNumFrames();
	int sampleRate = input.getSampleRate();
	float binSize = (float)sampleRate / (float)numFrames;

	int subCutoff = 150 / binSize;
	int lowMidCut = 350 / binSize;
	int midCut = 1000 / binSize;
	int highMidCut = 4000 / binSize;

	int counts[5] = { 0, 0, 0, 0, 0 };

	for (size_t i = 0; i < numFrames; i++) {
		float sample = abs(input[i] * (float)sldAudioGain);

		if (i <= subCutoff) {
			s += sample;
			counts[0]++;
		} else if (i <= lowMidCut) {
			lm += sample;
			counts[1]++;
		} else if (i <= midCut) {
			m += sample;
			counts[2]++;
		} else if (i <= highMidCut) {
			hm += sample;
			counts[3]++;
		} else {
			t += sample;
			counts[4]++;
		}
	}

	// --- Softened Band Calculations ---
	subBass = ofLerp(subBass, (s / max(1, counts[0])) * 1.0f, 0.1f);
	lowMids = ofLerp(lowMids, (lm / max(1, counts[1])) * 1.0f, 0.1f);
	mids = ofLerp(mids, (m / max(1, counts[2])) * 1.0f, 0.1f);
	highMids = ofLerp(highMids, (hm / max(1, counts[3])) * 1.0f, 0.1f);
	treble = ofLerp(treble, (t / max(1, counts[4])) * 1.0f, 0.1f);
}

void ofApp::update() {
	if (isLive && video.isLoaded()) {

		// Countdown the strobe timer (decreases every frame)
		if (strobeTimer > 0) strobeTimer -= 0.1f;

		// Trigger/Reset the timer on a strong kick
		float impactDelta = std::max(0.0f, lowMids - (smoothedLowMids + 0.1f));
		if (impactDelta > 0.45f) {
			strobeTimer = 1.0f; // This will keep the strobe alive for about 10 frames
		}
		video.update();

		// Zoom Logic
		smoothedLowMids = ofLerp(smoothedLowMids, lowMids, 0.05f);
		float delta = std::max(0.0f, lowMids - (smoothedLowMids + 0.1f));
		float targetZoom = 1.0f + (delta * 15.0f);
		zoomValue = ofLerp(zoomValue, targetZoom, 0.3f);

		// Low Pass Hue (Color Smoothing)
		// Instead of jumping, smoothedHue "chases" the target hueValue
		smoothedHue = ofLerp(smoothedHue, hueValue, 0.05f);

		if (video.getIsMovieDone()) {
			loadRandomVideo();
		}
	}
}

void ofApp::draw() {
	if (!isLive) {
		ofSetBackgroundAuto(true);
		ofBackground(40);
		gui.draw();
		return;
	}

	// 1. DYNAMIC MOTION BLUR (Reactive Alpha)
	float blurAmount = ofMap(mids, 0.2, 0.8, 80, 15, true);
	ofSetBackgroundAuto(false);
	ofSetColor(0, 0, 0, blurAmount);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

	if (!video.isLoaded() || !video.getTexture().isAllocated()) return;

	// 2. IMPACT LOGIC
	float impactDelta = std::max(0.0f, lowMids - (smoothedLowMids + 0.1f));

	ofPushMatrix();
	ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);

	// 3. JITTER
	float jitter = ofMap(highMids, 0.3, 1.0, 0.0, 0.3, true);
	if (jitter > 0.01) ofTranslate(ofRandom(-jitter, jitter), ofRandom(-jitter, jitter));

	// 4. BOUNCE (Unified Scale Fixes Y-Bounce)
	float bounceScale = 1.0 + (impactDelta * 18.0);
	ofScale(zoomValue * bounceScale, zoomValue * bounceScale);

	shader.begin();
	shader.setUniform1f("time", ofGetElapsedTimef());

	// AGGRESSIVE UNIFORM MAPPING
	shader.setUniform1f("pixelSize", ofMap(treble, 0.4, 1.0, 1.0, 8.0, true));
	shader.setUniform1f("rgbShift", subBass * 8.0 + (impactDelta * 120.0));
	shader.setUniform1f("impactDelta", impactDelta);
	shader.setUniform1f("subBass", subBass);
	shader.setUniform1f("lowMids", lowMids);
	shader.setUniform1f("mids", mids);
	shader.setUniform1f("highMids", highMids);
	shader.setUniform1f("treble", treble);
	shader.setUniform1f("lowThresh", 0.10f); // Patterns trigger earlier
	shader.setUniform1f("highThresh", 0.80f); // Blocks trigger earlier
	shader.setUniform2f("res", (float)ofGetWidth(), (float)ofGetHeight());

	// 5. INVERT (High-bass trigger)
	bool subInvert = (subBass > 0.8f);
	shader.setUniform1i("invertToggle", subInvert ? 1 : 0);

	// 6. HSB COLOR PULSE
	float br = ofMap(highMids, 0.2, 0.8, 150, 190, true);
	ofSetColor(ofColor::fromHsb(fmod(smoothedHue, 255.0), 160, br));

	// 7. SLICING
	if (mids > 0.25) {
		int numSlices = (int)ofMap(mids, 0.25, 1.0, 16, 64, true);
		float maxShift = ofMap(mids, 0.25, 1.0, 0.5, 4.0, true);
		for (int i = 0; i < numSlices; i++) {
			float xOffset = ofRandom(-maxShift, maxShift);
			video.getTexture().drawSubsection(
				-ofGetWidth() / 2 + xOffset, -ofGetHeight() / 2 + (i * (ofGetHeight() / numSlices)),
				ofGetWidth(), ofGetHeight() / numSlices,
				0, i * (video.getHeight() / numSlices));
		}
	} else {
		video.draw(-ofGetWidth() / 2, -ofGetHeight() / 2, ofGetWidth(), ofGetHeight());
	}
	shader.end();

	// 8. IMPACT OVERLAY
	if (strobeTimer > 0.0f) {
		ofSetColor(255, 255, 255, strobeTimer * 40.0f);
		ofDrawRectangle(-ofGetWidth() / 2, -ofGetHeight() / 2, ofGetWidth(), ofGetHeight());
	}

	ofPopMatrix();

	// 9. CRT SCANLINES
	ofSetColor(0, 0, 0, 25);
	for (int i = 0; i < ofGetHeight(); i += 4)
		ofDrawLine(0, i, ofGetWidth(), i);

	// 10. HUD & GUI (Menu Logic)
	drawVisualizerHUD();
	gui.draw();
}

void ofApp::drawVisualizerHUD() {
	ofPushStyle();

	// --- 1. HUD Dimensions (Twice as high, shifted to Left) ---
	float panelW = 220;
	float panelH = 180; // Height doubled for better spacing
	float xBase = 15; // Anchored left to align with GUI
	float yBase = ofGetHeight() - panelH - 20;
	float maxBarH = 100.0; // Fixed bar height

	// --- 2. HUD Background (Opaque enough to see over chaos) ---
	ofSetColor(0, 0, 0, 180);
	ofDrawRectRounded(xBase - 10, yBase - 10, panelW - 20, panelH, 8);

	// --- 3. The Frequency Bars (Left Aligned & Clamped) ---
	float barW = 28;
	float barGap = 6;
	float barYAnchor = yBase + maxBarH + 10; // Bottom of the bars 

	float vals[] = { subBass, lowMids, mids, highMids, treble };
	ofColor colors[] = {
		ofColor(255, 80, 80), ofColor(255, 160, 50), ofColor(80, 255, 80),
		ofColor(80, 180, 255), ofColor(180, 80, 255)
	};

	for (int i = 0; i < 5; i++) {
		// h is mapped to the maxBarH and 5.0 is gain max
		float h = ofMap(vals[i], 0.0, 5.0, 0, maxBarH, true);
		ofSetColor(colors[i]);
		ofDrawRectangle(xBase + (i * (barW + barGap)), barYAnchor, barW, -h);
	}

	// --- 4. The Integrated Slider (Bottom of HUD) ---
	float sliderY = barYAnchor + 25;
	float sliderWidth = (barW + barGap) * 4 + barW;
	float sliderHeight = 12;

	// Track
	ofSetColor(60);
	ofDrawRectangle(xBase, sliderY, sliderWidth, sliderHeight);

	// Fill
	float fillWidth = ofMap(sldAudioGain, 0.0, 5.0, 0, sliderWidth, true);
	ofSetColor(200, 200, 255);
	ofDrawRectangle(xBase, sliderY, fillWidth, sliderHeight);

	// Label
	ofSetColor(255);
	ofDrawBitmapString("GAIN: " + ofToString((float)sldAudioGain, 1), xBase, sliderY + 25);

	ofPopStyle();
}

void ofApp::mousePressed(int x, int y, int button) {
	if (isLive) {
		float xStart = 15;
		float yBase = ofGetHeight() - 220 - 20;
		float sliderY = yBase + 150 + 30;
		float sliderWidth = 164;

		if (x >= xStart && x <= xStart + sliderWidth && y >= sliderY && y <= sliderY + 15) {
			sldAudioGain = ofMap(x, xStart, xStart + sliderWidth, 0.0, 5.0, true);
		}
	}
}

void ofApp::mouseDragged(int x, int y, int button) {
	if (isLive) {
		float xStart = 15;
		float yBase = ofGetHeight() - 220 - 20;
		float sliderY = yBase + 150 + 30;
		float sliderWidth = 164;

		if (x >= xStart && x <= xStart + sliderWidth && y >= sliderY && y <= sliderY + 15) {
			sldAudioGain = ofMap(x, xStart, xStart + sliderWidth, 0.0, 5.0, true);
		}
	}
}

void ofApp::loadRandomVideo() {
	if (!videoFiles.empty()) {
		int idx = floor(ofRandom(videoFiles.size()));

		// 1. Force a clean slate
		video.stop();
		video.close();

		// 2. Load with optimized settings
		video.setPixelFormat(OF_PIXELS_NATIVE);

		if (video.load(videoFiles[idx])) {
			video.setLoopState(OF_LOOP_NORMAL);
			video.play();
			video.setVolume(0.0f); // Mute video audio

			// 3. Force one update immediately to "warm up" the texture
			// This prevents the shader from hitting a NULL texture on the first frame
			video.update();

			ofLogNotice() << "Loaded " << videoFiles[idx] << " @ 360p";
		}
	}
}
