/*

	CU Edit Driver

	Written by: Jonathan Clevenger

	10/3/2022

*/


#include "cuheads.h"


CU::Driver::Driver(){
	// Make sure it's a valid terminal for color
	colorSupported = false;

	if(isatty(0)){
		// Yay!
		colorSupported = true;
	}

	// Disable the mouse
	terminalMouse.enabled = false;

	// Disable keyboard delay
	kbNoDelay();

	// Disable echoing
	disableEcho();

	// Update the variables
	updateDriver();

	// Hide the "pysical" cursor
	std::cout << "\x1B[?25l" << std::flush;

	// Bool to check if break happened
	breakCalled = false;

};

CU::Driver::~Driver(){
	// Shutdown the driver
	shutdownDriver();
};

void CU::Driver::setFPS(int fps){
	targetFPS = fps;
};

int CU::Driver::getFPS(){
	return FPS;
};

void CU::Driver::shutdownDriver(){
	// Enable echo
	enableEcho();

	// Reset color stuff
	std::cout << "\033[0m" << std::flush;

	// Clear the screen
	std::cout << "\033[3J\033[2J\033[H"<< std::flush;

	// Show the "pysical" cursor
	std::cout << "\x1B[?25h" << std::flush;

	disableMouse();

	// Reset the handler
    struct sigaction action;
    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, nullptr);

	// Say goodbye
	debugWrite("Closed driver");

};

void CU::Driver::updateDriver(){
	// Get the screen size
	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	scrWidth = w.ws_col;
	scrHeight = w.ws_row;
	
	scrSize = scrWidth*scrHeight;

	// Clear the buffer
	scrBuffer.clear();
	
	// Set the buffer
	scrBuffer.reserve(scrWidth*scrHeight*2);
	
	// Set the cursor to a valid location
	setCurPos(0,0);
};


void CU::Driver::setupHandle(__sighandler_t handle){
	breakHandle = handle;
	// Add handler for break command
	struct sigaction sigIntHandler;
	memset(&sigIntHandler, 0, sizeof(sigIntHandler));
	sigIntHandler.sa_handler = (__sighandler_t)breakHandle;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = SA_RESTART;
	if(sigaction(SIGINT, &sigIntHandler, NULL) || sigaction(SIGTSTP, &sigIntHandler, NULL) || sigaction(SIGQUIT, &sigIntHandler, NULL) || 
		sigaction(SIGTERM, &sigIntHandler, NULL) || sigaction(SIGABRT, &sigIntHandler, NULL)){
		halted();
		debugWrite("Failed to init handler", CU::DebugMsgType::ERROR);
		return;
	}
	debugWrite("Setup Handler");
};


void CU::Driver::halt(int e){
	breakCalled = e;
};

int CU::Driver::halted(){
	return breakCalled;
};

void CU::Driver::clearHalt(){
	breakCalled = 0;
};

int CU::Driver::getWidth(){
	return scrWidth;
};

int CU::Driver::getHeight(){
	return scrHeight;
};

void CU::Driver::enableEcho(){
    struct termios term;
    tcgetattr(fileno(stdin), &term);

    term.c_lflag |= ECHO;
    tcsetattr(fileno(stdin), 0, &term);
};

void CU::Driver::disableEcho(){
    struct termios term;
    tcgetattr(fileno(stdin), &term);

    term.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), 0, &term);
};

void CU::Driver::enableMouse(){
	// Enable mouse tracking
	if(!terminalMouse.enabled){
		std::cout << "\x1B[?1000;1003;1006;1015h" << std::flush;
		terminalMouse.enabled = true;
	}
};

void CU::Driver::disableMouse(){
	// Disable mouse tracking
	if(terminalMouse.enabled){
		std::cout << "\x1B[?1000;1003;1006;1015l" << std::flush;
		terminalMouse.enabled = false;
	}
};

void CU::Driver::enableColor(){
	colorSupported = true;
};

void CU::Driver::disableColor(){
	colorSupported = false;
};

