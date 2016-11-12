#include "ofApp.h"

#define FOCUS_Y (144)
#define CROPIMG_Y (FOCUS_Y + 256)

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetWindowShape(800, 600);
	ofBackground(54, 54, 54, 255);

	ofTrueTypeFont::setGlobalDpi(72);
	font.load("VeraSeBd.ttf", 24, true, true);

	offset = ofPoint(0, 0);
	ditherType = ATKINSON;


	imageGui.setup("Image","settings-image.xml");

	imageGui.setPosition(560, 8);
	
	openFile.addListener(this, &ofApp::openFileClicked);
	imageGui.add(openFile.setup("Open File..."));
	
	imgContrast.addListener(this, &ofApp::imgContrastChanged);
	imgBrightness.addListener(this, &ofApp::imgBrightnessChanged);
	imageGui.add(imgContrast.set("Contrast", 1.0, 0.5, 2.0));
	imageGui.add(imgBrightness.set("Brightness", 0, -128, 128));
	
	scale.addListener(this, &ofApp::scaleChanged);
	imageGui.add(scale.set("Scale", 1.0, 0.125, 2));


	focusGui.setup("Crop image","settings-focus.xml");
	focusGui.setPosition(560, FOCUS_Y + 72);

	focusWidth.addListener(this, &ofApp::focusSizeChanged);
	focusHeight.addListener(this, &ofApp::focusSizeChanged);
	focusGui.add(focusWidth.set("Width", 128, 8, 128));
	focusGui.add(focusHeight.set("Height", 64, 8, 64));

	ditherAtkinson.addListener(this, &ofApp::ditherAtkinsonChanged);
	ditherFloyd.addListener(this, &ofApp::ditherFloydChanged);
	ditherBayer.addListener(this, &ofApp::ditherBayerChanged);
	ditherSelect.setName("Dithering Method");
	ditherSelect.add(ditherBayer.set("(1)Bayer", true));
	ditherSelect.add(ditherFloyd.set("(2)Floyd-Steinberg", true));
	ditherSelect.add(ditherAtkinson.set("(3)Atkinson", true));

	focusGui.add(ditherSelect);


	captureGui.setup("Copy as...", "settings-capture.xml");
	captureGui.setPosition(560, CROPIMG_Y + 72);

	copyCcode.addListener(this, &ofApp::copyCcodeClicked);
	copyAbShell.addListener(this, &ofApp::copyAbShellClicked);
	captureGui.add(copyCcode.setup("a C array"));
	captureGui.add(copyAbShell.setup("AbShell commands"));
}
void ofApp::openFileClicked()
{
	ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a jpg or png or bmp");
	if (openFileResult.bSuccess) {
		processOpenFileSelection(openFileResult);
	}
}
//--------------------------------------------------------------
void ofApp::imgBrightnessChanged(int & v)
{
	updateImages();
}

void ofApp::imgContrastChanged(float & v)
{
	updateImages();
}

//--------------------------------------------------------------
void ofApp::focusSizeChanged(int &v) {
	int height = (focusHeight + 4) / 8;
	focusHeight = height * 8;
	focus.setSize(focusWidth, focusHeight);
}

void ofApp::scaleChanged(float &v) {
	updateImages();
}

void ofApp::setDitherType(int d)
{
	ditherAtkinson.setWithoutEventNotifications(d == ATKINSON);
	ditherBayer.setWithoutEventNotifications(d == ORDERED_BAYER);
	ditherFloyd.setWithoutEventNotifications(d == FLOYD_STEINBERG);
	ditherType = d;
	processDither();
}

void ofApp::ditherAtkinsonChanged(bool & v)
{
	setDitherType(ATKINSON);
}

void ofApp::ditherBayerChanged(bool &v) {
	setDitherType(ORDERED_BAYER);
}

void ofApp::ditherFloydChanged(bool &v) {
	setDitherType(FLOYD_STEINBERG);
}

