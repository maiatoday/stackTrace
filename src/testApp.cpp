#include "testApp.h"
#include "StreamMote.h"

//========================

#define	SPRING_MIN_STRENGTH		0.005
#define SPRING_MAX_STRENGTH		0.1

#define	SPRING_MIN_WIDTH		1
#define SPRING_MAX_WIDTH		3

#define MIN_MASS				1
#define MAX_MASS				3

#define MIN_BOUNCE				0.2
#define MAX_BOUNCE				0.9

#define	FIX_PROBABILITY			10		// % probability of a particle being fixed on creation
#define FORCE_AMOUNT			10

#define EDGE_DRAG				0.98

#define	GRAVITY					1

#define MAX_ATTRACTION			10
#define MIN_ATTRACTION			3

#define SECTOR_COUNT			10

#define START_MOTE_COUNT		64
#define MIN_THRESHOLD           200
#define MAX_THRESHOLD           6000
#define MID_DISTANCE            3000

StreamMote* testApp::makeStreamMote(ofPoint pos, float m = 1.0f, float d = 1.0f) {
	StreamMote* p = new StreamMote(pos, m, d);
	p->setInsideColor(pInsidePalette->getSampleColor());
	p->setOutsideColor(pOutsidePalette->getSampleColor());
	p->setChildColor(pInsidePalette->getSampleColor());
	p->setLabelString(pTextSampler->getSampleText());
	p->setGlyph(pGlyphSampler->getSampleGlyph());
	p->setBlankGlyph(pBlankSampler->getSampleGlyph());
	p->setFadeDist(width * 0.6);
	p->disableCollision();
	physics.addParticle(p);
	p->release();	// cos addParticle(p) retains it
	return p;
}

void testApp::initScene() {
	// clear all particles and springs etc
	physics.clear();

}

void testApp::addRandomParticle() {
	float posX = ofRandom(0, width);
	float posY = ofRandom(0, height);
	float posZ = ofRandom(-width / 2, width / 2);
	float mass = ofRandom(MIN_MASS, MAX_MASS);
	float bounce = ofRandom(MIN_BOUNCE, MAX_BOUNCE);
	float radius = ofMap(mass, MIN_MASS, MAX_MASS, NODE_MIN_RADIUS * fromKinectWidth,
			NODE_MAX_RADIUS * fromKinectWidth);

	// physics.makeParticle returns a particle pointer so you can customize it
	StreamMote* p = makeStreamMote(ofPoint(posX, posY, posZ));

	// and set a bunch of properties (you don't have to set all of them, there are defaults)
	p->setMass(mass)->setBounce(bounce)->setRadius(radius)->makeFree()->disableCollision();
	p->setFont(&myFont);

}

void testApp::addRandomSpring() {
	ofxMSAParticle *a = physics.getParticle((int) ofRandom(0, physics.numberOfParticles()));
	ofxMSAParticle *b = physics.getParticle((int) ofRandom(0, physics.numberOfParticles()));
	physics.makeSpring(a, b, ofRandom(SPRING_MIN_STRENGTH, SPRING_MAX_STRENGTH), ofRandom(10, width / 2));
}

void testApp::killRandomParticle() {
	ofxMSAParticle *p = physics.getParticle(floor(ofRandom(0, physics.numberOfParticles())));
//    if(p && p != &mouseNode) p->kill();
//    if(p && p != pAttractMote && p != pRepelMote) p->kill();
}

void testApp::killRandomSpring() {
	ofxMSASpring *s = physics.getSpring(floor(ofRandom(0, physics.numberOfSprings())));
	if (s)
		s->kill();
}

void testApp::killRandomConstraint() {
	ofxMSAConstraint *c = physics.getConstraint(floor(ofRandom(0, physics.numberOfConstraints())));
	if (c)
		c->kill();
}

void testApp::addRandomForce(float f) {
	forceTimer = f;
	for (unsigned int i = 0; i < physics.numberOfParticles(); i++) {
		ofxMSAParticle *p = physics.getParticle(i);
		if (p->isFree())
			p->addVelocity(ofPoint(ofRandom(-f, f), ofRandom(-f, f), ofRandom(-f, f)));
	}
}

void testApp::lockRandomParticles() {
	for (unsigned int i = 0; i < physics.numberOfParticles(); i++) {
		ofxMSAParticle *p = physics.getParticle(i);
		if (ofRandom(0, 100) < FIX_PROBABILITY)
			p->makeFixed();
		else
			p->makeFree();
	}
//    mouseNode.makeFixed();
}