void CU::Driver::setCurPos(int x,int y){
	scrCurPos[0] = x;
	scrCurPos[1] = y;
	if(x < 0 ){
		scrCurPos[0] = 0;
	}
	if(x >= scrWidth ){
		scrCurPos[0] = scrWidth-1;
	}
	if(y < 0 ){
		scrCurPos[1] = 0;
	}
	if(y >= scrHeight ){
		scrCurPos[1] = scrHeight-1;
	}
};

//void CU::Driver::hideCursor(){};

//void CU::Driver::showCursor(){};

void CU::Driver::setBGColor(CU::Color c){
	scrBuffer[scrSize+(scrCurPos[1]*scrWidth) + scrCurPos[0]] &= 0xF0;
	scrBuffer[scrSize+(scrCurPos[1]*scrWidth) + scrCurPos[0]] |= (char)c;
};

void CU::Driver::setFGColor(CU::Color c){
	scrBuffer[scrSize+(scrCurPos[1]*scrWidth) + scrCurPos[0]] &= 0x0F;
	scrBuffer[scrSize+(scrCurPos[1]*scrWidth) + scrCurPos[0]] |= (char)c<<8;
};

void CU::Driver::putChar(int c){
	scrBuffer[(scrCurPos[1]*scrWidth) + scrCurPos[0]] = c;	
};


void CU::Driver::clear(){
	if(scrBuffer.size()){
		std::fill_n(scrBuffer.begin(), scrSize, 0);
		std::fill_n(scrBuffer.begin()+scrSize, scrSize, 0);
	//	std::fill(scrBuffer.begin(), scrBuffer.end(), 0);
	//	std::fill(scrBuffer.begin()+scrSize, scrBuffer.end(), 7);
	}
};

void CU::Driver::flush(){
	// Wait for the FPS to be correct before we draw the screen
	using namespace std::chrono;
	uint64_t vtime_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	if(vtime_now < vtime_last){
		return;
	}

	vtime_last = vtime_now + (1000 / targetFPS);

	if(std::time(0) >= fpsTime){
		fpsTime = std::time(0) + 1;
		FPS = frameCount;
		frameCount = 0;
	}
	frameCount += 1;

	char screen_buffer[scrSize*20];
	int buffer_offset = 0;
	std::string buffer_string;

	// Draw the mouse cursor overtop of everything
	if(terminalMouse.enabled){
		setCurPos(terminalMouse.blockX,terminalMouse.blockY);
		if(terminalMouse.buttonMask != CU::MouseMask::NONE){
			writeBChar(CU::BlockChar::MID_SHADE);
		}else{
			writeBChar(CU::BlockChar::SOLID);
		}
	}


	for(int y = 0; y < scrHeight; y++){
		for(int x = 0; x < scrWidth; x++){
			if(colorSupported){
				int bg = 40+(+scrBuffer[scrSize+(y*scrWidth) + x]&0x0F);
				int fg = 30+(+scrBuffer[scrSize+(y*scrWidth) + x]>>8);
				if(bg > 47){
					bg -= 8; // There is no bright backgrounds?
				}
				buffer_string += "\x1B[";
				buffer_string += std::to_string(bg);
				buffer_string += "m";
				if(fg > 37){
					fg += 52;
				}
				buffer_string += "\x1B[";
				buffer_string += std::to_string(fg);
				buffer_string += "m";
			}

			int C = scrBuffer[(y*scrWidth) + x];
			if(C & 0x8000){
				if((C&0xFF) < UNIBlockCount){
					buffer_string += CU::UNIBlockChars[C&0xFF];
				}else{
					buffer_string += '?';
				}
			}else{
				if(C >= 32 && C <= 126)
					buffer_string += (char)(C?C:' ');
				else
					buffer_string += '?';
			}
		}
	}

	// set the cursor position
	std::fputs("\x1b[3J\033[H",stdout);
	
	// output the buffer to the screen
	std::fputs(buffer_string.c_str(),stdout);
	std::fflush(stdout);
};

void CU::Driver::writeBChar(CU::BlockChar c){
	scrBuffer[(scrCurPos[1]*scrWidth) + scrCurPos[0]] = (int)c;	
};

