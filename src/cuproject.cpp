/*

	CU-Logic 

	Written by: Jonathan Clevenger

	Started: 10/16/2022

*/

#include "cuheads.h"


CU::Project::Project(){

};

void CU::Project::create(std::string name){
	// Set the rate to a default
	simulationSpeed = 250;
};


int CU::Project::addElement(CU::Element &element){
	elementList.emplace_back(element);
	return elementList.size()-1;
};

void CU::Project::connectElements(int f, int fpin, int t, int tpin){
	if(f >= 0 && f < elementList.size() && 
		t >= 0 && t < elementList.size()){
		CU::ElementPin *ipin = elementList[t].inputPin(tpin);
		if(ipin != nullptr){
			CU::ElementPin *opin = elementList[f].outputPin(fpin);
			if(opin != nullptr){
				ipin->from = opin;
				return;
			}
		}
	}
	CU::debugWrite(std::string("Bad pin id:")+std::to_string(fpin)+","+std::to_string(tpin), CU::DebugMsgType::ERROR);
};

void CU::Project::probeElement(int e, int opin, std::string pn){
	if(e>=0 && e<elementList.size()){
		CU::Portal p;
		p.SET(elementList[e].outputPin(opin), pn);
		toPins.push_back(p);
	}
};


void CU::Project::advance(){

	OsciliTick otick;

	// Run through all gates and calculate the outputs
	for(int i = 0; i < elementList.size(); i++){
		elementList[i].run();
	}

	for( int i = 0; i < toPins.size(); i++){
		toPins[i].UPDATE();

		otick.variables.push_back(toPins[i].uuid);
		otick.variableData.push_back(toPins[i].pin.SELF());
	}
	
	osciliMemory.push_back(otick);

	if(osciliMemory.size() > CU::OscilloscopeMaxTicks){
		// Remove the first tick
		osciliMemory.erase(osciliMemory.begin(),osciliMemory.begin()+1);
	}

	// Advance the tick
	currentTick += 1;

	if(simulationSpeed){
		simulationSpeed -= 1;
	}
};

void CU::Project::printOscilloscope(CU::Driver &videoDriver){
	int ox = 0;
	int oy = 6;
	int viewH = 3;

	for(int tick = 0; tick < osciliMemory.size(); tick ++){	
		for(int v = 0; v < osciliMemory.at(tick).variables.size(); v++){
			CU::LOGIC ltick = CU::LOGIC::UNKNOWN;
			if(tick > 0){ ltick = osciliMemory[tick-1].variableData[v]; }	
			CU::LOGIC ctick = osciliMemory[tick].variableData[v];

			std::string varName = osciliMemory.at(tick).variables.at(v);

			videoDriver.drawBox(ox,oy+(v*(viewH+1)),CU::OscilloscopeMaxTicks+11,viewH+2,CU::BlockType::DOUBLE, CU::Color::WHITE, CU::Color::BLACK);
			videoDriver.drawVLine(ox+9,oy+(v*(viewH+1))+1,viewH,CU::BlockType::DOUBLE, CU::Color::WHITE, CU::Color::BLACK);
			std::string vname = CU::trimString(varName,8);
			videoDriver.writeStr(vname, ox+5-(vname.length()/2), oy+(v*(viewH+1))+1+(viewH>>1));

			// Draw the wave form
			CU::BlockChar outChar = CU::BlockChar::XCROSS; // Unknown State
			int waveData = 0;
			int osciDY = 0;
			if(ltick == CU::LOGIC::UNKNOWN ||
				ctick == CU::LOGIC::UNKNOWN){
				// Write an X
				outChar = CU::BlockChar::XCROSS;
				waveData = -1;
				osciDY = 0;
			}else if(ltick == CU::LOGIC::LOW && ctick == CU::LOGIC::HIGH){
				// Draw a rising edge
				waveData = 1;
			}else if(ltick == CU::LOGIC::HIGH && ctick == CU::LOGIC::LOW){
				// Draw a falling edge
				waveData = 2;
			}else if(ctick == CU::LOGIC::LOW){
				outChar = CU::BlockChar::HBAR;
				waveData = 0;
				osciDY = 1;
			}else if(ctick == CU::LOGIC::HIGH){
				outChar = CU::BlockChar::HBAR;
				waveData = 0;
				osciDY = -1;
			}
			int osX = ox+9+CU::OscilloscopeMaxTicks-osciliMemory.size()+tick;
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

