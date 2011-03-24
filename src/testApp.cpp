#include "testApp.h"
#include "DataMote.h"

//========================

#include "ofxMSAPhysics.h"


#define	SPRING_MIN_STRENGTH		0.005
#define SPRING_MAX_STRENGTH		0.1

#define	SPRING_MIN_WIDTH		1
#define SPRING_MAX_WIDTH		3

#define NODE_MIN_RADIUS			2
#define NODE_MAX_RADIUS			7

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


bool				mouseAttract	= false;
bool				doMouseXY		= false;		// pressing left mmouse button moves mouse in XY plane
bool				doMouseYZ		= false;		// pressing right mouse button moves mouse in YZ plane
bool				doRender		= true;
int					forceTimer		= false;


float				rotSpeed		= 0;
float				mouseMass		= 1;

static int			width;
static int			height;


ofxMSAPhysics		physics;
DataMote		    mouseNode;
//ofxMSAParticle	    mouseNode;

ofImage				ballImage;

DataMote* makeDataMote(ofPoint pos, float  m = 1.0f, float d = 1.0f)
{
    DataMote *p = new DataMote(pos, m, d);
    physics.addParticle(p);
    p->release();	// cos addParticle(p) retains it
    return p;
}

void initScene()
{
    // clear all particles and springs etc
    physics.clear();

    // you can add your own particles to the physics system
    physics.addParticle(&mouseNode);
    mouseNode.makeFixed();
    mouseNode.setMass(MIN_MASS);
    mouseNode.moveTo(ofPoint(0, 0, 0));
    mouseNode.setRadius(NODE_MAX_RADIUS);

    // or tell the system to create and add particles
//    makeDataMote(ofPoint(-width/4, 0, -width/4), MIN_MASS)->makeFixed();		// create a node in top left back and fix
//    makeDataMote(ofPoint( width/4, 0, -width/4), MIN_MASS)->makeFixed();		// create a node in top right back and fix
//    makeDataMote(ofPoint(-width/4, 0,  width/4), MIN_MASS)->makeFixed();		// create a node in top left front and fix
//    makeDataMote(ofPoint( width/4, 0,  width/4), MIN_MASS)->makeFixed();		// create a node in top right front and fix
}
void addRandomParticle()
{
    float posX		= ofRandom(0, width);
    float posY		= ofRandom(0, height);
    float posZ		= ofRandom(-width/2, width/2);
    float mass		= ofRandom(MIN_MASS, MAX_MASS);
    float bounce	= ofRandom(MIN_BOUNCE, MAX_BOUNCE);
    float radius	= ofMap(mass, MIN_MASS, MAX_MASS, NODE_MIN_RADIUS, NODE_MAX_RADIUS);

    // physics.makeParticle returns a particle pointer so you can customize it
    DataMote* p = makeDataMote(ofPoint(posX, posY, posZ));

    // and set a bunch of properties (you don't have to set all of them, there are defaults)
    p->setMass(mass)->setBounce(bounce)->setRadius(radius)->enableCollision()->makeFree();

    // add an attraction to the mouseNode
    if(mouseAttract) physics.makeAttraction(&mouseNode, p, ofRandom(MIN_ATTRACTION, MAX_ATTRACTION));
}

void addRandomSpring()
{
    ofxMSAParticle *a = physics.getParticle((int)ofRandom(0, physics.numberOfParticles()));
    ofxMSAParticle *b = physics.getParticle((int)ofRandom(0, physics.numberOfParticles()));
    physics.makeSpring(a, b, ofRandom(SPRING_MIN_STRENGTH, SPRING_MAX_STRENGTH), ofRandom(10, width/2));
}


void killRandomParticle()
{
    ofxMSAParticle *p = physics.getParticle(floor(ofRandom(0, physics.numberOfParticles())));
    if(p && p != &mouseNode) p->kill();
}

void killRandomSpring()
{
    ofxMSASpring *s = physics.getSpring( floor(ofRandom(0, physics.numberOfSprings())));
    if(s) s->kill();
}

void killRandomConstraint()
{
    ofxMSAConstraint *c = physics.getConstraint(floor(ofRandom(0, physics.numberOfConstraints())));
    if(c) c->kill();
}