//--------------------------------------------------------------
void ofApp::update(){

}
//--------------------------------------------------------------
void ofApp::draw(){
	ofSetColor(80, 80, 80);
	ofRect(0, 0, 512, 600);
	if (img.isAllocated()) {
		ofRectangle bounds = ofRectangle(0, 0, img.getWidth(), img.getHeight());
		ofRectangle sub = ofRectangle(0, 0, 512, 600);
		sub.translate(offset);
		sub = sub.getIntersection(bounds);

		ofSetColor(255,255,255);
		img.drawSubsection(sub.x - offset.x, sub.y - offset.y, sub.getWidth(), sub.getHeight(), sub.x, sub.y);
		if (bw_img.isAllocated()) {
			ofRectangle bounds = ofRectangle(0, 0, bw_img.getWidth(), bw_img.getHeight());
			ofRectangle sub = ofRectangle(0, 0, 512, 600);
			sub.translate(offset);
			sub = sub.getIntersection(focus + offset);
			sub = sub.getIntersection(bounds);
			bw_img.drawSubsection(sub.x - offset.x, sub.y - offset.y, sub.getWidth(), sub.getHeight(), sub.x, sub.y);
			bw_img.drawSubsection(592, FOCUS_Y, sub.width, sub.height, sub.x, sub.y);
		}
	}
	else {
		ofSetColor(144, 144, 144);
		font.drawString("Hit SPACE to select an image", 40, 200);
	}
	if (capture.isAllocated()) {
		capture.draw(592, CROPIMG_Y);
	}
	imageGui.draw();
	focusGui.draw();
	captureGui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key)
	{
	case ' ':
		openFileClicked();
		break;
	case '1':
		ditherBayer = true;
		break;
	case '2':
		ditherFloyd = true;
		break;
	case '3':
		ditherAtkinson = true;
		break;
	case '-':
	{
		float s = round((scale + 0.0624) / 0.125) * 0.125;
		scale = (s > 0.25) ? (s - 0.125) : 0.125;
		updateImages();
	}
		break;
	case '+':
	{
		float s = round((scale + 0.0624) / 0.125) * 0.125;
		scale = (s < 2) ? (s + 0.125) : 2;
		updateImages();
	}
		break;
	case OF_KEY_RIGHT:
		offset.x = offset.x + 1;
		break;
	case OF_KEY_LEFT:
		offset.x = offset.x - 1;
		break;
	case OF_KEY_DOWN:
		offset.y = offset.y + 1;
		break;
	case OF_KEY_UP:
		offset.y = offset.y - 1;
		break;
	case OF_KEY_RETURN:
		processCapture();
		break;
	}
	if (img.isAllocated())
		processDither();
}

