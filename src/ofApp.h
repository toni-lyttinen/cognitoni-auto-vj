#pragma once
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxFft.h"
#include <atomic>

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	~ofApp(); // Destructor for proper cleanup
	void drawVisualizerHUD();
	void drawEventCredits();
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void audioIn(ofSoundBuffer & input);

	// GUI - Layout Elements
	ofxPanel gui;
	ofxPanel guiLive;
	ofxButton btnSelectFolder;
	ofxLabel lblFolderPath;
	ofxButton btnStart;
	ofxButton btnStop;
	ofxLabel lblDeviceHeader;
	ofxLabel lblSpacer;
	ofxFloatSlider sldAudioGain;

	// GUI - Input Selection
	vector<ofxToggle *> deviceToggles;
	vector<bool> deviceToggleStates;
	bool isUpdatingGui = false; // handle radio button logic
	bool bIsTransitioning = false; // Block input for buttons as of buttons are clickable even when not rendered

	// Logic State
	string folderPath = "";
	int selectedDeviceIndex = -1;
	bool isLive = false;

	// Audio & Analysis
	ofSoundDevice::Api currentApi;
	ofSoundStream soundStream;
	ofShader shader;
	ofxFft* fft; // You may need the ofxFft addon, or use ofSoundStream's internal if available
	vector<float> fftBins;

	// Frequencies (Used by Shader & Draw)
	std::atomic<float> subBass{0.0f}; // Deep thumps
	std::atomic<float> lowMids{0.0f}; // Kicks and bass guitar
	std::atomic<float> mids{0.0f}; // Vocals and snare
	std::atomic<float> highMids{0.0f}; // Lead instruments/shimmer
	std::atomic<float> treble{0.0f}; // Cymbals/sharp noise
	float hueValue = 0;

	float smoothedLowMids = 0.0f; // used to count headroom
	float smoothedRGBShift = 0.0f; // Stores the decaying RGB bloom value
    bool invertActive = false;      // Tracks if the sub-bass inversion is triggered

	// Video Handling
	ofVideoPlayer video;
	vector<string> videoFiles;
	bool bNeedsNewVideo = false;
	bool bIsLoading = false;
	bool bPendingLoad = false;
	void loadRandomVideo();
	float zoomValue = 1.0f;
	float strobeTimer = 0.0f;

	float smoothedHue = 0.0f;

	// UI Event Handlers
	void selectFolderPressed();
	void startPressed();
	void stopPressed();
	void deviceButtonPressed(bool & val);
	void updateStartButtonLabel();
	void buildSettingsGui();
	bool startLiveSession(bool allowVideoLoad = true);
	void stopLiveSession();

	// Memory Management
	void cleanupDeviceToggles();
};