void testApp::unlockRandomParticles() {
	for (unsigned int i = 0; i < physics.numberOfParticles(); i++) {
		ofxMSAParticle *p = physics.getParticle(i);
		p->makeFree();
	}
//    mouseNode.makeFixed();
}

void testApp::setUserAttract(bool _attractOn) {
	userAttract = _attractOn;
////    ofPoint attractPoint = pAttractMote->getPosition();
//    printf("attract point x %f y %f z %f\n", attractPoint.x, attractPoint.y, attractPoint.z);
	if (userAttract) {
		// loop through all particles and add attraction to mouse
		// (doesn't matter if we attach attraction from mouse-mouse cos it won't be added internally
//        if (pAttractMote->getX() != 0) {
//            for(unsigned int i=0; i<physics.numberOfAttractions(); i++) physics.getAttraction(i)->turnOn();
//        }
//        if (pRepelMote->getX() != 0) {
//            for(unsigned int i=0; i<physics.numberOfParticles(); i++) physics.makeAttraction(pRepelMote, physics.getParticle(i), ofRandom(-MIN_ATTRACTION, -MAX_ATTRACTION));
//        }
	} else {
		// loop through all existing attractsions and delete them
		for (unsigned int i = 0; i < physics.numberOfAttractions(); i++)
			physics.getAttraction(i)->turnOff();
	}
}

void testApp::updateParticles() {

	physics.update();
	bool doFork = false;
	int userCount = recordUser.getNumberOfTrackedUsers();
	if ((numberUsers == 0) && (userCount > 0)) {
		//someone arrived
		doFork = true;
		someoneThere = true;
		//        ofBackground(255, 255,255);
		//        ofSetBackgroundAuto(false);
	} else if ((userCount == 0) && (numberUsers > 0)) {
		//last person left
		doFork = false;
		someoneThere = false;
		//        ofBackground(0,0,0);
		//        ofSetBackgroundAuto(true);
	}
	numberUsers = userCount;

	for (unsigned int i = 0; i < physics.numberOfParticles(); i++) {
		StreamMote *p = static_cast<StreamMote*>(physics.getParticle(i));

		int x = p->getX() * toKinectWidth;
		int y = p->getY() * toKinectHeight;
		//        int z = p->getZ();
		if (USE_KINECT && isTracking && isMasking) {
			char c = allUserMasks.getPixelsRef()[allUserMasks.width * y + x];
			p->setLabel(c);
		}
	}
}

//========================
//--------------------------------------------------------------
testApp::testApp() :
		minThreshold(MIN_THRESHOLD), maxThreshold(MAX_THRESHOLD), midDistance(MID_DISTANCE), moteCount(
				START_MOTE_COUNT), fullscreen(false) {
	pInsidePalette = new ColorSampler("images/inside.jpg");
	pOutsidePalette = new ColorSampler("images/outside.jpg");
	pTextSampler = new TextSampler("data/text/sample.txt");
	pGlyphSampler = new GlyphSampler("data/images/glyphs");
	pBlankSampler = new GlyphSampler("data/images/erase");
	numberUsers = 0;
	flipCount = 0;
	userAttract = false;
	mouseRepel = false;
	doMouseXY = false;		// pressing left mmouse button moves mouse in XY plane
	doMouseYZ = false;		// pressing right mouse button moves mouse in YZ plane
	doRender = true;
	forceTimer = false;
	rotSpeed = 0;
	mouseMass = 1;

}

testApp::~testApp() {
	delete pInsidePalette;
	delete pOutsidePalette;
	delete pTextSampler;
	delete pGlyphSampler;
	delete pBlankSampler;

}

