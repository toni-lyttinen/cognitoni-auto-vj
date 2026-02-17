#include "ofApp.h"

ofApp::~ofApp() {
    // Force a full hardware release on exit
    if (soundStream.getSoundStream()) {
        soundStream.stop();
        soundStream.close();
    }
    
    cleanupDeviceToggles();
    if (fft != nullptr) {
        delete fft;
        fft = nullptr;
    }
}

void ofApp::cleanupDeviceToggles() {
    for (auto toggle : deviceToggles) {
        if (toggle) {
            delete toggle;
        }
    }
    deviceToggles.clear();
    deviceToggleStates.clear();
}

void ofApp::updateStartButtonLabel() {
    btnStart.setName("START VJ");
    btnStart.getParameter().setName("START VJ");
    btnStop.setName("STOP VJ");
    btnStop.getParameter().setName("STOP VJ");
}

void ofApp::buildSettingsGui() {
    isUpdatingGui = true; 
    bIsTransitioning = true; // Block all inputs

    cleanupDeviceToggles(); 
    gui.clear();
    gui.setup("Cognitoni Auto VJ");

    gui.add(btnSelectFolder.setup("1. Select Video Folder"));
    gui.add(lblFolderPath.setup("Selected path:", folderPath));
    gui.add(lblSpacer.setup("", ""));
    gui.add(lblDeviceHeader.setup("2. Select Input Device", ""));

    vector<ofSoundDevice> devices;
    {
        ofSoundStream deviceFetcher;
        devices = deviceFetcher.getDeviceList(currentApi);
    } 

    for (int i = 0; i < (int)devices.size(); i++) {
        if (devices[i].inputChannels > 0) {
            ofxToggle * tgl = new ofxToggle();
            gui.add(tgl->setup(devices[i].name, false));
            tgl->addListener(this, &ofApp::deviceButtonPressed);
            deviceToggles.push_back(tgl);
            deviceToggleStates.push_back(false);
        }
    }

    gui.add(lblSpacer.setup("", ""));
    gui.add(btnStart.setup("START VJ"));

    btnSelectFolder.addListener(this, &ofApp::selectFolderPressed);
    btnStart.addListener(this, &ofApp::startPressed);

    selectedDeviceIndex = -1; 
    isUpdatingGui = false;    
}

bool ofApp::startLiveSession(bool allowVideoLoad) {
    if (videoFiles.empty() || selectedDeviceIndex < 0) return false;

    string targetName = deviceToggles[selectedDeviceIndex]->getName();
    auto devices = soundStream.getDeviceList(currentApi);
    ofSoundDevice selectedDevice;
    bool found = false;
    for (auto& d : devices) {
        if (d.inputChannels > 0 && d.name == targetName) {
            selectedDevice = d;
            found = true;
            break;
        }
    }

    if (!found) return false;

    ofSoundStreamSettings settings;
    settings.setApi(currentApi);
    settings.setInDevice(selectedDevice);
    settings.setInListener(this);
    settings.numInputChannels = (selectedDevice.inputChannels > 2) ? 2 : selectedDevice.inputChannels;
    settings.numOutputChannels = 0;
	settings.sampleRate = selectedDevice.sampleRates.empty() ? 44100 : selectedDevice.sampleRates[0];
	settings.bufferSize = 1024;

    if (soundStream.setup(settings)) {
        isLive = true;
        if (video.isLoaded()) video.play();
        else if (allowVideoLoad) loadRandomVideo();
        return true;
    }
    return false;
}

