/*

	CU-Logic 

	Written by: Jonathan Clevenger

	Started: 10/16/2022

*/

#include "cuheads.h"


//CU::Component


// Define default logic gates

void CULF_NOT(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs, int var1){
    if(inputs.size() && outputs.size()){
        outputs.back().SET( inputs.back().NOT() );
    }
};

void CULF_AND(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs, int var1){
    if(inputs.size() && outputs.size()){
        for(CU::ElementPin &i : inputs){
            if( i.SELF() == CU::LOGIC::LOW) {
                outputs.back().SET( CU::LOGIC::LOW );
                return;
            }
        }
        outputs.back().SET( CU::LOGIC::HIGH );
    }
};

void CULF_OR(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs, int var1){
    if(inputs.size() && outputs.size()){
        for(CU::ElementPin &i : inputs){
            if( i.data == CU::LOGIC::HIGH) {
                outputs.back().SET( CU::LOGIC::HIGH );
                return;
            }
        }
        outputs.back().SET( CU::LOGIC::LOW );
    }
};

void CULF_XOR(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs, int var1){
    if(inputs.size() && outputs.size()){
        int ic = 0;
        for(CU::ElementPin &i : inputs){
            if( i.data == CU::LOGIC::HIGH) 
                ic += 1;
        }
        if(ic != inputs.size())
            outputs.back().SET( CU::LOGIC::HIGH );
        else
            outputs.back().SET( CU::LOGIC::LOW );
    }
};

std::vector<uint64_t> CU_ClockRate;
std::vector<uint64_t> CU_NextClockTick;
std::vector<bool> CU_ClockTicked;

void CULF_SCLOCK(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs, int var1){
    // Synced with the computer clock and the simulation speed
    if(outputs.size()){
        if(CU_ClockTicked[var1] == true){
            // Flip the current state
            outputs.back().SET(outputs.back().NOT());
        }
    }
};

/*void CULF_ASCLOCK(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs){
    // Uhhh
};*/

CU::ElementSymbol CU_SYM_Default = {
    { 3, 3, 
        '+', '-', '+',
        '|', ' ', '|',
        '+', '-', '+',
    },
};

CU::ElementSymbol CU_SYM_NotGate = {
    { 5, 3, 
        '|', '`', '.',   0,  0,
        '|', ' ', ' ', '>','o',
        '|', '.', '\'',  0,  0
    },
};

CU::Element CUE_NotGate;
CU::Element CUE_AndGate;
CU::Element CUE_OrGate;
CU::Element CUE_XOrGate;
//CU::Element CUE_ClockASynced;

CU::ElementPin CUP_CONST_LOW;
CU::ElementPin CUP_CONST_HIGH;


void CU::GenerateElements(){

    CUE_NotGate.init(&CULF_NOT, CU_SYM_NotGate, 1, 1);
    CUE_AndGate.init(&CULF_AND, CU_SYM_Default, 2, 1);
    CUE_OrGate.init(&CULF_OR, CU_SYM_Default, 2, 1);
    CUE_XOrGate.init(&CULF_XOR, CU_SYM_Default, 2, 1);
    
    CUP_CONST_LOW.CLEAR();
    CUP_CONST_LOW.SET(CU::LOGIC::LOW);

    CUP_CONST_HIGH.CLEAR();
    CUP_CONST_HIGH.SET(CU::LOGIC::HIGH);

};

void CU::TickInternalClock(){
    for(int  i =0; i < CU_NextClockTick.size(); i++){
        if(CU_ClockTicked[i]) CU_ClockTicked[i] = false;
        if(CU::getCurrentSimulationTick() >= CU_NextClockTick[i]){
            CU_NextClockTick[i] = CU::getCurrentSimulationTick() + CU_ClockRate[i];
            CU_ClockTicked[i] = true;
        }
    }
};

CU::Element CU::MakeClock(int rate){
    CU::Element tmpe;

    tmpe.init(&CULF_SCLOCK, CU_SYM_Default, 0, 1);

    if(CU_NextClockTick.size())
        CU_NextClockTick.push_back(CU_NextClockTick.front());
    else
        CU_NextClockTick.push_back(0);
    
    CU_ClockTicked.push_back(false);
    CU_ClockRate.push_back(rate);

    tmpe.var1 = CU_NextClockTick.size()-1;

    return tmpe;
};