void CU::Driver::writeBChar(CU::BlockChar c, CU::Color fg, CU::Color bg){
	scrBuffer[(scrCurPos[1]*scrWidth) + scrCurPos[0]] = (int)c;	
	scrBuffer[scrSize+(scrCurPos[1]*scrWidth) + scrCurPos[0]] = (((int)fg)<<8) | (int)bg;
};

void CU::Driver::drawBox(int x,int y,int w,int h,CU::BlockType t, CU::Color fg, CU::Color bg){
	int schar = 0;

	switch(t){
		case CU::BlockType::BLOCK:
			schar = (int)CU::BlockChar::SOLID;
			break;
		case CU::BlockType::DOUBLE:
			schar = (int)CU::BlockChar::DHBAR;
			break;
		case CU::BlockType::TXT:
			schar = (int)'-';
			break;
		case CU::BlockType::SINGLE:
			schar = (int)CU::BlockChar::HBAR;
			break;
	}
	for(int e = x; e < x+w; e++){
		int offset = (y*scrWidth) + e;
		if(offset > 0 && offset < scrSize){
			scrBuffer[offset] = (int)schar;
			scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
		}
		offset = ((y+h-1)*scrWidth) + e;
		if(offset > 0 && offset < scrSize){
			scrBuffer[offset] = (int)schar;
			scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
		}
	}
	switch(t){
		case CU::BlockType::BLOCK:
			schar = (int)CU::BlockChar::SOLID;
			break;
		case CU::BlockType::DOUBLE:
			schar = (int)CU::BlockChar::DVBAR;
			break;
		case CU::BlockType::TXT:
			schar = (int)'|';
			break;
		case CU::BlockType::SINGLE:
			schar = (int)CU::BlockChar::VBAR;
			break;
	}
	for(int i = y; i < y+h; i++){
		int offset = (i*scrWidth) + x;
		if(offset > 0 && offset < scrSize){
			scrBuffer[offset] = (int)schar;
			scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
		}
		offset = (i*scrWidth) + x + w -1;
		if(offset > 0 && offset < scrSize){
			scrBuffer[offset] = (int)schar;
			scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
		}
	}


	switch(t){
		case CU::BlockType::BLOCK:
			schar = (int)CU::BlockChar::SOLID;
			break;
		case CU::BlockType::DOUBLE:
			schar = (int)CU::BlockChar::DTLCORNER;
			break;
		case CU::BlockType::TXT:
			schar = (int)'+';
			break;
		case CU::BlockType::SINGLE:
			schar = (int)CU::BlockChar::TLCORNER;
			break;
	}

	int offset = (y*scrWidth) + x;

	if(offset > 0 && offset < scrSize){
		scrBuffer[offset] = (int)schar;
		scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
	}

	switch(t){
		case CU::BlockType::BLOCK:
			schar = (int)CU::BlockChar::SOLID;
			break;
		case CU::BlockType::DOUBLE:
			schar = (int)CU::BlockChar::DTRCORNER;
			break;
		case CU::BlockType::TXT:
			schar = (int)'+';
			break;
		case CU::BlockType::SINGLE:
			schar = (int)CU::BlockChar::TRCORNER;
			break;
	}

	offset = (y*scrWidth) + x + w-1;
	if(offset > 0 && offset < scrSize){
		scrBuffer[offset] = (int)schar;
		scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
	}

	switch(t){
		case CU::BlockType::BLOCK:
			schar = (int)CU::BlockChar::SOLID;
			break;
		case CU::BlockType::DOUBLE:
			schar = (int)CU::BlockChar::DBLCORNER;
			break;
		case CU::BlockType::TXT:
			schar = (int)'+';
			break;
		case CU::BlockType::SINGLE:
			schar = (int)CU::BlockChar::BLCORNER;
			break;
	}
	offset = (y*scrWidth) + x + ((h-1)*scrWidth);
	if(offset > 0 && offset < scrSize){
		scrBuffer[offset] = (int)schar;
		scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
	}

	switch(t){
		case CU::BlockType::BLOCK:
			schar = (int)CU::BlockChar::SOLID;
			break;
		case CU::BlockType::DOUBLE:
			schar = (int)CU::BlockChar::DBRCORNER;
			break;
		case CU::BlockType::TXT:
			schar = (int)'+';
			break;
		case CU::BlockType::SINGLE:
			schar = (int)CU::BlockChar::BRCORNER;
			break;
	}
	offset = (y*scrWidth) + x + ((h-1)*scrWidth) + w-1;
	if(offset > 0 && offset < scrSize){
		scrBuffer[offset] = (int)schar;
		scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
	}
};