void ofApp::setup() {
    ofSetBackgroundAuto(false);

	// Set path for OSX release version
	#ifdef TARGET_OSX
		#ifndef DEBUG
			ofSetDataPathRoot("../Resources/data/");
		#endif
	#endif

    shader.load("shader.vert", "shader.frag");
    
    // Setup the "Live" GUI (the one seen while VJing)
    guiLive.setup("Cognitoni Auto VJ");
    guiLive.add(btnStop.setup("STOP VJ"));
    btnStop.addListener(this, &ofApp::stopPressed);

    #ifdef TARGET_OSX
        currentApi = ofSoundDevice::Api::OSX_CORE;
    #else
        currentApi = ofSoundDevice::Api::MS_WASAPI;
    #endif

    fft = ofxFft::create(1024, OF_FFT_WINDOW_HAMMING);
    if (fft != nullptr) fftBins.resize(fft->getBinSize());
    
    sldAudioGain = 1.0f;

    // Call the builder to create the initial Menu
    buildSettingsGui(); 
}

void ofApp::selectFolderPressed() {
    if (bIsTransitioning || isLive) return; // Hard block during transition or live session

    ofFileDialogResult res = ofSystemLoadDialog("Select Video Folder", true);

    if (res.bSuccess) {
        string newFolderPath = res.getPath();
        ofDirectory dir(newFolderPath);
        dir.allowExt("mp4");
        dir.allowExt("mov");
        dir.listDir();

        vector<string> newVideoFiles;
        for (int i = 0; i < dir.size(); i++) {
            newVideoFiles.push_back(dir.getPath(i));
        }

        ofLogNotice() << "Videos found: " << newVideoFiles.size();

        if (newVideoFiles.empty()) {
            ofSystemAlertDialog("Error: No videos found in that folder.");
            return;
        }

        folderPath = newFolderPath;
        lblFolderPath = folderPath;
        videoFiles = newVideoFiles;

        bPendingLoad = false;
        bIsLoading = false;
        
        // Use the reinforced texture cleanup
        video.stop();
        video.setUseTexture(false); 
        if (video.isLoaded()) {
            video.closeMovie();
        }

        loadRandomVideo();
    }
}

void ofApp::deviceButtonPressed(bool & val) {
    if (isUpdatingGui) return; // Stop the loop!
    if (isLive) return; // Keep selection immutable while running for cross-platform stability

    if (deviceToggleStates.size() != deviceToggles.size()) {
        deviceToggleStates.assign(deviceToggles.size(), false);
    }

    int changedIndex = -1;
    for (int i = 0; i < static_cast<int>(deviceToggles.size()); i++) {
        bool currentValue = deviceToggles[i]->getParameter().cast<bool>().get();
        if (currentValue != deviceToggleStates[i]) {
            changedIndex = i;
            deviceToggleStates[i] = currentValue;
        }
    }

    if (changedIndex == -1) return;

    bool changedValue = deviceToggles[changedIndex]->getParameter().cast<bool>().get();

    if (!changedValue) {
        if (changedIndex == selectedDeviceIndex) {
            isUpdatingGui = true;
            deviceToggles[changedIndex]->getParameter().cast<bool>().set(true);
            isUpdatingGui = false;
            deviceToggleStates[changedIndex] = true;
        }
        return;
    }

    selectedDeviceIndex = changedIndex;

    isUpdatingGui = true;
    for (int i = 0; i < static_cast<int>(deviceToggles.size()); i++) {
        bool shouldBeOn = (i == changedIndex);
        deviceToggles[i]->getParameter().cast<bool>().set(shouldBeOn);
        deviceToggleStates[i] = shouldBeOn;
    }
    isUpdatingGui = false;

    ofLogNotice() << ">>> NEW SELECTION: " << deviceToggles[changedIndex]->getName();
}

void ofApp::startPressed() {
    if (bIsTransitioning || isLive) return;

    // Load video first to manage thread priorities
    if (!video.isLoaded()) {
        loadRandomVideo();
    }

    // Start audio after video initialization
    startLiveSession(false); 
}

void ofApp::stopPressed() {
    // Exit application to ensure full hardware reset
    ofExit();
}