void testApp::setScreenRatios(void) {
	int windowMode = ofGetWindowMode();

	kinectWidth = 640;
	kinectHeight = 480;
	if (windowMode == OF_FULLSCREEN) {
		width = ofGetScreenWidth();
		height = ofGetScreenHeight();
	} else if (windowMode == OF_WINDOW) {
		width = ofGetWidth();
		height = ofGetHeight();
	}

	fromKinectWidth = (float) width / (float) kinectWidth;
	fromKinectHeight = (float) height / (float) kinectHeight;
	toKinectWidth = (float) kinectWidth / (float) width;
	toKinectHeight = (float) kinectHeight / (float) height;
	physics.setWorldSize(ofPoint(0, 0, 0), ofPoint(width, height, width));
}
//--------------------------------------------------------------
void testApp::setup() {
	if (XML.loadFile("mySettings.xml")) {
		minThreshold = XML.getValue("ROOM:THRESHOLD:MIN", MIN_THRESHOLD);
		maxThreshold = XML.getValue("ROOM:THRESHOLD:MAX", MAX_THRESHOLD);
		midDistance = XML.getValue("ROOM:MIDDLE", MID_DISTANCE);
		moteCount = XML.getValue("ROOM:MOTE_COUNT", START_MOTE_COUNT);
		fullscreen = (XML.getValue("ROOM:FULLSCREEN", 1) == 1) ? true : false;
		USE_KINECT = (XML.getValue("ROOM:KINECT", 1) == 1) ? true : false;
		FRAME_BUFFER_SPLIT = (XML.getValue("ROOM:FRAME_BUFFER_SPLIT", 1) == 1) ? true : false;
	}
	if (USE_KINECT) {
		setupKinect();
	}
	someoneThere = false;
	ofBackground(0, 0, 0);
//    ofBackground(255,255,255);
	ofSetBackgroundAuto(true);
//	ofSetWindowPosition(ofGetScreenWidth() - ofGetWidth() - 20, 20);

//#ifndef NO_KINECT
//    oni.setup();
//
//    // players
//    for (int i = 0; i < MAX_PLAYERS; i++) players[i].allocate(oni.width, oni.height);
//#endif
	//========================

	ofSetFullscreen(fullscreen);
	ofHideCursor();
	setScreenRatios();
	if (FRAME_BUFFER_SPLIT) {
		ofEnableBlendMode(OF_BLENDMODE_SCREEN);
		people.allocate(width, height);
		particlesDormant.allocate(width, height);
		branches.allocate(width, height);
		versions.allocate(width, height);
	} else {
		ofEnableAlphaBlending();
	}

// font needs to be loaded before the particles are created because they all use it to draw
	myFont.loadFont("verdana.ttf", (int) 8 * fromKinectWidth);

	//	physics.verbose = true;			// dump activity to log
//    physics.setGravity(ofPoint(0, GRAVITY, 0));
	physics.setGravity(ofPoint(0, 0, 0));

	// set world dimensions, not essential, but speeds up collision
	physics.setWorldSize(ofPoint(0, 0, 0), ofPoint(width, height, width));
//    physics.clearWorldSize();
	physics.setSectorCount(SECTOR_COUNT);
	physics.setDrag(0.97f);
	physics.setDrag(1);		// FIXTHIS
	physics.enableCollision();

	initScene();
	for (int i = 0; i < moteCount; i++)
		addRandomParticle();

//    for(unsigned int i=0; i<physics.numberOfParticles(); i++) physics.makeAttraction(pAttractMote, physics.getParticle(i), ofRandom(MIN_ATTRACTION, MAX_ATTRACTION));
	for (unsigned int i = 0; i < physics.numberOfAttractions(); i++)
		physics.getAttraction(i)->turnOff();

//    addRandomForce(FORCE_AMOUNT);
	//========================

#ifdef DO_VIDEO
//    writer = cvCreateVideoWriter(
//                 "test.avi",
//                 CV_FOURCC('M','J','P','G'),
//                 15,
//                 size);
#endif
	snapCounter = 0;
//	width = ofGetWidth();
//	height = ofGetHeight();
}

//--------------------------------------------------------------
void testApp::update() {
	if (USE_KINECT) {
		updateKinect();
	}

	XnUInt16 nUsersPrev = numberUsers;
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
	//========================

	updateParticles();
	if (numberUsers != nUsersPrev) {
		printf(" numberUsers %d", numberUsers);
	}
}

//--------------------------------------------------------------
void testApp::draw() {
	if (FRAME_BUFFER_SPLIT) {
		people.begin();
		ofClear(0, 0, 0, 0);
		if (USE_KINECT) {
			drawAllUserMask();
		}
		people.end();

		particlesDormant.begin();
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 100);
		drawAllparticles();
		particlesDormant.end();

		branches.begin();
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 100);
		drawAllBranches();
		branches.end();

		versions.begin();
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 100);
		drawAllVersion();
		versions.end();

		versions.draw(0, 0);
		branches.draw(0, 0);
		particlesDormant.draw(0, 0);
		people.draw(0, 0);
	} else {
		if (USE_KINECT) {
			drawAllUserMask();
		}
		ofSetColor(255, 255, 255, 100);
		physics.draw();
		if (doVideoWrite) {

#ifdef DO_VIDEO

//        IplImage * tempImg = cvCreateImage(
//                                 cvSize(cameraWidth,cameraHeight),
//                                 IPL_DEPTH_8U,
//                                 3);
//        saveScreen.grabScreen(screenWidth-cameraWidth,screenHeight-cameraHeight,cameraWidth,cameraHeight);
//        colorImg.setFromPixels(saveScreen.getPixels(), cameraWidth,cameraHeight);
//        cvCvtColor(colorImg.getCvImage(), tempImg, CV_RGB2BGR);
//        cvWriteFrame(writer,tempImg);
			saveScreen.grabScreen(0,0,width,height);
			TIS.saveThreaded(saveScreen);
#endif
		}

	}

}

