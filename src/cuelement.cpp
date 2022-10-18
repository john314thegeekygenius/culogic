/*

	CU-Logic 

	Written by: Jonathan Clevenger

	Started: 10/16/2022

*/

#include "cuheads.h"

CU::Element::Element(){
    // Do nothing
};

CU::Element::~Element(){
    // Tell all the other elements we are dead
    for(ElementPin &d : inputs){
        if(d.from!=nullptr)
            d.from->to = nullptr; // :p
    }
    for(ElementPin &d : outputs){
        if(d.to!=nullptr)
            d.to->from = nullptr; // :p
    }
};

void CU::Element::init(CU::ElementFunction func, ElementSymbol symb, int numIn, int numOut){
    inputs.clear();
    outputs.clear();

    numInputs = numIn;
    numOutputs = numOut;

    inputs.resize(numInputs);
    outputs.resize(numOutputs);

    setFunction(func);
    setSymbol(symb);
};

CU::Element::Element(const CU::Element& e){
    for(const ElementPin &d : e.inputs)
        inputs.push_back(d);
    for(const ElementPin &d : e.outputs)
        outputs.push_back(d);

    numInputs = e.numInputs;
    numOutputs = e.numOutputs;

    setFunction(e.callback);
//    setSymbol(e.);
};


void CU::Element::setFunction(CU::ElementFunction func){
    callback = func;    
};

void CU::Element::setSymbol(ElementSymbol symb){
    symbol = symb;
};

void CU::Element::setOutput(CU::ElementPin *pin, int outPin){
    if(outPin >= 0 && outPin < outputs.size())
        outputs[outPin].to = pin;
};

void CU::Element::setInput(CU::ElementPin *pin, int inPin){
    if(inPin >= 0 && inPin < inputs.size())
        inputs[inPin].from = pin;
};

int CU::Element::getInputs(){
    return inputs.size();    
};

int CU::Element::getOutputs(){
    return outputs.size();
};

void CU::Element::run(){
    if(callback != nullptr){
        for(auto &i : inputs){
            if(i.from != nullptr)
                i.SET(i.from->SELF());
        }
        callback(inputs, outputs);
        for(auto &o : outputs){
            if(o.to != nullptr)
                o.to->SET(o.SELF());
        }
    }else{
        // Uh oh!
        for(auto &o : outputs){
            o.data = CU::LOGIC::UNKNOWN;
            o.from = nullptr;
            o.to = nullptr;
        }
    }
};

CU::ElementPin *CU::Element::outputPin(int index){
    if(index >= 0 && index < outputs.size())
        return &outputs[index];
    return nullptr; // Yikes!
};

//CU::Component


// Define default logic gates

void CULF_NOT(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs){
    if(inputs.size() && outputs.size())
        outputs.back().SET( inputs.back().NOT() );
};

void CULF_AND(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs){
    if(inputs.size() && outputs.size()){
        for(CU::ElementPin &i : inputs){
            if( i.data == CU::LOGIC::LOW) {
                outputs.back().SET( CU::LOGIC::LOW );
                return;
            }
        }
        outputs.back().SET( CU::LOGIC::HIGH );
    }
};

void CULF_OR(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs){
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

void CULF_XOR(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs){
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

void CULF_SCLOCK(std::vector<CU::ElementPin> &inputs, std::vector<CU::ElementPin> &outputs){
    // Synced with the computer clock and the simulation speed
    //CU::getCurrentSimulationTick()
    //CU::getCurrentSimulationSpeed
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
CU::Element CUE_ClockSynced;
//CU::Element CUE_ClockASynced;

CU::ElementPin CUP_CONST_LOW;
CU::ElementPin CUP_CONST_HIGH;


void CU::GenerateElements(){
    CUE_NotGate.init(&CULF_NOT, CU_SYM_NotGate, 1, 1);
    CUE_AndGate.init(&CULF_AND, CU_SYM_Default, 2, 1);
    CUE_OrGate.init(&CULF_OR, CU_SYM_Default, 2, 1);
    CUE_XOrGate.init(&CULF_XOR, CU_SYM_Default, 2, 1);
    CUE_ClockSynced.init(&CULF_SCLOCK, CU_SYM_Default, 0, 1);
//  CUE_ClockASynced(&CULF_ASCLOCK, CU_SYM_Default, 0, 1);

	CUP_CONST_LOW.data = CU::LOGIC::LOW;
	CUP_CONST_LOW.from = nullptr;
	CUP_CONST_LOW.to = nullptr;
	CUP_CONST_HIGH.data = CU::LOGIC::HIGH;
	CUP_CONST_HIGH.from = nullptr;
	CUP_CONST_HIGH.to = nullptr;

};
