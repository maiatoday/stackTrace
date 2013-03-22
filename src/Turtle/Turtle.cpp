/*
 *  Turtle.cpp
 *  Lsystem
 *
 *  Created by Daan on 02-04-11.
 *  Copyright 2011 Daan van Hasselt. All rights reserved.
 *
 */

#include "Turtle.h"

Turtle::Turtle()
{
    angle = 90;
    curAngle = 10;
    length = 10;
    x = ofGetWidth()/2;
    y = ofGetHeight()/2;
    myColor.r = 0;
    myColor.g = 0;
    myColor.b = 0;
    myColor.r = 120;
    if (ofRandom(0,5) > 4) {
        bsodColor.r = 0;
        bsodColor.g = 10;
        bsodColor.b = 145;
        bsodColor.a = 255;
    } else {
        bsodColor.r =255;
        bsodColor.g = 255;
        bsodColor.b = 255;
        bsodColor.a = 255;
    }
    fadeFactor = 1;
    pMyFont = NULL;
    buildNumber = 0;
    maxLeafDepth = 7;
}

Turtle::Turtle(string _forward, string _left, string _right)
{
    forward = _forward;
    left = _left;
    right = _right;

    angle = 90;
    curAngle = 10;
    length = 10;
    x = ofGetWidth()/2;
    y = ofGetHeight()/2;
    myColor.r = 0;
    myColor.g = 0;
    myColor.b = 0;
    myColor.r = 120;
    if (ofRandom(0,5) > 4) {
        bsodColor.r = 0;
        bsodColor.g = 10;
        bsodColor.b = 145;
        bsodColor.a = 255;
    } else {
        bsodColor.r =255;
        bsodColor.g = 255;
        bsodColor.b = 255;
        bsodColor.a = 255;
    }
    fadeFactor = 1;
    pMyFont = NULL;
    buildNumber = 0;
    maxLeafDepth = 7;
}

void Turtle::draw(string input, float _x, float _y, float _angle)
{
    buildNumber = 0;
    x = _x;
    y = _y;
    if (_angle != 0) curAngle = _angle;

    int length = input.length();	// length of the input string

    string substr[length];				// split into 1-char substrings
    for(int i = 0; i < length; i++) {
        substr[i] = input.substr(i,1);
    }

    for(int i = 0; i < length; i++) {		// check every character
        if(substr[i] == forward)			// and act accordingly
            moveForward();
        if(substr[i] == left)
            turnLeft();
        if(substr[i] == right)
            turnRight();
        if(substr[i] == "[")
            pushValues();
        if(substr[i] == "]")
            popValues();
    }
}

void Turtle::pushValues()
{
    xHis.push_back(x);
    yHis.push_back(y);
    aHis.push_back(curAngle);
    if (xHis.size() == maxLeafDepth) {
        drawVerString();
    }
}

void Turtle::popValues()
{
    x = xHis[xHis.size()-1];
    y = yHis[yHis.size()-1];
    curAngle = aHis[aHis.size()-1];

    xHis.pop_back();
    yHis.pop_back();
    aHis.pop_back();
}

void Turtle::moveForward()
{
	ofPushStyle();
    float newX = x + (cos(ofDegToRad(curAngle))*length);
    float newY = y + (sin(ofDegToRad(curAngle))*length);

    //cout << "move forward from: " << x << ", " << y << " to " << newX << ", " << newY << endl;
    ofEnableAlphaBlending();
//	ofSetColor(0, 0, 0, 120);
    ofSetColor(myColor.r, myColor.g, myColor.b, myColor.a*fadeFactor);
    ofSetLineWidth(2);
    ofLine(x, y, newX, newY);
    x = newX;
    y = newY;
    ofPopStyle();
}

void Turtle::turnLeft()
{
//	cout << "turn left" << endl;
    curAngle += angle;
}

void Turtle::turnRight()
{
//	cout << "turn right" << endl;
    curAngle -= angle;
}

void Turtle::drawVerString()
{
	ofPushStyle();
    labelBuildString = labelString;
    char bb[8];
    snprintf(bb, 8, "_%d", buildNumber);
    labelBuildString.append(bb);
    ofSetColor(bsodColor.r,bsodColor.g,bsodColor.b, bsodColor.a*fadeFactor);
    if ((pMyFont) && (length >= MAX_L_LENGTH)) pMyFont->drawString(labelBuildString, x+5,y+5);
    buildNumber++;
    ofPopStyle();

}
