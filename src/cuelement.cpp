/*

	CU-Logic 

	Written by: Jonathan Clevenger

	Started: 10/16/2022

*/

#include "cuheads.h"


CU::Element::Element(){
    inputs.clear();    
};

CU::Element::Element(CU::ElementFunction func, ElementSymbol symb){
    inputs.clear();
    setFunction(func);
    setSymbol(symb);
};

void CU::Element::setFunction(CU::ElementFunction func){
    result = func;    
};

void CU::Element::setSymbol(ElementSymbol symb){
    symbol = symb;
};


//CU::Component


// Define default logic gates

CU::LOGIC CULF_NOT(std::vector<CU::ElementPin*> &inputs){
    if(inputs.size()){
        return inputs.back()->NOT();
    }
    return CU::LOGIC::HIGH;
};

CU::LOGIC CULF_AND(std::vector<CU::ElementPin*> &inputs){
    for(CU::ElementPin *i : inputs){
        if( i->data == CU::LOGIC::LOW) 
            return CU::LOGIC::LOW;
    }
    return CU::LOGIC::HIGH;
};

CU::LOGIC CULF_OR(std::vector<CU::ElementPin*> &inputs){
    for(CU::ElementPin *i : inputs){
        if( i->data == CU::LOGIC::HIGH) 
            return CU::LOGIC::HIGH;
    }
    return CU::LOGIC::LOW;
};

CU::LOGIC CULF_XOR(std::vector<CU::ElementPin*> &inputs){
    int ic = 0;
    for(CU::ElementPin *i : inputs){
        if( i->data == CU::LOGIC::HIGH) 
            ic += 1;
    }
    if(ic != inputs.size())
        return CU::LOGIC::HIGH;
    return CU::LOGIC::LOW;
};

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

CU::Element CUE_NotGate(&CULF_NOT, CU_SYM_NotGate);
CU::Element CUE_AndGate(&CULF_AND, CU_SYM_Default);
CU::Element CUE_OrGate(&CULF_OR, CU_SYM_Default);
CU::Element CUE_XOrGate(&CULF_XOR, CU_SYM_Default);


