#include "TreeScene.h"


// ------------------------------------------
void TreeScene::setup() {
	
	bDebug = true;
	butterFlyColor.push_back(0xd1007e);
	butterFlyColor.push_back(0x00a3c4);
	butterFlyColor.push_back(0x6ac539);
	butterFlyColor.push_back(0xff5500);
	butterFlyColor.push_back(0xff0009);
	
	theDot.loadImage("sceneAssets/monsters/dot.png");
	theDotS.loadImage("sceneAssets/trees/littleDot.png");
	
	
	
	// ------------ Control Panel For Trees
	panel.setup("Tree Scene", 700, 10, 300, 750);
	panel.addPanel("Triggers", 1, false);
	
	panel.setWhichPanel("Triggers");
	panel.addSlider("Tree Delay", "TREE_DELAY", 1.3, 0.0, 10.0, false);
	panel.addSlider("people glow", "PEOPLE_GLOW", 20.4, 0.0, 255.0, false);
	panel.addSlider("tree bottom offset", "TREE_OFF", 20.4, -800, 800.0, false);
	
	panel.addSlider("tree grow w", "TREE_GROW_W", 100.0, 0.0, 1000.0, false);
	panel.addSlider("tree grow h", "TREE_GROW_H", 100.0, 0.0, 1000.0, false);
	panel.addSlider("tree min pts", "TREE_MIN", 10.0, 0.0, 20.0, false);
	panel.addSlider("tree max pts", "TREE_MAX", 20.0, 20.0, 100.0, false);
	
	panel.addToggle("do people glow", "BPEOPLE_GLOW", 1);
	
	
	panel.loadSettings("settings/panels_xml/treeScenePanel.xml");
	
	
	// ------------ The Ferr Contour
	building.setupBuilding("buildingRefrences/buidlingFiles/treeFerryContour.xml");
	
	
	
	// ------------ Load some SVG Files for the Ferns 
	RandomFern tempFern;
	tempFern.addLeaf("sceneAssets/trees/Fern_Leaf_001.svg");
	tempFern.addLeaf("sceneAssets/trees/Fern_Leaf_002.svg");
	tempFern.addLeaf("sceneAssets/trees/Fern_Leaf_003.svg");
	
	
	
	// ------------ Settins 
	bGotMyFirstPacket	= false;
	
	// ------------ Get some Flocling butterflys in
	ButterflyAnimation tempButterFly;
	tempButterFly.addFile("sceneAssets/trees/Butterfly_001.svg");
	tempButterFly.addFile("sceneAssets/trees/Butterfly_002.svg");
	tempButterFly.addFile("sceneAssets/trees/Butterfly_003.svg");
	tempButterFly.addFile("sceneAssets/trees/Butterfly_004.svg");
	
	// make the butterflys
	if(butterflys.size() < MAX_BUTTERFLYS) {
		for (int i = 0; i < MAX_BUTTERFLYS; i++){
			ButterFlyParticle bf;
			bf.setInitialCondition(ofRandom(200, OFFSCREEN_WIDTH), ofRandom(0, OFFSCREEN_HEIGHT), 0,0);
			bf.setupButterfly();
			
			butterflys.push_back(bf);
			butterflys.back().img = &theDot;
			butterflys.back().color = butterFlyColor[(int)ofRandom(0, butterFlyColor.size()-1)];
		}
	}
	
	
}


//--------------------------------------------------------------
void TreeScene::cleanUpScene() {
	
	
	for(int i=treeBlobs.size()-1; i>=0; i--) {
		treeBlobs.erase(treeBlobs.begin() + i);
	}
	treeBlobs.clear();
	
	
	for(int i=trees.size()-1; i>=0; i--) {
		trees.erase(trees.begin() + i);
	}
	trees.clear();
	
	
}


//--------------------------------------------------------------
void TreeScene::keyPressed(int key) {
	// keys for the ferry
	building.keyPressed(key);	
	
	if(key == 'd') bDebug = !bDebug;
}

