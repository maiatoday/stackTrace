#include "StreamMote.h"
#include "LSystemStrings.h"
#include <stdlib.h>

#define MAX_LIFETIME (100)
#define MAX_VELOCITY (1.0)

#define MAX_STEP 100
#define MAX_FADE_COUNT (48.0)

StreamMote::StreamMote() :
		ofxMSAParticle() {
	init();
}

StreamMote::StreamMote(ofPoint pos, float m, float d) :
		ofxMSAParticle(pos, m, d) {
	init();
}

StreamMote::StreamMote(ofxMSAParticle &p) :
		ofxMSAParticle(p) {
	init();
}

StreamMote::~StreamMote() {
	//dtor
	// bleargh vector of pointers so must delete objects
	for (int i = 0; i < childMotes.size(); i++)
		delete childMotes[i];
	childMotes.clear();
}
void StreamMote::init() {
	mainStream = true;
	pMyFont = NULL;
	buildNumber = 0;
	frameStep = ofRandom(0, MAX_STEP);
	bsodColor.r = 0;
	bsodColor.g = 10;
	bsodColor.b = 145;
	bsodColor.a = 255;

	insideColor.a = 255;
	insideColor.r = 130;
	insideColor.g = 0;
	insideColor.b = 160;

	outsideColor.a = 130;
	outsideColor.r = 130;
	outsideColor.g = 130;
	outsideColor.b = 250;
	myAlpha = 255;
	addVelocity(ofPoint(0, ofRandom(-MAX_VELOCITY, MAX_VELOCITY), 0));

	pGlyph = NULL;
	label = 0;
//    addVelocity(ofPoint(ofRandom(-10, 10), ofRandom(-10, 10), ofRandom(-10, 10)));

	childColor = insideColor;
	childColor.a = CHILD_ALPHA;

	int childCount = ofRandom(30, 60);
	for (int i = 0; i < childCount; i++) {
		MoteHistory* newchild = new MoteHistory();
		childMotes.push_back(newchild);
	}
	turtle = Turtle("F", "-", "+");
	turtle.angle = ofRandom(10.0, 40.0);
	turtleLength = MIN_L_LENGTH;
	inc = 1;
	generationCounter = 0;
	fadefactor = 0;
	startAngle = 10;
}

void StreamMote::update() {
	if (frameStep == MAX_STEP) {
		buildNumber++;
		frameStep = 0;
		if (generationCounter < 2) {
			generationCounter++;
			turtleLength = MIN_L_LENGTH;
		}
	} else {
		frameStep++;
	}
	float f = 2;
	if (label == 0) {
		// ===no-one there===
		// adjust movement
//        addVelocity(ofPoint(0, ofRandom(-f, f), 0));

		turtleLength = MAX_L_LENGTH;
		if ((fadefactor <= MAX_FADE_COUNT)&& (fadefactor > 0)){
		fadefactor--;
		turtle.setFadeFactor(fadefactor/MAX_FADE_COUNT);
	}
} else {
	// ===someone there===       // adjust movement
//        float dist = getConstraintDelta()/maxDistWidthSquare;
//        if (dist>=0 && dist<0.01) {
//            addVelocity(ofPoint(ofRandom(-f, f), ofRandom(-f, f), ofRandom(-f, f)));
//        } else {
//        setVelocity(ofPoint(ofRandom(-f, f), ofRandom(-f, f), ofRandom(-f, f)));
//        }

	if (turtleLength <= MAX_L_LENGTH) {
		turtleLength += inc;
	}
//        if ((turtleLength >= MAX_L_LENGTH-1) ||  (turtleLength <= MIN_L_LENGTH)) {
//            // change directions if the pulse limit has been reached
//            inc = -inc;
//
//            if (inc > 0) turtle.angle = ofRandom(10.0, 40.0);
//        }
}

	if (childMotes.size() > 0) {
		for (int i = childMotes.size(); i > 1; i--) {
			childMotes[i - 1]->update(childMotes[i - 2]->getX(), childMotes[i - 2]->getY(), _radius - 2, true);
		}
		childMotes[0]->update(getX(), getY(), _radius - 2, true);
	}
	if ((!mainStream) && (buildNumber == 20)) {
		// harakiri
		kill();
	}
}

void StreamMote::draw() {

	drawTails();
	drawDormant();
	drawBranches();
	drawText();

}