void ofApp::audioIn(ofSoundBuffer & input) {
    if (!isLive || fft == nullptr) return;
    
    fft->setSignal(input.getBuffer());
    float* analyzerBuffer = fft->getAmplitude();
    int numBins = fft->getBinSize();
    
    float s = 0, lm = 0, m = 0, hm = 0, t = 0;

    // Use the actual buffer rate to keep math accurate
    int sampleRate = input.getSampleRate();
    
    // Frequency per bin = SampleRate / (N_FFT)
    float binSize = (float)sampleRate / (float)(numBins * 2);

    // Calculate indices dynamically to match frequency targets
    int subCutoff = 150 / binSize;
    int lowMidCut = 350 / binSize;
    int midCut = 1000 / binSize;
    int highMidCut = 4000 / binSize;

    int counts[5] = { 0, 0, 0, 0, 0 };

    for (int i = 0; i < numBins; i++) {
        // Apply frequency tilt to counter natural energy drop-off in higher frequencies
        float tilt = 1.0f + ((float)i / (float)numBins) * 10.0f;
        float sample = analyzerBuffer[i] * (float)sldAudioGain * 25.0f * tilt;

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
    // Note: Atomicity is handled automatically during assignment
    subBass = ofLerp(subBass, (s / max(1, counts[0])) * 1.0f, 0.1f);
    lowMids = ofLerp(lowMids, (lm / max(1, counts[1])) * 1.8f, 0.1f);
    mids = ofLerp(mids, (m / max(1, counts[2])) * 2.5f, 0.1f);
    highMids = ofLerp(highMids, (hm / max(1, counts[3])) * 4.0f, 0.1f);
    treble = ofLerp(treble, (t / max(1, counts[4])) * 6.0f, 0.1f);
}

void ofApp::update() {
    // If we just rebuilt the GUI, wait one frame, then enable buttons
    if (bIsTransitioning) {
        bIsTransitioning = false; 
        return; 
    }

    if (!isLive) return;

    video.update();

    // --- IMPACT & STROBE LOGIC ---
    if (strobeTimer > 0) strobeTimer -= 0.1f;
    
    // Calculate how much the current bass hit exceeds the average
    float impactDelta = std::max(0.0f, (float)lowMids - (smoothedLowMids + 0.12f));
    
    // Trigger flash on significant peaks
    if (impactDelta > 0.45f) strobeTimer = 1.0f;

    // --- ENVELOPE ZOOM & RGB LOGIC ---
    // Ignore tiny volume fluctuations to prevent constant shivering
    float cleanImpact = 0.0f;
    if (impactDelta > 0.05f) {
        // Map the impact to a usable range (0.0 to 1.0)
        cleanImpact = ofMap(impactDelta, 0.05, 0.5, 0.0, 1.0, true);
    }

    // Define target values for Zoom and RGB Shift
    float targetZoom = 1.0f + (cleanImpact * 0.35f);
    float targetRGB = cleanImpact * 120.0f; // Maximum bloom for RGB shift

    // Asymmetric Smoothing for Zoom
    if (targetZoom > zoomValue) {
        zoomValue = ofLerp(zoomValue, targetZoom, 0.4f); // Fast Attack
    } else {
        zoomValue = ofLerp(zoomValue, 1.0f, 0.08f);    // Slow Release
    }

	// Create a trigger for the invert that decays over time
	//if (subBass > smoothedLowMids + 0.4f && strobeTimer <= 0.0f) {
	//	invertActive = true; 
	//} else {
	//	invertActive = false;
	//}

    // Asymmetric Smoothing for RGB Shift (The RGB Fix)
    // This prevents the color channels from flickering too aggressively
    if (targetRGB > smoothedRGBShift) {
        smoothedRGBShift = ofLerp(smoothedRGBShift, targetRGB, 0.5f); // Instant snap
    } else {
        smoothedRGBShift = ofLerp(smoothedRGBShift, 0.0f, 0.1f);      // Smooth fade
    }

    // --- GENERAL SMOOTHING ---
    // Gradually update the baseline to follow long-term volume changes
    smoothedLowMids = ofLerp(smoothedLowMids, (float)lowMids, 0.05f);
    smoothedHue = ofLerp(smoothedHue, hueValue, 0.05f);

    // End-of-video check and random reload
    if (video.isLoaded() && video.getIsMovieDone()) {
        ofLogNotice() << "MOVIE DONE TRIGGER";
        loadRandomVideo();
    }
}

void ofApp::draw() {
    updateStartButtonLabel();

    // If not live, clear the screen and show settings
    if (!isLive) {
        ofSetBackgroundAuto(true);
        ofBackground(40); // Dark grey settings background
        gui.draw();
        return;
    }
    
    // Check if we are ready. If not, draw black and stop.
    if (!video.isLoaded() || !video.getTexture().isAllocated()) {
        ofSetBackgroundAuto(true);
        ofBackground(0);
        guiLive.draw();
        return;
    }

    // DYNAMIC MOTION BLUR (Reactive Alpha)
    float blurAmount = ofMap(mids, 0.2, 0.8, 80, 15, true);
    ofSetBackgroundAuto(false);
    ofSetColor(0, 0, 0, blurAmount);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

    if (!video.isLoaded() || !video.getTexture().isAllocated()) return;

    // IMPACT LOGIC
    float impactDelta = std::max(0.0f, (float)lowMids - (smoothedLowMids + 0.1f));

    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);

    // JITTER
    float jitter = ofMap(highMids, 0.3, 1.0, 0.0, 0.3, true);
    if (jitter > 0.01) ofTranslate(ofRandom(-jitter, jitter), ofRandom(-jitter, jitter));

    // BOUNCE (Unified Scale Fixes Y-Bounce)
    float bounceScale = 1.0 + (impactDelta * 18.0);
    ofScale(zoomValue * bounceScale, zoomValue * bounceScale);

    shader.begin();
    // Bind video texture and elapsed time to shader
    shader.setUniformTexture("tex0", video.getTexture(), 0);
    shader.setUniform1f("time", ofGetElapsedTimef());

    // Map audio frequencies to shader parameters
    shader.setUniform1f("pixelSize", ofMap(treble, 0.1, 1.2, 1.0, 14.0, true));
    shader.setUniform1f("rgbShift", subBass * 8.0 + (impactDelta * 120.0));
    shader.setUniform1f("impactDelta", impactDelta);
    shader.setUniform1f("subBass", subBass);
    shader.setUniform1f("lowMids", lowMids);
    shader.setUniform1f("mids", mids);
    shader.setUniform1f("highMids", highMids);
    shader.setUniform1f("treble", treble);
    shader.setUniform1f("lowThresh", 0.10f); // Patterns trigger earlier
    shader.setUniform1f("highThresh", 0.80f); // Blocks trigger earlier
    shader.setUniform2f("res", (float)video.getWidth(), (float)video.getHeight());

    // INVERT (High-bass trigger)
    bool subInvert = (subBass > 0.8f);
    shader.setUniform1i("invertToggle", subInvert ? 1 : 0);

    // HSB COLOR PULSE
    float br = ofMap(highMids, 0.2, 0.8, 150, 190, true);
    ofSetColor(ofColor::fromHsb(fmod(smoothedHue, 255.0), 160, br));

    // SLICING
    if (mids > 0.25) {
        int numSlices = (int)ofMap(mids, 0.25, 1.0, 16, 64, true);
        float maxShift = ofMap(mids, 0.25, 1.0, 0.5, 4.0, true);
        
        // Use video dimensions for math to prevent scaling drift
        float sliceHeightDest = (float)ofGetHeight() / numSlices;
        float sliceHeightSrc = (float)video.getHeight() / numSlices;
        float halfW = (float)ofGetWidth() / 2.0;
        float halfH = (float)ofGetHeight() / 2.0;

        for (int i = 0; i < numSlices; i++) {
            float xOffset = ofRandom(-maxShift, maxShift);
            
            video.getTexture().drawSubsection(
                -halfW + xOffset,          // Destination X (Centered)
                -halfH + (i * sliceHeightDest), // Destination Y (Centered)
                ofGetWidth(),              // Destination Width
                sliceHeightDest,           // Destination Height
                0,                         // Source X
                i * sliceHeightSrc,        // Source Y
                video.getWidth(),          // Source Width
                sliceHeightSrc             // Source Height
            );
        }
    } else {
        ofSetColor(255);
        video.getTexture().draw(-ofGetWidth()/2, -ofGetHeight()/2, ofGetWidth(), ofGetHeight());
    }
    shader.end();

    // IMPACT OVERLAY
    if (strobeTimer > 0.0f) {
        ofSetColor(255, 255, 255, strobeTimer * 40.0f);
        ofDrawRectangle(-ofGetWidth() / 2, -ofGetHeight() / 2, ofGetWidth(), ofGetHeight());
    }

    ofPopMatrix();

    // CRT SCANLINES
    ofSetColor(0, 0, 0, 25);
    for (int i = 0; i < ofGetHeight(); i += 4)
        ofDrawLine(0, i, ofGetWidth(), i);

    // HUD & GUI (Menu Logic)
    drawVisualizerHUD();
    guiLive.draw();
    drawEventCredits(); // credits
}

