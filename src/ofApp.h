#pragma once
#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void drawVisualizerHUD();
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void audioIn(ofSoundBuffer & input);

	// GUI - Layout Elements
	ofxPanel gui;
	ofxButton btnSelectFolder;
	ofxLabel lblFolderPath;
	ofxButton btnStart;
	ofxLabel lblDeviceHeader;
	ofxLabel lblSpacer;
	ofxFloatSlider sldAudioGain;

	// GUI - Input Selection
	vector<ofxToggle *> deviceToggles;
	bool isUpdatingGui = false; // MUST be here to handle radio button logic

	// Logic State
	string folderPath = "";
	int selectedDeviceID = -1;
	bool isLive = false;

	// Audio & Analysis
	ofSoundDevice::Api currentApi;
	ofSoundStream soundStream;
	ofShader shader;

	// Frequencies (Used by Shader & Draw)
	float subBass; // Deep thumps
	float lowMids; // Kicks and bass guitar
	float mids; // Vocals and snare
	float highMids; // Lead instruments/shimmer
	float treble; // Cymbals/sharp noise
	float hueValue = 0;

	float smoothedLowMids = 0.0f; // used to count headroom

	// Video Handling
	ofVideoPlayer video;
	vector<string> videoFiles;
	void loadRandomVideo();
	float zoomValue = 1.0f;
	float strobeTimer = 0.0f;

	float smoothedHue = 0.0f;

	// UI Event Handlers
	void selectFolderPressed();
	void startPressed();
	void deviceButtonPressed(bool & val);
};
