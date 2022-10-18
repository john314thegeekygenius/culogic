/*

	CU-Logic - A text based Logic Simulator and Solver

	Written by: Jonathan Clevenger

	Started: 10/14/2022

*/

#include "cuheads.h"

CULogic editor;


int CU::getCurrentSimulationTick(){
	return editor.getSimuTick();
};

int CU::getCurrentSimulationSpeed(){
	return editor.getSimuRate();
};


void breakHandle(int e){
	editor.halt(e);
//	CU::debugWrite("int "+std::to_string(e));
};

int main(int argc, char *argv[]){
	CU::openDebugFile();
	editor.init(argc, argv);

	editor.run();
	editor.shutdown();
	CU::closeDebugFile();
	return 0;
};


void CULogic::init(int argc, char *argv[]){
	running = true;

	// Setup the break handler
	videoDriver.setupHandle(&breakHandle);
	videoDriver.setFPS(30);

	shuttingDown = false;

	CU::GenerateElements();

	project.create("Untitled");
	project.addElement(CUE_NotGate);
	project.addElement(CUE_ClockSynced);

	//clipboard.clear();

	//loadSettings();

	// Open any files in the arguments
	/*
	for(int i = 1; i < argc; i++){
		std::string path = editor.loadFile(argv[i], false);
		if(path.length()){
			// We want to edit the file now
			editor.EditorSelected = true;
			editor.EditorOpen = true;
		}else{
			// Open a new file
			editor.createFile(argv[i]);
			// We want to edit the file now
			editor.EditorSelected = true;
			editor.EditorOpen = true;
		}
	}*/	
};

void CULogic::close(){
	shuttingDown = true;
};

void CULogic::run(){
	int cx = 0;
	int cy = 0;
	uint64_t nextTime = 0;
	while(running){
		CU::Mouse_t termMouse = videoDriver.getMouse();

		drawGUI();

		if(CU::getMillis() >= nextTime){
			nextTime = CU::getMillis() + project.getSimuSpeed();
			project.advance();
		}

		if(shuttingDown){
			running = false;
		}else{

			CU::keyCode key = videoDriver.getkey();

			if(key == CU::keyCode::k_escape){
				close();
			}
		}

		// Make the cursor blink every half a second
		/*
		using namespace std::chrono;
		uint64_t vtime_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		if(vtime_now >= cursorTime){
			cursorTime = vtime_now + 500;
			cursorBlink = !cursorBlink; 
		}*/

		videoDriver.clearHalt();

		videoDriver.flush();
		//videoDriver.updateDriver();

	}

};

void CULogic::shutdown(){
	
};

void CULogic::halt(int e){
	videoDriver.halt(e);
};

int CULogic::getSimuTick(){
	return project.getTick();
};

int CULogic::getSimuRate(){
	return project.getSimuSpeed();
};

void CULogic::drawGUI(){
	// Clear the screen
	videoDriver.clear();
	videoDriver.drawBar(0,0,videoDriver.getWidth(), videoDriver.getHeight(), ' ', CU::Color::BLACK, CU::Color::L_BLACK);
	// Draw the header
	videoDriver.drawBar(0,0,videoDriver.getWidth(), 1, ' ', CU::Color::BLUE, CU::Color::GREEN);

	// Write the header
	videoDriver.writeStr("CU Logic",0,0);

	project.printOscilloscope(videoDriver);

};


void CULogic::handleArgs(int argc, char *argv[]){
	for(int i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"/no-color")==0){
			// Force no color support
		}
		if(std::strcmp(argv[i],"/color")==0){
			// Force color support

		}
	}
};


void CULogic::ErrorMsgBox(std::string error){
	int menuWidth = 32;
	int menuHeight = 8;
	int menuX = (videoDriver.getWidth()>>1) - (menuWidth>>1);
	int menuY = (videoDriver.getHeight()>>1) - (menuHeight>>1)+1;

	bool userAck = false;
	while(!userAck){

		CU::keyCode key = videoDriver.getkey();

		if(key == CU::keyCode::k_escape || key == CU::keyCode::k_return){
			userAck = true;
		}

		// Draw a window

		// Draw the background
		videoDriver.drawBar(menuX,menuY,menuWidth,menuHeight, ' ', CU::Color::CYAN, CU::Color::L_WHITE);
		// Draw a feild
		videoDriver.drawBox(menuX,menuY,menuWidth,menuHeight, CU::BlockType::SINGLE, CU::Color::CYAN, CU::Color::L_WHITE);
		// Draw a title bar
		videoDriver.drawBar(menuX,menuY,menuWidth, 1, ' ', CU::Color::CYAN, CU::Color::L_WHITE);
		videoDriver.writeStr("Error!", menuX + (menuWidth>>1)-5, menuY);

		videoDriver.writeStr(error, menuX + (menuWidth>>1)-(error.length()>>1), menuY+(menuHeight>>1));

		videoDriver.clearHalt();

		videoDriver.flush();
		//videoDriver.updateDriver();
	}
};