void toggleMouseAttract()
{
    mouseAttract = !mouseAttract;
    if(mouseAttract) {
        // loop through all particles and add attraction to mouse
        // (doesn't matter if we attach attraction from mouse-mouse cos it won't be added internally
        for(int i=0; i<physics.numberOfParticles(); i++) physics.makeAttraction(&mouseNode, physics.getParticle(i), ofRandom(MIN_ATTRACTION, MAX_ATTRACTION));
    } else {
        // loop through all existing attractsions and delete them
        for(int i=0; i<physics.numberOfAttractions(); i++) physics.getAttraction(i)->kill();
    }
}

void addRandomForce(float f)
{
    forceTimer = f;
    for(int i=0; i<physics.numberOfParticles(); i++) {
        ofxMSAParticle *p = physics.getParticle(i);
        if(p->isFree()) p->addVelocity(ofPoint(ofRandom(-f, f), ofRandom(-f, f), ofRandom(-f, f)));
    }
}

void lockRandomParticles()
{
    for(int i=0; i<physics.numberOfParticles(); i++) {
        ofxMSAParticle *p = physics.getParticle(i);
        if(ofRandom(0, 100) < FIX_PROBABILITY) p->makeFixed();
        else p->makeFree();
    }
    mouseNode.makeFixed();
}

void unlockRandomParticles()
{
    for(int i=0; i<physics.numberOfParticles(); i++) {
        ofxMSAParticle *p = physics.getParticle(i);
        p->makeFree();
    }
    mouseNode.makeFixed();
}

void testApp::updateMoteLabel()
{
    const XnLabel* pLabels = oni.sceneMD.Data();
    XnLabel label;
    for(int i=0; i<physics.numberOfParticles(); i++) {
        DataMote *p = static_cast<DataMote*>(physics.getParticle(i));
        int x = p->getX();
        int y = p->getY();
        int z = p->getZ();
		label = pLabels[width*y+x];
        p->setLabel(label);
    }

}

//========================
//--------------------------------------------------------------
testApp::testApp()
{

}

testApp::~testApp()
{

}

//--------------------------------------------------------------
void testApp::setup()
{
    ofEnableAlphaBlending();
    ofSetWindowPosition(ofGetScreenWidth() - ofGetWidth() - 20, 20);

    oni.setup();

    // players
    for (int i = 0; i < MAX_PLAYERS; i++) players[i].allocate(oni.width, oni.height);

    //========================
#ifdef _COMMENT_OUT
    ofBackground(255, 255, 255);
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
#endif
    ballImage.loadImage("ball.png");

    width = ofGetWidth();
    height = ofGetHeight();

    //	physics.verbose = true;			// dump activity to log
//    physics.setGravity(ofPoint(0, GRAVITY, 0));
    physics.setGravity(ofPoint(0, GRAVITY/2, 0));

    // set world dimensions, not essential, but speeds up collision
    physics.setWorldSize(ofPoint(0, -height, 0), ofPoint(width, height, width));
    physics.setSectorCount(SECTOR_COUNT);
    physics.setDrag(0.97f);
    physics.setDrag(1);		// FIXTHIS
    physics.enableCollision();

    initScene();

#ifdef _COMMENT_OUT
    // setup lighting
    GLfloat mat_shininess[] = { 50.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 0, height/2, 0.0, 0.0 };
    glShadeModel(GL_SMOOTH);

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHT0);

    // enable back-face culling (so we can see through the walls)
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
#endif

    //========================
}

//--------------------------------------------------------------
void testApp::update()
{
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    oni.update();

    //========================
    width = ofGetWidth();
    height = ofGetHeight();

    physics.update();
    updateMoteLabel();

    //========================
}