void CU::Driver::drawSubBox(int x,int y,int w,int h,CU::BlockType t, CU::Color fg, CU::Color bg){
	drawBox(x,y,w,h,t,fg,bg);
	int offset = (y*scrWidth) + x;
	if(offset > 0 && offset < scrSize){
		scrBuffer[offset] = (int)CU::BlockChar::LINTERS;
		scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
	}
	offset = (y*scrWidth) + x + w-1;
	if(offset > 0 && offset < scrSize){
		scrBuffer[offset] = (int)CU::BlockChar::RINTERS;
		scrBuffer[scrSize+offset] = (((int)fg)<<8) | (int)bg;
	}
};

void CU::Driver::drawBar(int x,int y,int w,int h, int floodchar, CU::Color fg, CU::Color bg){
	CU::Clamp(x,y,w,h,0,0,scrWidth,scrHeight);
	for(int i = y; i < y+h; i++){
		for(int e = x; e < x+w; e++){
			scrBuffer[(i*scrWidth) + e] = (int)floodchar;	
			scrBuffer[scrSize+(i*scrWidth) + e] = (((int)fg)<<8) | (int)bg;
		}
	}
};

void CU::Driver::drawPattern(int x,int y,int w,int h, std::string floodstring, CU::Color fg, CU::Color bg){
	CU::Clamp(x,y,w,h,0,0,scrWidth,scrHeight);
	int patternMod = 0;
	for(int i = y; i < y+h; i++){
		for(int e = x; e < x+w; e++){
			scrBuffer[(i*scrWidth) + e] = (int)floodstring[patternMod];	
			scrBuffer[scrSize+(i*scrWidth) + e] = (((int)fg)<<8) | (int)bg;
			patternMod += 1;
			patternMod %= floodstring.length();
		}
	}
};

void CU::Driver::writeStr(std::string s, int x,int y){
	if(y < 0) { return; }
	if(y >= scrHeight) { return; }
	y *= scrWidth;
	for(char c : s){
		if(x < 0) { continue; }
		if(c < 32 || c > 126) c = '?';
		scrBuffer[(y) + x] = (int)c;
		x++;
		if(x > scrWidth){
			return;
		}
	}
};

void CU::Driver::writeStr(std::string s, int x,int y, CU::Color fg, CU::Color bg){
	if(y < 0) { return; }
	if(y >= scrHeight) { return; }
	y *= scrWidth;
	for(char c : s){
		if(x < 0) { continue; }
		if(c < 32 || c > 126) c = '?';
		scrBuffer[(y) + x] = (int)c;
		scrBuffer[scrSize+(y) + x] = (((int)fg)<<8) | (int)bg;
		x++;
		if(x > scrWidth){
			return;
		}
	}
};

void CU::Driver::writeStrW(std::string s, int x,int y, int w){
	std::string short_str = "";
	for(int i = 0; i < s.length(); i++){
		if(s.at(i)=='.'){
			break;
		} 
		if(i >= w){
			short_str.push_back('*');
			break;
		}
		short_str.push_back(s.at(i));
	}
	int ext_pos = s.length();
	for(int i = 0; i < s.length(); i++){
		if(s.at(i)=='.'){
			ext_pos = i;
			break;
		}
	}
	for(int i = ext_pos; i < s.length(); i++){
		if(i >= ext_pos+8){
			short_str.push_back('*');
			break;
		}
		short_str.push_back(s.at(i));
	}
	
	writeStr(short_str,x,y);
};

