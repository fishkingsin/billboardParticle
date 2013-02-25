#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
#ifdef USE_SYPHON
	server.setName("billboardParticle");
//	fbo.allocate(1024,768);
#endif
    ofSetBackgroundAuto(false);
    ofSetFrameRate(60);
//	ofSetLogLevel(OF_LOG_VERBOSE);
	ofBackground(0, 0, 0);
	
	cameraRotation.set(0);
	zoom = -500;
	zoomTarget = 200;
	
	billboards.getVertices().resize(NUM_BILLBOARDS);
	billboards.getColors().resize(NUM_BILLBOARDS);
	billboards.getNormals().resize(NUM_BILLBOARDS,ofVec3f(0));
	
	// ------------------------- billboard particles
	for (int i=0; i<NUM_BILLBOARDS; i++) {
		
        billboardVels[i].set(ofRandomf()*10*sinf((i*1.0f/NUM_BILLBOARDS)*TWO_PI), ofRandomf()*10*cos((i*1.0f/NUM_BILLBOARDS)*TWO_PI), 0);
        billboards.getVertices()[i].set(0,0,0);
        age[i]= 0;//ofRandom(0.5,1.0);
		billboards.getColors()[i].set(ofColor::fromHsb(i%255, 255, 255));
	    billboardSizeTarget[i] = ofRandom(4, 64);
		billboardNormal[i] = ofVec3f(100,0,0);
	}
	
	
	billboards.setUsage( GL_DYNAMIC_DRAW );
	billboards.setMode(OF_PRIMITIVE_POINTS);
	//billboardVbo.setVertexData(billboardVerts, NUM_BILLBOARDS, GL_DYNAMIC_DRAW);
	//billboardVbo.setColorData(billboardColor, NUM_BILLBOARDS, GL_DYNAMIC_DRAW);
	
	// load the bilboard shader
	// this is used to change the
	// size of the particle
	billboardShader.load("Billboard");
	
	// we need to disable ARB textures in order to use normalized texcoords
	ofDisableArbTex();
	texture.loadImage("dot.png");
	ofEnableAlphaBlending();
    past = ofGetElapsedTimef();
    //    fbo.allocate(ofGetWidth(), ofGetHeight() , GL_RGBA);
    emitter.set(0, 0);
    emitterTex.loadImage("emitter.png");
    
    gui = new ofxUICanvas(0, 0, ofGetWidth()*0.5, ofGetHeight());
    
    float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float length = (ofGetWidth()*0.25)-xInit;
    int dim = 32;
    gui->addLabel("ParticleRecoder", OFX_UI_FONT_LARGE);
    gui->setDrawPaddingOutline(true);
    gui->setDrawBack(true);
    gui->setColorBack(ofColor(125,125,125,125));
    gui->addFPS();
	
    gui->addSlider("NOISE_STRENGTH", 0.0, 1.0f, noiseStrength, length-xInit, dim);
    gui->addSlider("NOISE_POWER", 0.0, 1.0f, noisePower, length-xInit, dim);
    gui->addSlider("AGING", 0.0, 1.0f ,  aging, length-xInit, dim);
    gui->addSlider("COLOR_CHANGE", 0.0, 2000.0f ,  colorChange, length-xInit, dim);
    gui->addSlider("EMITTER_NOISE_STRENGTH", 0.0, ofGetWidth() ,  ofGetWidth()*0.5, length-xInit, dim);
    
    gui->addRangeSlider("PARTICLE_SIZE", 0.0, 255.0, 50.0, 100.0, length-xInit,dim);
	gui->addSlider("BG_COLOR_RED", 0.0f, 255.0 ,  0.0f, length-xInit, dim);
	gui->addSlider("BG_COLOR_BLUE", 0.0f, 255.0 ,  0.0f, length-xInit, dim);
	gui->addSlider("BG_COLOR_GREEN", 0.0f, 255.0 ,  0.0f, length-xInit, dim);
	gui->addSlider("BG_COLOR_ALPHA", 0.0f, 255.0 ,  0.0f, length-xInit, dim);
    gui->addToggle( "AUTO_EMITT", false, dim, dim);
    gui->addToggle( "RECORD", false, dim, dim);
	
	post.init(ofGetWidth(), ofGetHeight());
    post.createPass<FxaaPass>()->setEnabled(false);
    post.createPass<BloomPass>()->setEnabled(false);
    post.createPass<DofPass>()->setEnabled(false);
	post.createPass<KaleidoscopePass>()->setEnabled(false);
    post.createPass<NoiseWarpPass>()->setEnabled(false);
    post.createPass<PixelatePass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
	ofxUILabel * lable = new ofxUILabel("SHADER",OFX_UI_FONT_LARGE);