//--------------------------------------------------------------
void testApp::draw()
{



    //========================
#ifdef _COMMENT_OUT
    if (false) {
        if(doRender) {

            ofEnableAlphaBlending();
            glEnable(GL_DEPTH_TEST);

            glPushMatrix();

            glTranslatef(width/2, 0, -width / 3);		// center scene
            static float rot = 0;
            glRotatef(rot, 0, 1, 0);			// rotate scene
            rot += rotSpeed;						// slowly increase rotation (to get a good 3D view)

            if(forceTimer) {
                float translateMax = forceTimer;
                glTranslatef(ofRandom(-translateMax, translateMax), ofRandom(-translateMax, translateMax), ofRandom(-translateMax, translateMax));
                forceTimer--;
            }


            glDisable(GL_LIGHTING);
            glBegin(GL_QUADS);
            // draw right wall
            glColor3f(.9, 0.9, 0.9);
            glVertex3f(width/2, height+1, width/2);
            glColor3f(1, 1, 1);
            glVertex3f(width/2, -height, width/2);
            glColor3f(0.95, 0.95, 0.95);
            glVertex3f(width/2, -height, -width/2);
            glColor3f(.85, 0.85, 0.85);
            glVertex3f(width/2, height+1, -width/2);

            // back wall
            glColor3f(.9, 0.9, 0.9);
            glVertex3f(width/2, height+1, -width/2);
            glColor3f(1, 1, 1);
            glVertex3f(width/2, -height, -width/2);
            glColor3f(0.95, 0.95, 0.95);
            glVertex3f(-width/2, -height, -width/2);
            glColor3f(.85, 0.85, 0.85);
            glVertex3f(-width/2, height+1, -width/2);

            // left wall
            glColor3f(.9, 0.9, 0.9);
            glVertex3f(-width/2, height+1, -width/2);
            glColor3f(1, 1, 1);
            glVertex3f(-width/2, -height, -width/2);
            glColor3f(0.95, 0.95, 0.95);
            glVertex3f(-width/2, -height, width/2);
            glColor3f(.85, 0.85, 0.85);
            glVertex3f(-width/2, height+1, width/2);

            // front wall
            glColor3f(0.95, 0.95, 0.95);
            glVertex3f(width/2, -height, width/2);
            glColor3f(.85, 0.85, 0.85);
            glVertex3f(width/2, height+1, width/2);
            glColor3f(.9, 0.9, 0.9);
            glVertex3f(-width/2, height+1, width/2);
            glColor3f(1, 1, 1);
            glVertex3f(-width/2, -height, width/2);

            // floor
            glColor3f(.8, 0.8, 0.8);
            glVertex3f(width/2, height+1, width/2);
            glVertex3f(width/2, height+1, -width/2);
            glVertex3f(-width/2, height+1, -width/2);
            glVertex3f(-width/2, height+1, width/2);
            glEnd();

//		glEnable(GL_LIGHTING);

            // draw springs
            glColor4f(0.5, 0.5, 0.5, 0.5);
            for(int i=0; i<physics.numberOfSprings(); i++) {
                ofxMSASpring *spring = (ofxMSASpring *) physics.getSpring(i);
                ofxMSAParticle *a = spring->getOneEnd();
                ofxMSAParticle *b = spring->getTheOtherEnd();
                ofPoint vec = b->getPosition() - a->getPosition();
                float dist = msaLength(vec);
                float angle = acos( vec.z / dist ) * RAD_TO_DEG;
                if(vec.z <= 0 ) angle = -angle;
                float rx = -vec.y * vec.z;
                float ry =  vec.x * vec.z;

                glPushMatrix();
                glTranslatef(a->getX(), a->getY(), a->getZ());
                glRotatef(angle, rx, ry, 0.0);
                float size  = ofMap(spring->strength, SPRING_MIN_STRENGTH, SPRING_MAX_STRENGTH, SPRING_MIN_WIDTH, SPRING_MAX_WIDTH);

                glScalef(size, size, dist);
                glTranslatef(0, 0, 0.5);
                glutSolidCube(1);
                glPopMatrix();
            }



            // draw particles
            glAlphaFunc(GL_GREATER, 0.5);

            ofEnableNormalizedTexCoords();
            ballImage.getTextureReference().bind();
            for(int i=0; i<physics.numberOfParticles(); i++) {
                ofxMSAParticle *p = physics.getParticle(i);
                if(p->isFixed()) glColor4f(1, 0, 0, 1);
                else glColor4f(1, 1, 1, 1);

                glEnable(GL_ALPHA_TEST);

                // draw ball
                glPushMatrix();
                glTranslatef(p->getX(), p->getY(), p->getZ());
                glRotatef(180-rot, 0, 1, 0);
//			glutSolidSphere(p->getRadius(), 2, 2);
//			ofCircle(0, 0, p->getRadius());
//			ballImage.draw(0, 0, p->getRadius()*2, p->getRadius()*2);
                glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex2f(-p->getRadius(), -p->getRadius());
                glTexCoord2f(1, 0);
                glVertex2f(p->getRadius(), -p->getRadius());
                glTexCoord2f(1, 1);
                glVertex2f(p->getRadius(), p->getRadius());
                glTexCoord2f(0, 1);
                glVertex2f(-p->getRadius(), p->getRadius());
                glEnd();
                glPopMatrix();

                glDisable(GL_ALPHA_TEST);

                float alpha = ofMap(p->getY(), -height * 1.5, height, 0, 1);
                if(alpha>0) {
                    glPushMatrix();
                    glTranslatef(p->getX(), height, p->getZ());
                    glRotatef(-90, 1, 0, 0);
                    glColor4f(0, 0, 0, alpha * alpha * alpha * alpha);
//				ofCircle(0, 0, p->getRadius());
                    float r = p->getRadius() * alpha;
                    glBegin(GL_QUADS);
                    glTexCoord2f(0, 0);
                    glVertex2f(-r, -r);
                    glTexCoord2f(1, 0);
                    glVertex2f(r, -r);
                    glTexCoord2f(1, 1);
                    glVertex2f(r, r);
                    glTexCoord2f(0, 1);
                    glVertex2f(-r, r);
                    glEnd();
                    glPopMatrix();
                }

            }
            ballImage.getTextureReference().unbind();
            ofDisableNormalizedTexCoords();


            glPopMatrix();
        }

        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glColor4f(0, 0, 0, 1);
        ofDrawBitmapString( " FPS: " + ofToString(ofGetFrameRate(), 2)
                            + " | Number of particles: " + ofToString(physics.numberOfParticles(), 2)
                            + " | Number of springs: " + ofToString(physics.numberOfSprings(), 2)
                            + " | Mouse Mass: " + ofToString(mouseNode.getMass(), 2)
                            , 20, 15);
    }
#endif
    //========================
    ofBackground(0, 0, 0);
//    glPushMatrix();
//
//    glScalef(ofGetWidth() / (float)oni.width, ofGetHeight() / (float)oni.height, 1);
//
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    ofSetColor(255, 255, 255);
//    if (oni.bDrawCam)
//        oni.drawCam(0, 0);
//    else
//        oni.drawDepth(0, 0);
//
//    ofSetColor(255, 255, 255, 200);
//    if (oni.bDrawPlayers)
//        oni.drawPlayers(0, 0);
//    glPopMatrix();

    oni.skeletonTracking();
    physics.draw();
}