void CU::Driver::writeStrW(std::string s, int x,int y, int w, Color fg, Color bg){
	std::string short_str = "";
	for(int i = 0; i < s.length(); i++){
		if(s.at(i)=='.'){
			break;
		} 
		if(i >= w){
			short_str.push_back('*');
			break;
		}
		short_str.push_back(s.at(i));
	}
	int ext_pos = s.length();
	for(int i = 0; i < s.length(); i++){
		if(s.at(i)=='.'){
			ext_pos = i;
			break;
		}
	}
	for(int i = ext_pos; i < s.length(); i++){
		if(i >= ext_pos+8){
			short_str.push_back('*');
			break;
		}
		short_str.push_back(s.at(i));
	}
	
	writeStr(short_str,x,y,fg,bg);
};


void CU::Driver::kbNoDelay(){
	// From: https://stackoverflow.com/questions/421860/capture-characters-from-standard-input-without-waiting-for-enter-to-be-pressed
	// Use termios to turn off line buffering
	struct termios term;
	tcgetattr(0, &term); // 0 is STDIN
	term.c_lflag &= ~ICANON;
	// TODO:
	// Make this work???
	// https://stackoverflow.com/questions/40574789/how-to-detect-xoff-and-xon-in-linux-terminal-application
	term.c_lflag &= ~IXON;

	tcsetattr(0, TCSANOW, &term); // 0 is STDIN
	setbuf(stdin, NULL);

};

int CU::Driver::kbhit(void) {
    int nbbytes;
    ioctl(0, FIONREAD, &nbbytes);  // 0 is STDIN
	if(nbbytes){
		debugWrite("Raw bytes:"+std::to_string(nbbytes));
	}
    return nbbytes;
};

char CU::Driver::getch() {
	char buf = 0;
	if(ungetchbuffer.size()){
		buf = ungetchbuffer.back();
		ungetchbuffer.pop_back();
	}else{
		read(0, &buf, 1);
		debugWrite("Raw:"+std::to_string(buf));
	}
	return (buf);
};

void CU::Driver::ungetch(char buf) {
	ungetchbuffer.push_back(buf);
};

int CU::Driver::getMValue(){
	char ch = 0;
	int outValue = 0;
	bool loadFlag = false;
	std::string ostr = "";
	for(int i = 0; i < 12; i++){
		ch = getch();
		if(ch == 59){
			loadFlag = !loadFlag;
		}
		if(loadFlag==false || (ch == 77 || ch == 109)) {
			ungetch(ch);
			break; // end of value collection
		}
		if(ch != 59){
			outValue *= 10;
			outValue += (int)ch-48;
		}
	}
	CU::debugWrite(std::to_string(outValue));
	return outValue;
};

