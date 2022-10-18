/*

	CU-Logic 

	Written by: Jonathan Clevenger

	Started: 10/16/2022

*/

#include "cuheads.h"


CU::Project::Project(){

};

void CU::Project::create(std::string name){
};

void CU::Project::addElement(CU::Element &element){
	elementList.emplace_back(element);
	CU::Portal p;
	p.pin = elementList.back().outputPin(0);
	p.uuid = "A";
	toPins.emplace_back(p);

	elementList.back().setOutput(toPins.back().pin,0); // Doesn't matter if pin is null

	elementList.back().setInput(&CUP_CONST_LOW,0);
};

void CU::Project::advance(){

	OsciliTick otick;

	// Run through all gates and calculate the outputs
	for(int i = 0; i < elementList.size(); i++){
		Element &e = elementList[i];
		e.run();
	}

	for( CU::Portal tp : toPins){
		if(tp.pin){
			otick.variables.push_back(tp.uuid);
			otick.variableData.push_back(tp.pin->SELF());
		}
	}
	if(osciliMemory.size() >= CU::OscilloscopeMaxTicks){
		// Remove the first tick
		osciliMemory.erase(osciliMemory.begin(),osciliMemory.begin()+1);
	}
	osciliMemory.push_back(otick);

	// Advance the tick
	currentTick += 1;
};

void CU::Project::printOscilloscope(CU::Driver &videoDriver){
	int ox = 0;
	int oy = 6;
	int viewH = 3;

	for(int tick = 0; tick < osciliMemory.size(); tick ++){
		
		OsciliTick &ltick = osciliMemory[tick];
		if(tick > 0){ ltick = osciliMemory[tick-1]; }
		
		OsciliTick & otick = osciliMemory[tick];

		for(int v = 0; v < otick.variables.size(); v++){
			std::string &varName = otick.variables[v];
			CU::LOGIC &varData = otick.variableData[v];
			
			videoDriver.drawBox(ox,oy+(v*(viewH+1)),CU::OscilloscopeMaxTicks+11,viewH+2,CU::BlockType::DOUBLE, CU::Color::WHITE, CU::Color::BLACK);
			videoDriver.drawVLine(ox+9,oy+(v*(viewH+1))+1,viewH,CU::BlockType::DOUBLE, CU::Color::WHITE, CU::Color::BLACK);
			std::string vname = CU::trimString(varName,8);
			videoDriver.writeStr(vname, ox+5-(vname.length()/2), oy+(v*(viewH+1))+1+(viewH>>1));

			// Draw the wave form
			CU::BlockChar outChar = CU::BlockChar::XCROSS; // Unknown State
			int waveData = 0;
			int osciDY = 0;
			if(ltick.variableData[v] == CU::LOGIC::UNKNOWN ||
				otick.variableData[v] == CU::LOGIC::UNKNOWN){
				// Write an X
				outChar = CU::BlockChar::XCROSS;
				waveData = -1;
				osciDY = 0;
			}else if(ltick.variableData[v] == CU::LOGIC::LOW &&
				otick.variableData[v] == CU::LOGIC::HIGH){
				// Draw a rising edge
				waveData = 1;
			}else if(ltick.variableData[v] == CU::LOGIC::HIGH &&
				otick.variableData[v] == CU::LOGIC::LOW){
				// Draw a falling edge
				waveData = 2;
			}else if(otick.variableData[v] == CU::LOGIC::LOW){
				outChar = CU::BlockChar::HBAR;
				waveData = 0;
				osciDY = 1;
			}else if(otick.variableData[v] == CU::LOGIC::HIGH){
				outChar = CU::BlockChar::HBAR;
				waveData = 0;
				osciDY = -1;
			}
			int osX = ox+(CU::OscilloscopeMaxTicks+9)-tick;
			int osY = oy+(v*(viewH+1))+2;
			if(waveData <= 0){
				videoDriver.setCurPos(osX,osY+osciDY);
				videoDriver.setFGColor(CU::Color::BLUE);
				videoDriver.writeBChar(outChar);
			}else if(waveData == 1){
				videoDriver.setCurPos(osX,osY);
				videoDriver.setFGColor(CU::Color::BLUE);
				videoDriver.writeBChar(CU::BlockChar::VBAR);
				videoDriver.setCurPos(osX,osY-1);
				videoDriver.setFGColor(CU::Color::BLUE);
				videoDriver.writeBChar(CU::BlockChar::TLCORNER);
				videoDriver.setCurPos(osX,osY+1);
				videoDriver.setFGColor(CU::Color::BLUE);
				videoDriver.writeBChar(CU::BlockChar::BRCORNER);
			}else if(waveData == 2){
				videoDriver.setCurPos(osX,osY);
				videoDriver.setFGColor(CU::Color::BLUE);
				videoDriver.writeBChar(CU::BlockChar::VBAR);
				videoDriver.setCurPos(osX,osY-1);
				videoDriver.setFGColor(CU::Color::BLUE);
				videoDriver.writeBChar(CU::BlockChar::TRCORNER);
				videoDriver.setCurPos(osX,osY+1);
				videoDriver.setFGColor(CU::Color::BLUE);
				videoDriver.writeBChar(CU::BlockChar::BLCORNER);
			}
			/*
			videoDriver.setCurPos(osX,osY);
			videoDriver.setFGColor(CU::Color::BLUE);
			videoDriver.putChar('0'+((int)varData));
			*/
		}
	}
};

int CU::Project::getTick(){
    return currentTick;
};

int CU::Project::getSimuSpeed() {
	return simulationSpeed;
};