//	gui->addWidgetEastOf (new ofxUISpacer(ofGetWidth()*0.25,0,ofGetWidth()*0.25,100,"SPACER"), "NOISE_STRENGTH");
	gui->addWidgetEastOf (lable,"NOISE_STRENGTH");
	for (unsigned i = 0; i < post.size(); ++i)
    {
//        gui->addWidgetDown();
		gui->addWidgetPosition(new ofxUILabelToggle(post[i]->getName(), false, length-xInit),
						  OFX_UI_WIDGET_POSITION_DOWN,
						  OFX_UI_ALIGN_RIGHT,
							   false);
	}
    ofAddListener(gui->newGUIEvent,this,&testApp::guiEvent);
    gui->loadSettings("GUI/guiSettings.xml");
	
	duration.setup(12345);
	ofAddListener(duration.events.trackUpdated, this, &testApp::trackUpdated);
	

}
//--------------------------------------------------------------
//Or wait to receive messages, sent only when the track changed
void testApp::trackUpdated(ofxDurationEventArgs& args){
	ofLogVerbose("Duration Event") << "track type " << args.track->type << " updated with name " << args.track->name << " and value " << args.track->value << endl;
	if(args.track->name=="/NOISE_STRENGTH")
    {
        noiseStrength = args.track->value;
		((ofxUISlider*)gui->getWidget(args.track->name.substr(1,string::npos)))->setValue(args.track->value);
    }
    else if(args.track->name=="/NOISE_POWER")
    {
        noisePower = args.track->value;
		((ofxUISlider*)gui->getWidget(args.track->name.substr(1,string::npos)))->setValue(args.track->value);
    }
    else if(args.track->name=="/AGING")
    {
        aging = args.track->value;
		((ofxUISlider*)gui->getWidget(args.track->name.substr(1,string::npos)))->setValue(args.track->value);
    }
    else if(args.track->name=="/AUTO_EMITT")
    {
        autoEmitt = args.track->on;
		((ofxUIToggle*)gui->getWidget(args.track->name.substr(1,string::npos)))->setValue(args.track->on);
    }
    else if(args.track->name=="/RECORD")	
    {
        bRecord = args.track->on;
		((ofxUIToggle*)gui->getWidget(args.track->name.substr(1,string::npos)))->setValue(args.track->on);
    }
    
    else if(args.track->name=="/COLOR_CHANGE")
    {
        colorChange = args.track->value;
		((ofxUISlider*)gui->getWidget(args.track->name.substr(1,string::npos)))->setValue(args.track->value);
    }
    
    else if(args.track->name=="/PARTICLE_SIZE_MAX")
    {
		
        particleSizeMax = args.track->value;
		ofxUIRangeSlider* slider = (ofxUIRangeSlider*)gui->getWidget("PARTICLE_SIZE");
		if(slider)slider->setValueHigh(args.track->value);
		
	}
	else if(args.track->name=="/PARTICLE_SIZE_MIN")
	{
        particleSizeMin = args.track->value;
		ofxUIRangeSlider* slider = (ofxUIRangeSlider*)gui->getWidget("PARTICLE_SIZE");
		if(slider)slider->setValueLow(args.track->value);
		
    }
    else if(args.track->name=="/EMITTER_NOISE_STRENGTH")
    {
        noiseEmitterStrength = args.track->value;
		((ofxUISlider*)gui->getWidget(args.track->name.substr(1,string::npos)))->setValue(args.track->value);
    }
	else if(args.track->name=="/BG_COLOR")
	{
		bgColor = args.track->color;
		((ofxUISlider*)gui->getWidget("BG_COLOR_RED"))->setValue(bgColor.r);
		((ofxUISlider*)gui->getWidget("BG_COLOR_GREEN"))->setValue(bgColor.g);
		((ofxUISlider*)gui->getWidget("BG_COLOR_BLUE"))->setValue(bgColor.b);
		((ofxUISlider*)gui->getWidget("BG_COLOR_ALPHA"))->setValue(bgColor.a);
		
	}
	else if(args.track->name=="/BG_COLOR_ALPHA")
	{
		bgColor.a = args.track->value;
		((ofxUISlider*)gui->getWidget("BG_COLOR_ALPHA"))->setValue(bgColor.a);
	}
	
	
}
void testApp::exit()
{
    gui->saveSettings("GUI/guiSettings.xml");
    delete gui;
    
}
//--------------------------------------------------------------
void testApp::guiEvent(ofxUIEventArgs &e)
{
    if(e.widget->getName()=="NOISE_STRENGTH")
    {
        noiseStrength = ((ofxUISlider*)e.widget)->getScaledValue();
    }
    else if(e.widget->getName()=="NOISE_POWER")
    {
        noisePower = ((ofxUISlider*)e.widget)->getScaledValue();
    }
    else if(e.widget->getName()=="AGING")
    {
        aging = ((ofxUISlider*)e.widget)->getScaledValue();
    }
    else if(e.widget->getName()=="AUTO_EMITT")
    {
        autoEmitt = ((ofxUIToggle*)e.widget)->getValue();
    }
    else if(e.widget->getName()=="RECORD")
    {
        bRecord = ((ofxUIToggle*)e.widget)->getValue();
    }
    
    else if(e.widget->getName()=="COLOR_CHANGE")
    {
        colorChange = ((ofxUISlider*)e.widget)->getScaledValue();
    }
    
    else if(e.widget->getName()=="PARTICLE_SIZE")
    {
        particleSizeMax = ((ofxUIRangeSlider*)e.widget)->getScaledValueHigh();
        particleSizeMin = ((ofxUIRangeSlider*)e.widget)->getScaledValueLow();
    }
    else if(e.widget->getName()=="EMITTER_NOISE_STRENGTH")
    {
        noiseEmitterStrength = ((ofxUISlider*)e.widget)->getScaledValue();

    }
	else if(e.widget->getName()=="BG_COLOR_RED")
	{
		bgColor.r = ((ofxUISlider*)e.widget)->getScaledValue();

	}
	else if(e.widget->getName()=="BG_COLOR_BLUE")
	{
		bgColor.b = ((ofxUISlider*)e.widget)->getScaledValue();
		
	}
	else if(e.widget->getName()=="BG_COLOR_GREEN")
	{
		bgColor.g = ((ofxUISlider*)e.widget)->getScaledValue();
		
	}
	else if(e.widget->getName()=="BG_COLOR_ALPHA")
	{
		bgColor.a = ((ofxUISlider*)e.widget)->getScaledValue();
		
	}
	else {
		for (unsigned i = 0; i < post.size(); ++i)
		{
    
			if(post[i]->getName()==e.widget->getName())
			{
				post[i]->setEnabled(((ofxUIToggle*)e.widget)->getValue());
			}
		}
    }
    
    
}
//--------------------------------------------------------------
void testApp::update() {
	
	float t = (ofGetElapsedTimef()) * noisePower;
	float div = 250.0;
	float cur = ofGetElapsedTimef();
    ofVec3f v (ofSignedNoise(t, emitter.y/ofGetHeight()*0.5)*noiseEmitterStrength,ofSignedNoise(emitter.x/ofGetWidth()*0.5, t)*noiseEmitterStrength);
//	ofVec3f v(ofSignedNoise(t,ofGetWidth()*0.5), ofSignedNoise(ofGetHeight()*0.5, t));

    emitter+=(v-emitter)*0.01;
    if(emitter.x < -(particleSizeMax*0.5)-ofGetWidth()*0.5)
    {
//        emitter-=v*2;
        emitter.x = ofGetWidth()*0.5;
        
    }else if(emitter.y < -(particleSizeMax*0.5)-ofGetHeight()*0.5)
    {
//        emitter-=v*2;
        emitter.y = ofGetHeight()*0.5;
        
    }else if(emitter.x > -(particleSizeMax*0.5)+ofGetWidth()*0.5)
    {
//        emitter-=v*2;
        emitter.x = -ofGetWidth()*0.5;
        
    }else if(emitter.y > -(particleSizeMax*0.5)+ofGetHeight()*0.5)
    {
//        emitter-=v*2;
        emitter.y = -ofGetHeight()*0.5;
    }
    
	for (int i=0; i<billboards.getNumVertices(); i++) {
		if(age[i]>0)
        {
            // noise
            ofVec3f vec(
                        ofSignedNoise(t, billboards.getVertex(i).y/div, billboards.getVertex(i).z/div)*noiseStrength,
                        ofSignedNoise(billboards.getVertex(i).x/div, t, billboards.getVertex(i).z/div)*noiseStrength,
                        0);//ofSignedNoise(billboards.getVertex(i).x/div, billboards.getVertex(i).y/div, t)*noiseStrength);
            
            vec *= 10 * ofGetLastFrameTime();
            billboardVels[i] += vec;
            billboards.getVertices()[i] += billboardVels[i];
            billboardVels[i] *= 0.99f;
//			billboards.setNormal(i,billboards.getNormal(i)*age[i]);//+ billboardSizeTarget[i] * ofNoise(t+i),0,0));
			billboards.setNormal(i,billboardNormal[i]*age[i]);
            billboards.getColors()[i].set(ofColor(billboards.getColors()[i],age[i]*255));
            
            age[i]-=(cur-past)*aging ;
            
        }
        else
        {
            if(autoEmitt)
            {
                billboardVels[i].set(ofRandom(emitter.x-pEmitter.x), ofRandom(emitter.y-pEmitter.y) , ofRandom(-10,10) );
                billboards.getVertices()[i].set(emitter.x+particleSizeMax*0.5,emitter.y+particleSizeMax*0.5,0);
                age[i]= ofRandom(0.1,1.0);
                billboards.getColors()[i].set(ofColor::fromHsb(int(billboards.getColors()[i].getHue()+ofGetElapsedTimef()*colorChange)%255, 255, 255,age[i]*255));
				billboardNormal[i]= ofVec3f(ofRandom(particleSizeMin, particleSizeMax));
				billboards.setNormal(i,billboardNormal[i]);

            }
        }
    }
    pEmitter = emitter;
    past = cur;
    
}

