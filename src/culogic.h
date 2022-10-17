/*

	CU Logic

	Written by: Jonathan Clevenger

	10/14/2022
*/
#pragma once

namespace CU {

};

class CULogic {
private:
	CU::Driver videoDriver;
	//CUSettings settings;
	bool running = false;
	bool shuttingDown = false;
	CU::Project project;
	
public:
	void init(int argc, char *argv[]);
	void shutdown();
//	void loadSettings();
	void halt(int e);
	void run();
	void close();
	void handleInt();
	void drawGUI();
	void fixCursor();

	void handleArgs(int argc, char *argv[]);

	void ErrorMsgBox(std::string error);
	bool AreYouSure(std::string warning);
	std::string getUserString(std::string msg, int maxLength = -1);

};


