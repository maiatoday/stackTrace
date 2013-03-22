#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "StreamMote.h"
#include "ofxOpenCv.h"

#include "ColorSampler.h"
#include "TextSampler.h"
#include "GlyphSampler.h"

#include "ofxMSAPhysics.h"

#include "ofxThreadedImageSaver.h"
#include "ofxXmlSettings.h"
#include "ofxOpenNI.h"

#define MAX_PLAYERS 5
#define MAX_FLIPCOUNT 10
class testApp: public ofBaseApp {

public:
	testApp();
	~testApp();

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void resized(int w, int h);

	void initScene();
	void addRandomParticle();
	void addRandomSpring();
	void killRandomParticle();
	void killRandomSpring();
	void killRandomConstraint();
	void addRandomForce(float f);
	void lockRandomParticles();
	void unlockRandomParticles();

	StreamMote* makeStreamMote(ofPoint pos, float m, float d);
	void updateParticles();
	XnUInt16 numberUsers;
	int flipCount;
	void setUserAttract(bool _attractOn);
	void setScreenRatios(void);

	ofTrueTypeFont myFont;

	bool doVideoWrite;
#ifdef DO_VIDEO
	ofImage saveScreen;
//    ofxCvColorImage colorImg;
//    CvVideoWriter * writer;
	ofxThreadedImageSaver TIS;
#endif
	int snapCounter;

	ColorSampler *pInsidePalette;
	ColorSampler *pOutsidePalette;
	TextSampler *pTextSampler;
	GlyphSampler *pGlyphSampler;
	GlyphSampler *pBlankSampler;

	bool userAttract;
	bool mouseRepel;
	bool doMouseXY;		// pressing left mmouse button moves mouse in XY plane
	bool doMouseYZ;		// pressing right mouse button moves mouse in YZ plane
	bool doRender;
	int forceTimer;

	float rotSpeed;
	float mouseMass;

	int width;
	int kinectWidth;
	int height;
	int kinectHeight;
	float fromKinectWidth;
	float fromKinectHeight;
	float toKinectWidth;
	float toKinectHeight;

	ofxMSAPhysics physics;

	bool someoneThere;
//    SoundMachine sound;
	ofxXmlSettings XML;
	int minThreshold;
	int maxThreshold;
	int midDistance;
	int moteCount;
	bool fullscreen;

	//------------------ kinect methods and variables
	bool USE_KINECT;
	void setupKinect();
	void updateKinect();
	void drawKinect();
	bool isLive, isTracking, isCloud, isCPBkgnd, isMasking;
	bool isTrackingHands, isFiltering;

	ofxOpenNIContext recordContext;
	ofxDepthGenerator recordDepth;

	ofxImageGenerator recordImage;

	ofxHandGenerator recordHandTracker;

	ofxUserGenerator recordUser;

	void drawMasks();
	void drawAllUserMask();

	int nearThreshold, farThreshold;

	ofImage allUserMasks, user1Mask, user2Mask, depthRangeMask;

	float filterFactor;

};

#endif