CU::keyCode CU::Driver::getkey() {
	int key = (int)CU::keyCode::k_null;
	int keyCount = kbhit();

/*
	https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Mouse-Tracking
	https://stackoverflow.com/questions/5966903/how-to-get-mousemove-and-mouseclick-in-bash
	https://stackoverflow.com/questions/44116977/get-mouse-position-in-pixels-using-escape-sequences

	terminalMouse.blockX = 0;
	terminalMouse.blockY = 0;

	terminalMouse.clickX = 0;
	terminalMouse.clickY = 0;
	terminalMouse.buttonMask = CU::MouseMask::NONE;

	Left click Down: 0;63;7M
	Left click Up  : 0;50;6m

	Middle click Down: 1;50;6M
	Middle click Up  : 1;50;6m

	Right click Down: 2;50;6M
	Right click Up  : 2;50;6m

	Scroll Up   : 64;53;9M
	Scroll Down : 65;53;9M

	MB - Mouse Button
	X = x+49
	Y = y+49
	RT - Mouse Action  77 - Down,  109 - Up
	left:
		  ESC   MB     X    Y   RT
	D: 27 91 60 48 59 49 59 49 77
	R: 27 91 60 48 59 49 59 49 109
	right
	D: 27 91 60 50 59 49 59 49 77

	SCID - scroll identifier

			ESC     SCID    X     Y
Scrl Up  :27 91 60 54 52 59 49 59 49 77 
Scrl Dwn :27 91 60 54 53 59 49 59 49 77 

				   ESC    MMID     X     Y 
	Mouse Move: 27 91 60 51 53 59 49 59 49 77 
*/

	if(keyCount){
		char ch = getch(); 

		// Handle Escape codes (Special keys)
		if(ch == 0x1b){
			if(keyCount>1){
				char specialcheck = getch();
				if(specialcheck == 0x5b){
					if(keyCount>3){
						specialcheck = getch();
						if(specialcheck == 0x3c){
							if(terminalMouse.enabled){
								// Mouse events!
								char idchk[2];
								idchk[0] = getch();
								idchk[1] = getch();

								if(idchk[0] >= 48 && idchk[0] <= 50 && idchk[1]==59) {
									ungetch(idchk[1]);
									// Get the mouse position
									terminalMouse.clickX = getMValue()-1;
									terminalMouse.clickY = getMValue()-1;

									terminalMouse.blockX = terminalMouse.clickX;
									terminalMouse.blockY = terminalMouse.clickY;

									if(idchk[0] == 48)
										terminalMouse.buttonMask = CU::MouseMask::LBUTTON;
									if(idchk[0] == 49)
										terminalMouse.buttonMask = CU::MouseMask::MBUTTON;
									if(idchk[0] == 50)
										terminalMouse.buttonMask = CU::MouseMask::RBUTTON;
									char mskchk = getch();
									if(mskchk == 109){
										int t = (int)terminalMouse.buttonMask;
										t |= (int)CU::MouseMask::RELEASED;
										terminalMouse.buttonMask = (CU::MouseMask)t;
									}
								}
								if(idchk[0]==51) {
									if(idchk[1]==53){
	//									int t = (int)terminalMouse.buttonMask;
	//									t |= (int)CU::MouseMask::DOWN;
									}
									// Get the mouse position
									terminalMouse.blockX = getMValue()-1;
									terminalMouse.blockY = getMValue()-1;
								}
								if(idchk[0]==54) {
									if(idchk[1]==52){
										// Mouse scrolled up
										terminalMouse.scroll = -1;
									}
									if(idchk[1]==53){
										// Mouse scrolled down
										terminalMouse.scroll = 1;
									}
									// Get the mouse position
									terminalMouse.blockX = getMValue()-1;
									terminalMouse.blockY = getMValue()-1;
								}
								// Catch the mouse identifier
								char mouseCatch = getch();
							}
						}else
						if(specialcheck == 0x31){
							specialcheck = getch();
							if(specialcheck == 0x3b){
								// Read the special key
								ch = getch();
								if(ch == 50) {
									key |= (int)CU::keyCode::c_shift;
								}
								if(ch == 51) {
									key |= (int)CU::keyCode::c_alt;
								}
								if(ch == 53) {
									key |= (int)CU::keyCode::c_ctrl;
								}
								keyCount -= 3;
							}
						}
						if(keyCount==4){
							if(specialcheck == 0x33){
								ch = getch();
								if(ch == 126) {
									key |= (int)CU::keyCode::s_delete;
								}
							}
							if(specialcheck == 0x35){
								ch = getch();
								if(ch == 126) {
									key |= (int)CU::keyCode::s_pg_up;
								}
							}
							if(specialcheck == 0x36){
								ch = getch();
								if(ch == 126) {
									key |= (int)CU::keyCode::s_pg_down;
								}
							}
						}
					}
					if(keyCount<=3){
						ch = getch();
						// Set to the correct keycode
						if(ch == 65){
							key |= (int)CU::keyCode::s_up;
						}
						if(ch == 66){
							key |= (int)CU::keyCode::s_down;
						}
						if(ch == 67){
							key |= (int)CU::keyCode::s_right;
						}
						if(ch == 68){
							key |= (int)CU::keyCode::s_left;
						}
						if(ch == 0x48){
							key |= (int)CU::keyCode::s_home;
						}
						if(ch == 0x46){
							key |= (int)CU::keyCode::s_end;
						}
					}
				}
			}else{
				key = (int)CU::keyCode::k_escape;
			}
		}else{
			key = ch;
		}
		//CU::debugWrite("Key pressed "+std::to_string(ch));
	}
	return (CU::keyCode)key;
};

