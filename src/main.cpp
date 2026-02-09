#include "ofApp.h"
#include "ofMain.h"

int main() {
	// 1. Configure the window settings
	ofGLWindowSettings settings;

	// Set the initial window size
	settings.setSize(1280, 720);

	// OF_WINDOW allows for the adjustable window size you requested
	settings.windowMode = OF_WINDOW;

	// Using OpenGL 3.2+ is critical for M1 Macs and modern Linux drivers
	settings.setGLVersion(3, 2);

	// 2. Create the window
	auto window = ofCreateWindow(settings);

	// 3. Start the app
	ofRunApp(window, make_shared<ofApp>());
	ofRunMainLoop();
}