//--------------------------------------------------------------
void testApp::keyPressed(int key) {
	switch (key) {
	case 'a':
		setUserAttract(!userAttract);
		break;
	case 'p':
		for (int i = 0; i < 100; i++)
			addRandomParticle();
		break;
	case 'P':
		for (int i = 0; i < 100; i++)
			killRandomParticle();
		break;
	case 's':
		addRandomSpring();
		break;
	case 'S':
		killRandomSpring();
		break;
	case 'c':
		physics.isCollisionEnabled() ? physics.disableCollision() : physics.enableCollision();
		break;
	case 'C':
		killRandomConstraint();
		break;
	case 'r':
		doRender ^= true;
		break;
	case 'f':
		addRandomForce(FORCE_AMOUNT);
		break;
	case 'F':
		addRandomForce(FORCE_AMOUNT * 3);
		break;
	case 'l':
		lockRandomParticles();
		break;
	case 'u':
		unlockRandomParticles();
		break;
	case ' ':
//        initScene();
		someoneThere = !someoneThere;
		if (someoneThere) {
//            ofBackground(255, 255,255);
		} else {
//            ofBackground(0,0,0);
		}
//        ofSetBackgroundAuto(!someoneThere);
		break;
	case 'x':
		doMouseXY = true;
		someoneThere = !someoneThere;
		break;
	case 't':
		ofToggleFullscreen();
		setScreenRatios();
		break;
	case 'z':
		doMouseYZ = true;
		break;
	case ']':
		rotSpeed += 0.01f;
		break;
	case '[':
		rotSpeed -= 0.01f;
		break;
	case '+': {
//        mouseNode.setMass(mouseNode.getMass() +0.1);
		physics.setGravity(ofPoint(GRAVITY, 0, 0));
	}
		break;
	case '-':
//        mouseNode.setMass(mouseNode.getMass() -0.1);
		physics.setGravity(ofPoint(-GRAVITY, 0, 0));
		break;
	case '0':
//        mouseNode.setMass(mouseNode.getMass() -0.1);
		physics.setGravity(ofPoint(0, 0, 0));
		break;
	case 'm':
//        mouseNode.hasCollision() ? mouseNode.disableCollision() : mouseNode.enableCollision();
		break;
	case '`':
		int w = ofGetWidth();
		int h = ofGetHeight();
		ofImage screenImg;
		screenImg.allocate(w, h, OF_IMAGE_COLOR);
		screenImg.grabScreen(0, 0, w, h);
		screenImg.saveImage("screenshot-" + ofToString(snapCounter) + ".png");

		if (FRAME_BUFFER_SPLIT) {
			string name = "people";
			snapFrameBuffer(name, people, snapCounter);
			name = "versions";
			snapFrameBuffer(name, versions, snapCounter);
			name = "branches";
			snapFrameBuffer(name, branches, snapCounter);
			name = "particles";
			snapFrameBuffer(name, particlesDormant, snapCounter);
		}
		snapCounter++;
		break;
	}

}

//--------------------------------------------------------------
void testApp::keyReleased(int key) {
	switch (key) {
	case 'x':
		doMouseXY = false;
		break;
	case 'z':
		doMouseYZ = false;
		break;
	}

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button) {
	switch (button) {
	case 0:
		doMouseXY = true;
		mouseMoved(x, y);
		break;
	case 2:
		doMouseYZ = true;
		mouseMoved(x, y);
		break;
	}

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button) {
	doMouseXY = doMouseYZ = false;
	doVideoWrite = !doVideoWrite;

}

//--------------------------------------------------------------
void testApp::resized(int w, int h) {

}