CU::Mouse_t CU::Driver::getMouse(){
	CU::Mouse_t mcpy;
	mcpy.blockX = terminalMouse.blockX;
	mcpy.blockY = terminalMouse.blockY;
	mcpy.clickX = terminalMouse.clickX;
	mcpy.clickY = terminalMouse.clickY;
	mcpy.buttonMask = terminalMouse.buttonMask;
	mcpy.scroll = terminalMouse.scroll;
	mcpy.enabled = terminalMouse.enabled;
	// Clear the mouse
	terminalMouse.buttonMask = CU::MouseMask::NONE;
	terminalMouse.scroll = 0;
	return mcpy;
};

int CU::stoi(std::string in_str){
    int i;
    char *s = (char*)in_str.c_str();
    i = 0;
    while(*s >= '0' && *s <= '9')
    {
        i = i * 10 + (*s - '0');
        s++;
    }
    return i;
};

std::string CU::to_string(int value,int fill){
	std::stringstream ss;
	ss << std::setw(fill) << std::setfill('0') << value;
	return ss.str();
};

std::string CU::to_stringc(int value,char fillc, int fillw){
	std::stringstream ss;
	ss << std::setw(fillw) << std::setfill(fillc) << value;
	return ss.str();
};


void CU::Clamp(int &x, int &y, int &w, int &h, int minx, int miny, int maxw, int maxh){
	if(x < minx) { x = minx; }
	if(y < miny) { y = miny; }
	if(w > maxw) { w = maxw; }
	if(h > maxh) { h = maxh; }
	if(w < 0) { w = 0; }
	if(h < 0) { h = 0; }
};

std::string CU::trimString(std::string s, int w){
	std::string short_str = "";
	int offset = s.length()-(w-1);
	if(offset < 0 ) { offset = 0; }
	for(int i = offset; i < s.length(); i++){
		short_str.push_back(s.at(i));
	}
	return short_str;
};

std::string CU::fileizeString(std::string s, int fnw, int exw){
	std::string short_str = "";
	for(int i = 0; i < s.length(); i++){
		if(s.at(i)=='.'){
			break;
		} 
		if(i >= fnw){
			short_str.push_back('*');
			break;
		}
		short_str.push_back(s.at(i));
	}
	int ext_pos = s.length();
	for(int i = 0; i < s.length(); i++){
		if(s.at(i)=='.'){
			ext_pos = i;
			break;
		}
	}
	for(int i = ext_pos; i < s.length(); i++){
		if(i >= ext_pos+exw){
			short_str.push_back('*');
			break;
		}
		short_str.push_back(s.at(i));
	}
	
	return short_str;
};

std::string CU::filenameString(std::string s){
	std::string short_str = "";
	int fnameBegin = 0;
	for(int i = s.length()-1; i >= 0; i--){
		if(s.at(i)=='/'){
			fnameBegin = i;
			break;
		} 
	}
	for(int i = fnameBegin; i < s.length(); i++){
		if(s.at(i) >= 32 && s.at(i) <= 126){
			short_str.push_back(s.at(i));
		}else{
			short_str.push_back('?');
		}
	}
	return short_str;
};

std::string CU::safeifyString(std::string s){
	std::string short_str = "";
	for(int i = 0; i < s.length(); i++){
		if(s.at(i) >= 32 && s.at(i) <= 126){
			short_str.push_back(s.at(i));
		}else{
			short_str.push_back('?');
		}
	}
	return short_str;
};

std::string CU::extentString(std::string s){
	std::string short_str = "";
	int extpoint = 0;
	for(int i = 0; i < s.length(); i++){
		if(s.at(i)=='.'){
			extpoint = i+1;
			break;
		}
	}
	for(int i = extpoint; i < s.length(); i++){
		short_str.push_back(s.at(i));
	}
	return short_str;
};


std::ofstream debugFile;

void CU::openDebugFile(){
	// Open a debug file
	debugFile.open("dbgout.txt");
	if(!debugFile.is_open()){
		// Uh... Never mind then.
	}
	debugWrite("Opened debug file");	
};

void CU::closeDebugFile(){
	// Close the debug file
	if(debugFile.is_open()){
		debugFile.close();
	}
};

void CU::debugWrite(std::string s, CU::DebugMsgType msgType){
	if(debugFile.is_open()){
		const std::string errorMsgs[3] = { "INFO", "ERROR", "WARN"};
		debugFile << errorMsgs[(int)msgType] << ":" << s << std::endl;
	}
};