//--------------------------------------------------------------
void TreeScene::mouseDragged(int wx, int wy, int x, int y, int button){
	panel.mouseDragged(wx, wy, button);
}

//--------------------------------------------------------------
void TreeScene::mousePressed(int wx, int wy, int x, int y, int button){
	panel.mousePressed(wx, wy, button);
	
	// events for the ferry building
	building.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void TreeScene::mouseReleased(int wx, int wy, int x, int y, int button){
	panel.mouseReleased();
}




//--------------------------------------------------------------
void TreeScene::updateFlocking() {
	
	float scalex =  (float)OFFSCREEN_WIDTH / (float)packet.width;
	float scaley = (float)OFFSCREEN_HEIGHT / (float)packet.height;
	
	
	// on every frame 
	// we reset the forces
	// add in any forces on the particle
	// perfom damping and
	// then update
	
	for (int i = 0; i < butterflys.size(); i++){
		butterflys[i].resetForce();
	}
	
	for (int i = 0; i < butterflys.size(); i++){
		for (int j = 0; j < butterflys.size(); j++){
			if (i != j){
				butterflys[i].addForFlocking(butterflys[j]);	
			}
		}
		
		
		for(int q=0; q<tracker->blobs.size(); q++) {
			
			ofRectangle newRec = tracker->blobs[q].boundingRect;
			
			float forceFactor = (newRec.width * newRec.height) / (packet.width*packet.height);
			
			newRec.x		*= scalex;
			newRec.y		*= scaley;
			newRec.width	*= scalex;
			newRec.height	*= scaley;
			ofPoint center;
			center.x = tracker->blobs[q].centroid.x * scalex;
			center.y = tracker->blobs[q].centroid.y * scaley;
			
			
			butterflys[i].addAttractionForce(center.x, center.y - 20, 200, 1.0);
			
			
		}
	}
	
	for (int i = 0; i < butterflys.size(); i++){
		butterflys[i].addFlockingForce();
		butterflys[i].addDampingForce();
		butterflys[i].update();
	}
	
	// wrap torroidally.
	for (int i = 0; i < butterflys.size(); i++){
		ofxVec2f pos = butterflys[i].pos;
		
		float screenW = 0;
		float screenH = 0;
		
		float offx = ((OFFSCREEN_WIDTH - screenW)/2);
		float offy = (OFFSCREEN_HEIGHT - screenH);
		float gap  = 20;
		if (pos.x < -offx)						pos.x = OFFSCREEN_WIDTH-offx;
		if (pos.x > OFFSCREEN_WIDTH-offx)		pos.x = -offx;
		if (pos.y < -(offy+gap))				pos.y = offy;
		if (pos.y > offy)						pos.y = - (offy + gap);
		
		butterflys[i].pos = pos;
	}
	
	
	
}




// ------------------------------------------
void TreeScene::update() {
	
	panel.update();
	
	float scalex = (float)OFFSCREEN_WIDTH / (float)packet.width;
	float scaley = (float)OFFSCREEN_HEIGHT / (float)packet.height;
	
	
	// --------------------- Tree Blobs
	for(int i=0; i<tracker->blobs.size(); i++) {
		
		int lookID = tracker->blobs[i].id;
		
		ofRectangle newRec = tracker->blobs[i].boundingRect;
		newRec.x		*= scalex;
		newRec.y		*= scaley;
		newRec.width	*= scalex;
		newRec.height	*= scaley;
		
		ofPoint center =  tracker->blobs[i].centroid;
		center.x		= center.x * scalex;
		center.y		= center.y * scaley;
		
		
		for(int j=0; j<treeBlobs.size(); j++) {
			
			// found you :)
			if(lookID == treeBlobs[j].id) {
				
				treeBlobs[j].age		= ofGetElapsedTimef() - treeBlobs[j].initTime; // getting older
				treeBlobs[j].rect	    = newRec;
				treeBlobs[j].center     = center;
				
				// ok we are old enough lets make a blob
				if(treeBlobs[j].age >= panel.getValueF("TREE_DELAY") && treeBlobs[j].bAlive == false) {
					
					printf("--- time to grow a tree here:%i---\n", treeBlobs[j].id);
					treeBlobs[j].bAlive = true;
					
					// time to grow a tree
					printf("--- time to grow a tree:%i---\n", treeBlobs[j].id);
					trees.push_back(MagicTree());
					
					trees.back().initTree(0, 0, (int)ofRandom(panel.getValueF("TREE_MIN"), panel.getValueF("TREE_MAX")));	// <--- i need a init pos
					trees.back().img	= &theDot;
					trees.back().id		= treeBlobs[j].id; 
					trees.back().rect	= treeBlobs[j].rect;
					trees.back().center = treeBlobs[j].center;
					
					// init the tree pos
					trees.back().treeBaseD   = trees.back().center;
					trees.back().treeBaseD.y = (newRec.y + newRec.height);
					trees.back().treeBase	 = trees.back().treeBaseD;
					
					
				}
				
			}
		}
		
	}
	
	
	
	// --------------------- The Magic Trees
	/*
	 for(int i=0; i<tracker->blobs.size(); i++) {
	 
	 int lookID = tracker->blobs[i].id;
	 
	 ofRectangle& newRec = tracker->blobs[i].boundingRect;
	 newRec.x		*= scalex;
	 newRec.y		*= scaley;
	 newRec.width	*= scalex;
	 newRec.height	*= scaley;
	 
	 for(int j=0; j<trees.size(); j++) {
	 if(lookID == treeBlobs[j].id) {
	 
	 treeBlobs[j].age		= ofGetElapsedTimef() - treeBlobs[j].initTime;
	 treeBlobs[j].rect	    = newRec;
	 treeBlobs[j].center.x   = tracker->blobs[i].centroid.x * scalex;
	 treeBlobs[j].center.y   = tracker->blobs[i].centroid.y * scaley;
	 
	 }
	 }
	 
	 }
	 */
	
	
	
	/*
	 // --------------------- Tree Blobs
	 for(int i=0; i<treeBlobs.size(); i++) {
	 
	 //treeBlobs[i].age = (float)(ofGetElapsedTimef()-treeBlobs[i].initTime);	
	 
	 
	 if(treeBlobs[i].age > panel.getValueF("TREE_DELAY") && !treeBlobs[i].bAlive) {
	 treeBlobs[i].bAlive = true;	
	 
	 // time to grow a tree
	 printf("--- time to grow a tree:%i---\n", treeBlobs[i].id);
	 trees.push_back(MagicTree());
	 
	 trees.back().initTree(0, 0, (int)ofRandom(20, 50));	// <--- i need a init pos
	 trees.back().img = &theDot;
	 trees.back().id = treeBlobs[i].id; 
	 
	 for(int j=0; j<tracker->blobs.size(); j++) {
	 
	 ofRectangle& newRec = tracker->blobs[j].boundingRect;
	 newRec.x		*= scalex;
	 newRec.y		*= scaley;
	 newRec.width	*= scalex;
	 newRec.height	*= scaley;
	 
	 trees.back().rect	    = newRec;
	 trees.back().center.x   = tracker->blobs[j].centroid.x * scalex;
	 trees.back().center.y   = tracker->blobs[j].centroid.y * scaley;
	 
	 // init the tree pos
	 trees.back().treeBaseD   = trees.back().center;
	 trees.back().treeBaseD.y = trees.back().rect.y + trees.back().rect.height + panel.getValueF("TREE_OFF");
	 trees.back().treeBase	 = trees.back().treeBaseD;
	 }
	 }
	 
	 
	 }
	 */
	
	// --------------------- Tree People
	for(int i=0; i<trees.size(); i++) {
		trees[i].update();	
	}
	// clean up the trees
	for(int i=trees.size()-1; i>=0; i--) {
		
		if(trees[i].bDead) {
			trees[i].cleanUp();
			trees.erase(trees.begin() + i);
			printf("tree deleted \n");
		}
	}
	
	
	
	
	
	// --------------------- update the trees
	/*for(int i=0; i<tracker->blobs.size(); i++) {
	 
	 int lookID = tracker->blobs[i].id;
	 float barea  = (float)(tracker->blobs[i].boundingRect.height*tracker->blobs[i].boundingRect.width) / (float)(packet.width*packet.height);
	 
	 ofRectangle& newRec = tracker->blobs[i].boundingRect;
	 
	 newRec.x		*= scalex;
	 newRec.y		*= scaley;
	 newRec.width	*= scalex;
	 newRec.height	*= scaley;
	 newRec.height   += panel.getValueF("TREE_OFF");
	 
	 for(int j=0; j<trees.size(); j++) {
	 if(lookID == trees[j].id) {
	 trees[j].frameAge  = 0;// tracker->blobs[i].frameAge;
	 trees[j].rect	   = newRec;
	 trees[j].center.x   = tracker->blobs[i].centroid.x * scalex;
	 trees[j].center.y   = tracker->blobs[i].centroid.y * scaley;
	 }
	 }
	 
	 
	 }
	 */
	
	updateFlocking();
	
	
	// --------------------- packet info
	if(packet.frameNumber >= 0 && !bGotMyFirstPacket) {
		printf("got my first packet - %i\n", packet.frameNumber);
		bGotMyFirstPacket = true;
		
	}
	
}

// ---------------------------------------------------------
void TreeScene::drawTop() {
	if(bDebug) panel.draw();
}

// ---------------------------------------------------------
void TreeScene::draw() {
	
	float scalex =  (float)OFFSCREEN_WIDTH / (float)packet.width;
	float scaley = (float)OFFSCREEN_HEIGHT / (float)packet.height;
	
	if(bDebug) {
		ofSetColor(0, 25, 255);
		ofFill();
		ofRect(-300, -500, 10000, 10000);
	}
	
	ofEnableAlphaBlending();
	glPushMatrix();
	glTranslatef(0, 0, 0);
	
	// the big trees - funky style
	glPushMatrix();
	glTranslatef(0, panel.getValueF("TREE_OFF"), 0);
	for (int i = 0; i < trees.size(); i++){
		trees[i].draw();
	}
	glPopMatrix();
	
	// draw the people
	for(int i=0; i<packet.nBlobs; i++) {
		
		
		if(panel.getValueB("BPEOPLE_GLOW")) {
			// drop shadow
			ofEnableAlphaBlending();
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			for (int j = 0; j < packet.nPts[i]; j+=2) {
				float x = packet.pts[i][j].x * scalex;
				float y = packet.pts[i][j].y * scaley;	
				
				ofSetRectMode(OF_RECTMODE_CENTER);
				ofSetColor(255, 255, 255, panel.getValueF("PEOPLE_GLOW"));
				theDotS.draw(x, y, 55, 55);
				ofSetRectMode(OF_RECTMODE_CORNER);
			}
			glDisable(GL_BLEND);
			ofDisableAlphaBlending();
		}
		
		// people
		ofSetColor(150, 150, 150);
		ofFill();
		ofBeginShape();
		for (int j = 0; j < packet.nPts[i]; j++) {
			
			float x = packet.pts[i][j].x * scalex;
			float y = packet.pts[i][j].y * scaley;
			
			ofVertex(x, y);
		}
		ofEndShape(true);
		
		if(bDebug) {
			ofNoFill();
			ofSetColor(255, 35, 0);
			ofRect(packet.rect[i].x * scalex, 
				   packet.rect[i].y * scaley, 
				   packet.rect[i].width * scalex, 
				   packet.rect[i].height * scaley);
			
			
			ofDrawBitmapString("w:"+ofToString(packet.rect[i].width)+"\n"+
							   "h:"+ofToString(packet.rect[i].height)+"\n", 
							   20+packet.rect[i].x * scalex, 
							   20+packet.rect[i].y * scaley);
		}
		
	}
	
	
	// draw the butter flys
	for(int i = 0; i < butterflys.size(); i++){
		butterflys[i].draw();
	}
	
	
	if(bDebug) {	
		// The Tree Blobs
		for(int i=0; i<treeBlobs.size(); i++) {
			glPushMatrix();
			glTranslatef(treeBlobs[i].center.x, treeBlobs[i].center.y, 0);
			ofFill();
			ofSetColor(200, 40, 255);
			ofCircle(0, 0, 40);
			ofSetColor(255, 255, 255);
			ofDrawBitmapString("id: "+ofToString(treeBlobs[i].id) +"\n"+
							   ofToString(treeBlobs[i].age), 0, 0);
			glPopMatrix();
		}
	}
	
	
	glPopMatrix();
	
	if(bDebug) {
		float screenW = 0;
		float screenH = 0;
		ofNoFill();
		ofSetColor(245, 2, 2);
		ofRect(((OFFSCREEN_WIDTH - screenW)/2), (OFFSCREEN_HEIGHT-screenH), screenW, screenH);
	}
}







// --------------------------------------------------------- blob events
void TreeScene::blobOn( int x, int y, int bid, int order ) {
	
	ofCvTrackedBlob * blober = &tracker->getById(bid);
	
	// if i am hole make a shape instead
	if(blober->hole) {
		
	}
	
	// else make a Tree
	else {
		
		//	if(blober->boundingRect.width >= panel.getValueI("TREE_GROW_W") || 
		//	   blober->boundingRect.height >= panel.getValueI("TREE_GROW_H")) {
		
		
		
		
		treeBlobs.push_back(TreeBlob());
		treeBlobs.back().id = bid;
		treeBlobs.back().age = 0;
		treeBlobs.back().bAlive = false;
		
		treeBlobs.back().initTime = ofGetElapsedTimef();
		
		printf("new tree blob  - %i\n", treeBlobs.back().id);
		//	}
		
		//if(blober.boundingRect.width >= minSpawnToGrowW || blober.boundingRect.height >= minSpawnToGrowH) {
		//	treePeople.push_back(TreePerson());
		//	treePeople.back().init( tracker->getById(bid) );
		//	treePeople.back().tree.img = &theDot;
		//}
		/*
		 printf("-- id:%i, frameAge:%i\n", bid, tracker->getById(bid).frameAge);
		 if(blober->frameAge > 50) {
		 trees.push_back(MagicTree());
		 
		 float tx = x * TREE_SCALE;
		 float ty = y * TREE_SCALE;
		 trees.back().initTree(tx, ty, 10);
		 trees.back().img = &theDot;
		 trees.back().id = bid;
		 trees.back().frameAge = blober->frameAge;
		 
		 printf("new tree  - %i\n", bid);
		 
		 }
		 */
	}
	
	
	
}

// ---------------------------------------------------------
void TreeScene::blobMoved( int x, int y, int bid, int order ) {
	
}

// ---------------------------------------------------------
void TreeScene::blobOff( int x, int y, int bid, int order ) {	
	
	for(int i=trees.size()-1; i>=0; i--) {
		if(trees[i].id == bid) {
			
			trees[i].id = -1;
			trees[i].bNoBlobAnymore = true;
			printf("tree off - %i\n", bid);
			
		}
	}
	
	// clean up the tree blobs
	for(int i=treeBlobs.size()-1; i>=0; i--) {
		if(treeBlobs[i].id == bid) {
			
			treeBlobs.erase(treeBlobs.begin() + i);
			printf("--- tree blob removed [%i] -- \n", bid);
		}
	}
}
