void testApp::setupKinect() {
	isLive = true;
	isTracking = true;
	isTrackingHands = false;
	isFiltering = false;
	isCloud = false;
	isCPBkgnd = true;
	isMasking = true;

	nearThreshold = 500;
	farThreshold = 1000;

	filterFactor = 0.1f;

	ofBackground(0, 0, 0);

	recordContext.setup();	// all nodes created by code -> NOT using the xml config file at all
//recordContext.setupUsingXMLFile();
	recordDepth.setup(&recordContext);
	recordImage.setup(&recordContext);

	recordUser.setup(&recordContext);
	recordUser.setSmoothing(filterFactor);				// built in openni skeleton smoothing...
	recordUser.setUseMaskPixels(isMasking);
	recordUser.setUseCloudPoints(isCloud);
	recordUser.setMaxNumberOfUsers(MAX_NUMBER_USERS);// use this to set dynamic max number of users (NB: that a hard upper limit is defined by MAX_NUMBER_USERS in ofxUserGenerator)

	if (isTrackingHands) {
		recordHandTracker.setup(&recordContext, 4);
		recordHandTracker.setSmoothing(filterFactor);		// built in openni hand track smoothing...
		recordHandTracker.setFilterFactors(filterFactor);// custom smoothing/filtering (can also set per hand with setFilterFactor)...set them all to 0.1f to begin with
	}
	recordContext.toggleRegisterViewport();
	recordContext.toggleMirror();

}
void testApp::updateKinect() {
// update all nodes
	recordContext.update();
	recordDepth.update();
	recordImage.update();

// demo getting depth pixels directly from depth gen
	depthRangeMask.setFromPixels(recordDepth.getDepthPixels(nearThreshold, farThreshold), recordDepth.getWidth(),
			recordDepth.getHeight(), OF_IMAGE_GRAYSCALE);

// update tracking/recording nodes
	if (isTracking)
		recordUser.update();

// demo getting pixels from user gen
	if (isTracking && isMasking) {
		allUserMasks.setFromPixels(recordUser.getUserPixels(), recordUser.getWidth(), recordUser.getHeight(),
				OF_IMAGE_GRAYSCALE);
//		user1Mask.setFromPixels(recordUser.getUserPixels(1), recordUser.getWidth(), recordUser.getHeight(),
//				OF_IMAGE_GRAYSCALE);
//		user2Mask.setFromPixels(recordUser.getUserPixels(2), recordUser.getWidth(), recordUser.getHeight(),
//				OF_IMAGE_GRAYSCALE);
	}

}
void testApp::drawKinect() {
	ofSetColor(255, 255, 255);

	glPushMatrix();
	glScalef(0.75, 0.75, 0.75);

	if (isLive) {

		recordDepth.draw(0, 0, 640, 480);
		recordImage.draw(640, 0, 640, 480);

		depthRangeMask.draw(0, 480, 320, 240);// can use this with openCV to make masks, find contours etc when not dealing with openNI 'User' like objects

		if (isTracking) {
			recordUser.draw();

			if (isMasking)
				drawMasks();

		}
		if (isTrackingHands)
			recordHandTracker.drawHands();
	}

}
void testApp::drawMasks() {
//	glPushMatrix();
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
//	allUserMasks.draw(640, 0, 640, 480);
//	glDisable(GL_BLEND);
//	glPopMatrix();
//	user1Mask.draw(320, 480, 320, 240);
//	user2Mask.draw(640, 480, 320, 240);

}

void testApp::drawAllUserMask() {
	float coin = ofRandom(0.0, 1.0);
	ofPushStyle();
	if (coin < 0.92) {
		ofSetColor(50, 50, 50, 50);
	} else {
		ofSetColor(0, 10, 145, 80);
	}
	ofPushMatrix();
//	glEnable(GL_BLEND);
	ofScale(fromKinectWidth, fromKinectHeight, 0);
//	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	allUserMasks.draw(0, 0);
//	depthRangeMask.draw(0, 0, width, height);
//	glDisable(GL_BLEND);
	ofPopMatrix();
	ofPopStyle();

}

void testApp::drawAllparticles() {
	for (unsigned int i = 0; i < physics.numberOfParticles(); i++) {
			StreamMote *p = static_cast<StreamMote*>(physics.getParticle(i));
			p->drawTails();
			p->drawDormant();
	}
}
void testApp::drawAllBranches() {
	for (unsigned int i = 0; i < physics.numberOfParticles(); i++) {
			StreamMote *p = static_cast<StreamMote*>(physics.getParticle(i));
			p->drawBranches();
	}
}

void testApp::drawAllVersion() {
	for (unsigned int i = 0; i < physics.numberOfParticles(); i++) {
			StreamMote *p = static_cast<StreamMote*>(physics.getParticle(i));
			p->drawText();
	}
}


void testApp::snapFrameBuffer(string& name, ofFbo& fb, int count){
	//get the frame buffer pixels
	ofPixels pixels;
	    fb.readToPixels(pixels);
	    //save
	    ofSaveImage(pixels, name +"-" + ofToString(count) + ".png");
}