bool CULogic::AreYouSure(std::string warning){
	int menuWidth = 32;
	int menuHeight = 8;
	int menuX = (videoDriver.getWidth()>>1) - (menuWidth>>1);
	int menuY = (videoDriver.getHeight()>>1) - (menuHeight>>1)+1;

	bool returnValue = false;

	bool userAck = false;

	bool yesNoSelect = false;
	while(!userAck){

		CU::keyCode key = videoDriver.getkey();

		if(key == CU::keyCode::k_escape){
			userAck = true;
			returnValue = false;
		}
		if(key == CU::keyCode::l_y){
			userAck = true;
			returnValue = true;
		}
		if(key == CU::keyCode::l_n){
			userAck = true;
			returnValue = false;
		}
		if(key == CU::keyCode::s_left){
			yesNoSelect = false;
		}
		if(key == CU::keyCode::s_right){
			yesNoSelect = true;
		}
		if(key == CU::keyCode::k_tab){
			yesNoSelect = !yesNoSelect;
		}
		if(key == CU::keyCode::k_enter){
			userAck = true;
			returnValue = yesNoSelect;
		}

		// Draw a window

		// Draw the background
		videoDriver.drawBar(menuX,menuY,menuWidth,menuHeight, ' ', CU::Color::CYAN, CU::Color::L_WHITE);
		// Draw a feild
		videoDriver.drawBox(menuX,menuY,menuWidth,menuHeight, CU::BlockType::SINGLE, CU::Color::CYAN, CU::Color::L_WHITE);
		// Draw a title bar
		videoDriver.drawBar(menuX,menuY,menuWidth, 1, ' ', CU::Color::CYAN, CU::Color::L_WHITE);
		videoDriver.writeStr("Are you sure?", menuX + (menuWidth>>1)-5, menuY);
		std::string wmsg = "Are you sure you want to";
		videoDriver.writeStr(wmsg, menuX + (menuWidth>>1)-(wmsg.length()>>1), menuY+(menuHeight>>1));
		videoDriver.writeStr(warning, menuX + (menuWidth>>1)-(warning.length()>>1), menuY+(menuHeight>>1)+1);

		if(yesNoSelect){
			videoDriver.writeStr(" NO", menuX + 2, menuY+menuHeight-2);
			videoDriver.writeStr("[YES]", menuX + (menuWidth)-6, menuY+menuHeight-2);
		}else{
			videoDriver.writeStr("[NO]", menuX + 2, menuY+menuHeight-2);
			videoDriver.writeStr(" YES", menuX + (menuWidth)-6, menuY+menuHeight-2);
		}
		videoDriver.clearHalt();

		videoDriver.flush();
		//videoDriver.updateDriver();
	}
	return returnValue;
};


std::string CULogic::getUserString(std::string msg,int maxLength){
	int menuWidth = 32;
	int menuHeight = 8;
	int menuX = (videoDriver.getWidth()>>1) - (menuWidth>>1);
	int menuY = (videoDriver.getHeight()>>1) - (menuHeight>>1)+1;

	std::string outputString = "";

	bool userAck = false;

	while(!userAck){

		CU::keyCode key = videoDriver.getkey();

		if(key == CU::keyCode::k_escape){
			userAck = true;
		}
		if(key == CU::keyCode::k_enter){
			userAck = true;
		}
		if( (maxLength>0 && outputString.length() < maxLength) || maxLength == -1){
			// Read characters
			if( ((int)key >= (int)CU::keyCode::k_space) && ((int)key <= (int)CU::keyCode::k_grave) ){
				outputString.push_back((char)key);
			}
			if(key == CU::keyCode::k_backspace){
				outputString.pop_back();
			}
		}

		// Draw a window

		// Draw the background
		videoDriver.drawBar(menuX,menuY,menuWidth,menuHeight, ' ', CU::Color::CYAN, CU::Color::L_WHITE);
		// Draw a feild
		videoDriver.drawBox(menuX,menuY,menuWidth,menuHeight, CU::BlockType::SINGLE, CU::Color::CYAN, CU::Color::L_WHITE);
		// Draw a title bar
		videoDriver.drawBar(menuX,menuY,menuWidth, 1, ' ', CU::Color::CYAN, CU::Color::L_WHITE);
		// Draw a text feild
		videoDriver.drawBox(menuX+2,menuY+3,menuWidth-4,3, CU::BlockType::DOUBLE, CU::Color::CYAN, CU::Color::L_WHITE);

		videoDriver.writeStr("Input Feild", menuX + (menuWidth>>1)-5, menuY);
		videoDriver.writeStr(msg, menuX + (menuWidth>>1)-(msg.length()>>1), menuY+2);

		videoDriver.writeStr(outputString, menuX + 3, menuY+4);

		videoDriver.clearHalt();

		videoDriver.flush();
		//videoDriver.updateDriver();
	}
	return outputString;
};

void CULogic::handleInt(){
//	switch(videoDriver.halted()){
//	}	
};