//--------------------------------------------------------------
void testApp::keyPressed  (int key)
{
    if(key == '1') oni.bDrawPlayers = !oni.bDrawPlayers;
    if(key == '2') oni.bDrawCam = !oni.bDrawCam;
    switch(key) {
    case 'a':
        toggleMouseAttract();
        break;
    case 'p':
        for(int i=0; i<100; i++) addRandomParticle();
        break;
    case 'P':
        for(int i=0; i<100; i++) killRandomParticle();
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
        initScene();
        break;
    case 'x':
        doMouseXY = true;
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
    case '+':
        mouseNode.setMass(mouseNode.getMass() +0.1);
        physics.setGravity(ofPoint(0, GRAVITY, 0));
        break;
    case '-':
        mouseNode.setMass(mouseNode.getMass() -0.1);
        physics.setGravity(ofPoint(0, -GRAVITY, 0));
        break;
    case '0':
//        mouseNode.setMass(mouseNode.getMass() -0.1);
        physics.setGravity(ofPoint(0, 0, 0));
        break;
    case 'm':
        mouseNode.hasCollision() ? mouseNode.disableCollision() : mouseNode.enableCollision();
        break;
    }

}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{
    switch(key) {
    case 'x':
        doMouseXY = false;
        break;
    case 'z':
        doMouseYZ = false;
        break;
    }

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y )
{
    static int oldMouseX = -10000;
    static int oldMouseY = -10000;
    int velX = x - oldMouseX;
    int velY = y - oldMouseY;
    if(doMouseXY) mouseNode.moveBy(ofPoint(velX, velY, 0));
    if(doMouseYZ) mouseNode.moveBy(ofPoint(velX, 0, velY));
    oldMouseX = x;
    oldMouseY = y;

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{
    switch(button) {
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
void testApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{
    doMouseXY = doMouseYZ = false;

}

//--------------------------------------------------------------
void testApp::resized(int w, int h)
{

}