void StreamMote::drawDormant() {
	ofPushStyle();
	float f = 2;
	myAlpha = ofLerp(START_ALPHA, STOP_ALPHA, _radius / NODE_MAX_RADIUS);
	myAlpha = ofClamp(myAlpha * fadefactor / MAX_FADE_COUNT, START_ALPHA, STOP_ALPHA);
	ofSetColor(outsideColor.r, outsideColor.g, outsideColor.b, myAlpha);
	ofFill();
	ofCircle(getX(), getY(), _radius);
//        if (!mainStream) {
	ofSetColor(outsideColor.r, outsideColor.g, outsideColor.b, START_ALPHA);
//        } else {
//            ofSetColor(255,0,0, STOP_ALPHA);
//        }
	ofNoFill();
	ofCircle(getX(), getY(), _radius);
	ofPopStyle();
}
void StreamMote::drawBranches() {

	ofPushStyle();
	if (fadefactor != 0) {
		turtle.length = turtleLength;
		switch (generationCounter) {
		default:
		case 0:
			turtle.setMaxLeafDepth(2);
			turtle.draw(L1, getX(), getY(), startAngle);
			break;
		case 1:
			turtle.setMaxLeafDepth(3);
			turtle.draw(L2, getX(), getY(), startAngle);
			break;
		case 2:
			turtle.setMaxLeafDepth(4);
			turtle.draw(L3, getX(), getY(), startAngle);
			break;
		}
	}
	ofPopStyle();
}
void StreamMote::drawTails() {
	if (label != 0) {
		for (int i = 0; i < childMotes.size(); i++) {
			childMotes[i]->draw();
		}
	}
}
void StreamMote::drawText() {
	ofPushStyle();
	if (label != 0) {
		labelBuildString = labelString;
		char bb[8];
		snprintf(bb, 8, "%d", buildNumber);
		labelBuildString.append(bb);
		turtle.setLabelString(labelBuildString);
		if (pMyFont)
			pMyFont->drawString(labelBuildString, getX() + 5, getY() + 5);
	}
	ofPopStyle();
}

void StreamMote::setLabel(const unsigned int _label) {
	float f = 2;
	if (label != _label) {
		if (_label == 0) {
			//leaving player
//            generationCounter = 2;
			fadefactor = MAX_FADE_COUNT; //TODO this should decrement eventually
		} else {
			// moving over player or different player
//            startAngle = ofRandom(0,270);
			ofPoint vv = getVelocity();
			startAngle = atan(vv.y / vv.z) * 180 / PI;
			if (startAngle == 0) {
				startAngle = 1;
			}
			generationCounter = 0;
			fadefactor = MAX_FADE_COUNT;
			turtle.setFadeFactor(1);
			turtleLength = MIN_L_LENGTH;
			if ((abs(vv.x) <= 1) && (abs(vv.y) <= 1) && (abs(vv.z) <= 1)) {
				addVelocity(ofPoint(ofRandom(-f, f), ofRandom(-f, f), ofRandom(-f, f)));
			}
		}
	}
	label = _label;
	if (label == 0) {
		if (mainStream) {
//            addVelocity(ofPoint(-10,-10,-10));
		} else {
			// ===not over someone=== harakiri
			kill();
		}
	} else {

		// adjust movement
//            setVelocity(ofPoint(ofRandom(-f, f), ofRandom(-f, f), ofRandom(-f, f)));
	}

}

void StreamMote::setLabelString(const std::string& _labelString) {
	labelString = _labelString;
	labelString.erase(std::remove(labelString.begin(), labelString.end(), '\n'), labelString.end());
}

void StreamMote::setFont(ofTrueTypeFont* _pMyFont) {
	pMyFont = _pMyFont;
	turtle.setFont(_pMyFont);
}

void StreamMote::setInsideColor(ofColor _newColor) {
	insideColor = _newColor;
	setChildColor(insideColor);
	turtle.setMyColor(_newColor);
}
void StreamMote::setOutsideColor(ofColor _newColor) {
	outsideColor = _newColor;
}
void StreamMote::setChildColor(ofColor _newColor) {
	childColor = _newColor;
	childColor.a = childColor.a / 2;
	for (int i = 0; i < childMotes.size(); i++) {
		childMotes[i]->setColor(childColor);
	}
}

void StreamMote::setFadeDist(int _distance) {
	maxDistWidthSquare = _distance * _distance;
}

void StreamMote::setGlyph(ofImage* _pnewglyph) {
	pGlyph = _pnewglyph;
	pCurrentImage = pGlyph;
}

void StreamMote::setBlankGlyph(ofImage* _pnewglyph) {
	pBlank = _pnewglyph;
}

