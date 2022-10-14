/*

	CU Edit Driver

	Written by: Jonathan Clevenger

	10/3/2022

*/
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <csignal>
#include <chrono>
#include <filesystem>


namespace CU {

enum class Color{
	BLACK = 0,
	RED = 1,
	GREEN = 2,
	YELLOW = 3,
	BLUE = 4,
	MAGENTA = 5,
	CYAN = 6,
	WHITE = 7,

	L_BLACK = 8,
	L_RED = 9,
	L_GREEN = 10,
	L_YELLOW = 11,
	L_BLUE = 12,
	L_MAGENTA = 13,
	L_CYAN = 14,
	L_WHITE = 15,
};

enum class BlockChar{
	// Single lines
	TLCORNER = 0x8000,
	TRCORNER = 0x8001,
	BLCORNER = 0x8002,
	BRCORNER = 0x8003,
	LINTERS = 0x8004,
	RINTERS = 0x8005,
	TINTERS = 0x8006,
	BINTERS = 0x8007,
	HBAR = 0x8008,
	VBAR = 0x8009,
	// Double lines	
	DTLCORNER = 0x800A,
	DTRCORNER = 0x800B,
	DBLCORNER = 0x800C,
	DBRCORNER = 0x800D,
	DLINTERS = 0x800E,
	DRINTERS = 0x800F,
	DTINTERS = 0x8010,
	DBINTERS = 0x8011,
	DHBAR = 0x8012,
	DVBAR = 0x8013,
	// Block Chars
	SOLID = 0x8014,
	BOT_HALF = 0x8015,
	TOP_HALF = 0x8016,
	LIGHT_SHADE = 0x8017,
	MID_SHADE = 0x8018,
	DARK_SHADE = 0x8019,
};

const int UNIBlockCount = 0x26;

const std::string UNIBlockChars [] = {
	// Single lines
	"\u250F",
	"\u2513",
	"\u2517",
	"\u251B",
	"\u2523",
	"\u252B",
	"\u2533",
	"\u253B",
	"\u2501",
	"\u2503",
	// Double lines	
	"\u2554",
	"\u2557",
	"\u255A",
	"\u255D",
	"\u2560",
	"\u2563",
	"\u2566",
	"\u2569",
	"\u2550",
	"\u2551",
	// Solid Blocks
	"\u2588",
	"\u2584",
	"\u2580",
	"\u2590",
	"\u2591",
	"\u2592",
};

enum class BlockType {
	SINGLE = 0,
	DOUBLE = 1,
	BLOCK = 2,
	TXT = 3,
};

enum class DebugMsgType {
	INFO = 0,
	ERROR = 1,
	WARN = 2,
};

enum class MouseMask {
	NONE = 0x0,
	LBUTTON = 0x1,
	MBUTTON = 0x2,
	RBUTTON = 0x4,
	RELEASED = 0x10,
};

typedef struct Mouse_t {
	int blockX;
	int blockY;

	int clickX;
	int clickY;

	int scroll;
	MouseMask buttonMask;

	bool enabled;
}Mouse_t;

class Driver {
private:
	int scrWidth = 0;
	int scrHeight = 0;
	int scrSize = 0;
	int scrCurPos[2] = {0,0};
	bool colorSupported = false;

	volatile int breakCalled = 0;
	__sighandler_t breakHandle;

	std::vector<short> scrBuffer;

	// Write the current time
	uint64_t vtime_last = 0;

	std::time_t fpsTime;
	int FPS = 0;
	int targetFPS = 60;
	int frameCount = 0;

	Mouse_t terminalMouse;

	std::vector<int> ungetchbuffer;

public:
	Driver();
	~Driver();

	void setFPS(int fps);
	int getFPS();

	void shutdownDriver();
	void updateDriver();

	void setupHandle(__sighandler_t handle);
	void halt(int e);
	int halted();
	void clearHalt();

	int getWidth();
	int getHeight();

	void enableEcho();
	void disableEcho();

	void enableMouse();
	void disableMouse();

	void enableColor();
	void disableColor();

	Mouse_t getMouse();
	int getMValue();

	void setCurPos(int x,int y);
//	void hideCursor();
//	void showCursor();
	void setBGColor(Color c);
	void setFGColor(Color c);
	void putChar(int c);
	void clear();
	void flush();
	void writeBChar(BlockChar c);
	void writeBChar(BlockChar c, Color fg, Color bg);
	void drawBox(int x,int y,int w,int h,BlockType t, Color fg = Color::WHITE, Color bg = Color::BLACK);
	void drawSubBox(int x,int y,int w,int h,BlockType t, Color fg = Color::WHITE, Color bg = Color::BLACK);
	void drawBar(int x,int y,int w,int h, int floodchar = ' ', Color fg = Color::WHITE, Color bg = Color::BLACK);
	void drawPattern(int x,int y,int w,int h, std::string floodstr, Color fg = Color::WHITE, Color bg = Color::BLACK);
	void writeStr(std::string s, int x,int y);
	void writeStr(std::string s, int x,int y, Color fg, Color bg);
	void writeStrW(std::string s, int x,int y, int w);
	void writeStrW(std::string s, int x,int y, int w, Color fg, Color bg);

	// From: https://stackoverflow.com/questions/421860/capture-characters-from-standard-input-without-waiting-for-enter-to-be-pressed
	void kbNoDelay();
	int kbhit();
	char getch();
	void ungetch(char ch);
	keyCode getkey();

};

int stoi(std::string in_str);

std::string to_string(int value,int fill = 1);

std::string to_stringc(int value,char fillc = '0', int fillw = 1);

std::string trimString(std::string s, int w);

std::string fileizeString(std::string s, int fnw, int exw);

std::string filenameString(std::string s);

std::string extentString(std::string s);

std::string safeifyString(std::string s);

void Clamp(int &x, int &y, int &w, int &h, int minx, int miny, int maxw, int maxh);

void debugWrite(std::string s, DebugMsgType msgType = DebugMsgType::INFO);

void openDebugFile();

void closeDebugFile();

};