//--------------------------------------------------------------
void ofApp::updateImages() {
	if (original.isAllocated()) {
		img.clone(original);
	}
	if (img.isAllocated()) {
		img.resize(original.getWidth() * scale, original.getHeight() * scale);
		ofPixels p = img.getPixels();
		int b = 128 + imgBrightness;
		int s = img.getWidth() * img.getHeight() * 3;
		for (int i = 0; i < s; i++) {
			int c = p[i] - 128;
			c = imgContrast * c + b;
			c = c < 0 ? 0 : c;
			p[i] = c > 255 ? 255 : c;
		}
		img.setFromPixels(p);

		grey_img.clone(img);
		grey_img.setImageType(OF_IMAGE_GRAYSCALE);
		processDither();
	}
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::processOpenFileSelection(ofFileDialogResult openFileResult) {

	ofFile file(openFileResult.getPath());

	if (file.exists()) {
		string fileExtension = ofToUpper(file.getExtension());
		if (fileExtension == "JPG" || fileExtension == "PNG" || fileExtension == "BMP") {
			original.load(openFileResult.getPath());
			imgContrast = 1.0;
			imgBrightness = 0;
			scale = 1.0;
			offset.set(0, 0);
			updateImages();
		}
	}
}

//--------------------------------------------------------------
void ofApp::processDither() {
	if (grey_img.isAllocated()) {
		switch (ditherType)
		{
		case ORDERED_BAYER:

			dither.dither_ordered(grey_img, bw_img, DITHER_ORDERED_8x8);
			break;

		case FLOYD_STEINBERG:

			dither.dither_floyd_steinberg(grey_img, bw_img);
			break;

		case ATKINSON:

			dither.dither_atkinson(grey_img, bw_img);
			break;
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	focus.setPosition(x-focus.width / 2, y-focus.height / 2);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	if ((x < 512) && (y < 600)) {
		offset = clickPos - ofPoint(x, y) + offset;
		clickPos.set(x, y);
		focus.setPosition(x - focus.width / 2, y - focus.height / 2);
	}
}

//--------------------------------------------------------------

void ofApp::processCapture() {
	if (bw_img.isAllocated()) {
		capture.clear();
		ofRectangle bounds = ofRectangle(0, 0, bw_img.getWidth(), bw_img.getHeight());
		ofRectangle sub = ofRectangle(0, 0, 512, 600);
		sub.translate(offset);
		sub = sub.getIntersection(focus + offset);
		sub = sub.getIntersection(bounds);
		capture.cropFrom(bw_img, sub.x, sub.y, sub.width, sub.height);
	}
}

//--------------------------------------------------------------

void ofApp::mousePressed(int x, int y, int button){
	if ((x < 512) && (y < 600)) {
		switch (button)
		{
		case 0:
			clickPos = ofPoint(x, y);
			processCapture();
			break;
		default:
			break;
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


//--------------------------------------------------------------
void ofApp::copyToClipboard(const string &text) {
	ofGetWindowPtr()->setClipboardString(text);
}

void ofApp::copyCcodeClicked()
{
	stringstream ss;
	ss << "{\r\n\t//width = " << capture.getWidth() << ", height = " << capture.getHeight() << "\r\n";
	int ymax = (capture.getHeight() + 7) / 8;
	int linelen = 0;
	for (int y = 0; y < ymax; y++) {
		for (int x = 0; x < capture.getWidth(); x++) {
			int v = 0;
			for (int b = 0; b < 8; b++) {
				if (y * 8 + b >= capture.getHeight())
					break;
				ofColor color = capture.getColor(x, y * 8 + b);
				v |= (color.getHex() == 0 ? 0 : 1) * (1 << b);
			}
			if (linelen == 0)
				ss << "\t";
			ss	<< "0x"
				<< std::hex
				<< std::setw(2)
				<< std::setfill('0')
				<< v << ", ";
			linelen += 1;
			if (linelen > 15) {
				linelen = 0;
				ss << "\r\n";
			}
		}
	}
	if (linelen != 0)
		ss << "\r\n";
	ss << "};\r\n";
	copyToClipboard(ss.str());
}

void ofApp::copyAbShellClicked()
{
	stringstream ss;
	ss << "clear\r\nbitmap 0 0 " << capture.getWidth() << " " << capture.getHeight() << "\r\n";
	int ymax = (capture.getHeight() + 7) / 8;
	int linelen = 0;
	for (int y = 0; y < ymax; y++) {
		for (int x = 0; x < capture.getWidth(); x++) {
			int v = 0;
			for (int b = 0; b < 8; b++) {
				if (y * 8 + b >= capture.getHeight())
					break;
				ofColor color = capture.getColor(x, y * 8 + b);
				v |= (color.getHex() == 0 ? 0 : 1) * (1 << b);
			}
			if (linelen == 0)
				ss << ".x ";
			ss	<< std::setw(2)
				<< std::setfill('0')
				<< std::hex
				<< v;
			linelen += 2;
			if (linelen > 62) {
				linelen = 0;
				ss << "\r\n";
			}
		}
	}
	if (linelen != 0)
		ss << "\r\n";
	copyToClipboard(ss.str());
}