//--------------------------------------------------------------
void testApp::draw() {
//	fbo.begin();
	ofClear(0);
    ofBackground(bgColor);
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    //	ofBackgroundGradient(ofColor(255), ofColor(230, 240, 255));
    //	ofBackground(0);
    //	string info = ofToString(ofGetFrameRate(), 2)+"\n";
    //	info += "Particle Count: "+ofToString(NUM_BILLBOARDS);
    //	ofDrawBitmapStringHighlight(info, 30, 30);
    
    ofSetColor(255);
    
	
	post.begin(cam);
//    ofPushMatrix();
//    ofTranslate(ofGetWidth()/2, ofGetHeight()/2, 0);
//    ofRotate(cameraRotation.x, 1, 0, 0);
//    ofRotate(cameraRotation.y, 0, 1, 0);
//    ofRotate(cameraRotation.y, 0, 0, 1);
    
    // bind the shader so that wee can change the
    // size of the points via the vert shader
    billboardShader.begin();
    
    ofEnablePointSprites();
    texture.getTextureReference().bind();
    billboards.draw();
    texture.getTextureReference().unbind();
    ofDisablePointSprites();
    
    billboardShader.end();
        emitterTex.draw(emitter,particleSizeMax,particleSizeMax);
//    ofPopMatrix();
	post.end();
//	fbo.end();
//	fbo.draw(0, 0);
    if(bRecord)
        server.publishTexture(&post.getProcessedTextureReference());
    //    gui->draw();
	if(gui->isVisible()	)duration.draw(ofGetWidth()*0.5, 0, ofGetWidth()*0.25,ofGetHeight());
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    if(key == 'f') ofToggleFullscreen();
    if(key == OF_KEY_UP) zoomTarget +=10;
    if(key == OF_KEY_DOWN) zoomTarget -=10;
    if(key == '\t')
	{
		gui->toggleVisible();

	}
    if(key == 'a')autoEmitt = !autoEmitt;
	unsigned idx = key - '0';
    if (idx < post.size()) post[idx]->setEnabled(!post[idx]->getEnabled());
}


//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	if(autoEmitt)
		return;
    int eachTime = ofRandom(50,100);
    int particlesCount=0;
    for (int i=0; i<billboards.getNumVertices(); i++)
    {
        
        if(age[i]<=0)
        {
            
            billboardVels[i].set(ofRandom(x-pX), ofRandom(y-pY) , ofRandom(-10,10) );
            billboards.getVertices()[i].set(mouseX-ofGetWidth()*0.5,mouseY-ofGetHeight()*0.5,0);
            billboards.getColors()[i].set(ofColor::fromHsb(int(billboards.getColors()[i].getHue()+ofGetElapsedTimef()*colorChange)%255, 255, 255,age            [i]*255));
            age[i]= ofRandom(0.1,1.0);
            billboardNormal[i]= ofVec3f(ofRandom(particleSizeMin, particleSizeMax));
			billboards.setNormal(i,billboardNormal[i]);
            particlesCount++;
            if(particlesCount>eachTime)
            {
                break;
            }
            
        }
    }
    pX = x;
    pY = y;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}
