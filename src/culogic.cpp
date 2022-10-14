/*

	CU-Logic - A text based Logic Simulator and Solver

	Written by: Jonathan Clevenger

	Started: 10/14/2022

*/

#include "cuheads.h"

CUEditor editor;

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
	while(running){
		CU::Mouse_t termMouse = videoDriver.getMouse();

		drawGUI();

		if(shuttingDown){
			// Close all the files
			if(fileList.size()){
				if(closeCurrentFile() == false){
					shuttingDown = false;
				}
			}else{
				running = false;
			}
		}else{

			CU::keyCode key = videoDriver.getkey();

			if(key == CU::keyCode::k_escape){
				MainMenuTabsSelected = !MainMenuTabsSelected;
				if(MainMenuTabsSelected){
					mainMenu.selectTab(0);
				}else{
					mainMenu.closeTab(mainMenu.getTab());
					mainMenu.selectTab(-1);
				}
			}

			if(MainMenuTabsSelected){
				if(key == CU::keyCode::s_left){
					mainMenu.closeTab(mainMenu.getTab());
					int cur = mainMenu.getTab() - 1;
					if(cur < 0) { cur = 0; }
					mainMenu.selectTab(cur);
				}
				if(key == CU::keyCode::s_right){
					mainMenu.closeTab(mainMenu.getTab());
					int cur = mainMenu.getTab() + 1;
					if(cur >= mainMenu.numTabs()) { cur = mainMenu.numTabs()-1; }
					mainMenu.selectTab(cur);
				}
				if(key == CU::keyCode::k_tab){
					mainMenu.closeTab(mainMenu.getTab());
					int cur = mainMenu.getTab() + 1;
					if(cur >= mainMenu.numTabs()) { cur = 0; }
					mainMenu.selectTab(cur);
				}
				if(key == CU::keyCode::k_enter){
					if(mainMenu.tabOpen(mainMenu.getTab())){
						// Run the sub function for that menu
						mainMenu.runSubMenu(mainMenu.getTab(), mainMenu.curSubMenu(mainMenu.getTab()));
						// Close the tab
						mainMenu.closeTab(mainMenu.getTab());
						// We don't want to still be on the main menu tabs
						MainMenuTabsSelected = false;
						mainMenu.selectTab(-1);
					}else{
						mainMenu.openTab(mainMenu.getTab());
					}
				}
				if(key == CU::keyCode::s_up){
					if(mainMenu.tabOpen(mainMenu.getTab())){
						mainMenu.selectMenu(mainMenu.getTab(), mainMenu.curSubMenu(mainMenu.getTab())-1);
					}
				}
				if(key == CU::keyCode::s_down){
					if(mainMenu.tabOpen(mainMenu.getTab())){
						mainMenu.selectMenu(mainMenu.getTab(), mainMenu.curSubMenu(mainMenu.getTab())+1);
					}
				}
			}else{
				if(EditorSelected){
					doEditor(key);
				}else if(TerminalSelected){
					// TODO:
					// Add terminal
				}
			}
		}

		// Make the cursor blink every half a second
		using namespace std::chrono;
		uint64_t vtime_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		if(vtime_now >= cursorTime){
			cursorTime = vtime_now + 500;
			cursorBlink = !cursorBlink; 
		}

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

void CULogic::drawGUI(){
	// Clear the screen
	videoDriver.clear();
	videoDriver.drawBar(0,0,videoDriver.getWidth(), videoDriver.getHeight(), ' ', settings.foreground_color, settings.background_color);
	// Draw the header
	videoDriver.drawBar(0,0,videoDriver.getWidth(), 1, ' ', settings.head_fg_color, settings.head_bg_color);

	// Write the IDE name
	videoDriver.writeStr("CU Edit IDE",0,0);

	// Write the current time
	std::time_t t = std::time(0);
    std::tm *tstruct = localtime(&t);
    	
	int hour = tstruct->tm_hour;
	int minute = tstruct->tm_min;
	int second = tstruct->tm_sec;

	std::string timestr = "";
	
	timestr += CU::to_string(hour,2) + ':';
	timestr += CU::to_string(minute,2) +':';
	timestr += CU::to_string(second,2);
	
	// Center the time
	videoDriver.writeStr(timestr,(videoDriver.getWidth()>>1) - (timestr.length() >> 1) ,0);

	// Write the current date
	
	int month = tstruct->tm_mon+1;
	int day = tstruct->tm_mday;
	int year = tstruct->tm_year+1900;

	std::string datestr = "";
	
	datestr += CU::to_string(month,2) + '/';
	datestr += CU::to_string(day,2) +'/';
	datestr += CU::to_string(year,4);
	
	// Center the time
	videoDriver.writeStr(datestr, (videoDriver.getWidth() - datestr.length()) ,0);

	// Draw the editor
	if(EditorOpen){
		drawEditor();
	}

	// Draw the menus
	mainMenu.draw(videoDriver,0);

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
		videoDriver.drawBar(menuX,menuY,menuWidth,menuHeight, ' ', settings.sub_menu_fg_color, settings.sub_menu_bg_color);
		// Draw a feild
		videoDriver.drawBox(menuX,menuY,menuWidth,menuHeight, CU::BlockType::SINGLE, settings.sub_menu_fg_color, settings.sub_menu_bg_color);
		// Draw a title bar
		videoDriver.drawBar(menuX,menuY,menuWidth, 1, ' ', settings.menu_bar_fg_color, settings.menu_bar_bg_color);
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
		videoDriver.drawBar(menuX,menuY,menuWidth,menuHeight, ' ', settings.sub_menu_fg_color, settings.sub_menu_bg_color);
		// Draw a feild
		videoDriver.drawBox(menuX,menuY,menuWidth,menuHeight, CU::BlockType::SINGLE, settings.sub_menu_fg_color, settings.sub_menu_bg_color);
		// Draw a title bar
		videoDriver.drawBar(menuX,menuY,menuWidth, 1, ' ', settings.menu_bar_fg_color, settings.menu_bar_bg_color);
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
		videoDriver.drawBar(menuX,menuY,menuWidth,menuHeight, ' ', settings.sub_menu_fg_color, settings.sub_menu_bg_color);
		// Draw a feild
		videoDriver.drawBox(menuX,menuY,menuWidth,menuHeight, CU::BlockType::SINGLE, settings.sub_menu_fg_color, settings.sub_menu_bg_color);
		// Draw a title bar
		videoDriver.drawBar(menuX,menuY,menuWidth, 1, ' ', settings.menu_bar_fg_color, settings.menu_bar_bg_color);
		// Draw a text feild
		videoDriver.drawBox(menuX+2,menuY+3,menuWidth-4,3, CU::BlockType::DOUBLE, settings.sub_menu_fg_color, settings.sub_menu_bg_color);

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
	switch((CUBreakType)videoDriver.halted()){
		case CUBreakType::SAVE_EXIT:
			// Save the project and exit
			shutdown();
		break;
		case CUBreakType::COPY:
			// TODO:
			// Copy selected text to clipboard
			break;
		case CUBreakType::UNDO:
			// TODO:
			// Undo changes until the buffer is empty
			break;
	}
	
};