void ofApp::drawEventCredits() {
    ofPushStyle();
    ofPushMatrix();
    
    ofTranslate(ofGetWidth() - 270, 30);

    ofScale(1.5, 1.5);
    
    ofDrawBitmapStringHighlight("Visual tool by @cognitoni", 0, 0, ofColor(0, 200), ofColor(255));
    
    ofPopMatrix();
    ofPopStyle();
}

void ofApp::drawVisualizerHUD() {
    ofPushStyle();

    // HUD Dimensions (Twice as high, shifted to Left)
    float panelW = 220;
    float panelH = 180; 
    float xBase = 15; // Anchored left to align with GUI
    float yBase = ofGetHeight() - panelH - 20;
    float maxBarH = 100.0; // Fixed bar height

    // HUD Background 
    ofSetColor(0, 0, 0, 180);
    ofDrawRectRounded(xBase - 10, yBase - 10, panelW - 20, panelH, 8);

    // The Frequency Bars (Left Aligned & Clamped)
    float barW = 28;
    float barGap = 6;
    float barYAnchor = yBase + maxBarH + 10; // Bottom of the bars 

    float vals[] = { (float)subBass, (float)lowMids, (float)mids, (float)highMids, (float)treble };
    ofColor colors[] = {
        ofColor(255, 80, 80), ofColor(255, 160, 50), ofColor(80, 255, 80),
        ofColor(80, 180, 255), ofColor(180, 80, 255)
    };

    for (int i = 0; i < 5; i++) {
        float h = ofMap(vals[i], 0.0, 5.0, 0, maxBarH, true);
        ofSetColor(colors[i]);
        ofDrawRectangle(xBase + (i * (barW + barGap)), barYAnchor, barW, -h);
    }

    // The Integrated Slider (Bottom of HUD)
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
    if (videoFiles.empty()) return;
    
    // Release hardware decoder and GPU texture resources
    video.stop();
    video.setUseTexture(false); // Force dump of the GPU texture handle
    if (video.isLoaded()) {
        video.closeMovie();
    }

    bIsLoading = false;
    bPendingLoad = false;
    int idx = floor(ofRandom(videoFiles.size()));

    // Re-enable textures before loading new file
    video.setUseTexture(true); 
    bool loaded = video.load(videoFiles[idx]);
    if (!loaded) {
        ofLogError() << "FAILED TO LOAD VIDEO: " << videoFiles[idx];
        return;
    }

    video.setLoopState(OF_LOOP_NONE);
    video.play();
    ofLogNotice() << "STARTING VIDEO: " << videoFiles[idx];
}