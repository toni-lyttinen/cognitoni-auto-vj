#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

	ofGLWindowSettings settings;
	settings.setGLVersion(3, 2);
	settings.setSize(1280, 720);
	settings.windowMode = OF_WINDOW;

	auto window = ofCreateWindow(settings);

	ofRunApp(window, std::make_shared<ofApp>());
	ofRunMainLoop();

}
