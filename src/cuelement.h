/*

	CU Logic

	Written by: Jonathan Clevenger

	10/16/2022
*/

#pragma once


namespace CU {

const int MAX_GATE_PINS = 16;

enum class LOGIC {
    LOW = 0,
    HIGH = 1,
    UNKNOWN = 2,
};

typedef struct ElementPin {
    LOGIC data = LOGIC::LOW; // Could set it to LOGIC::UNKNOWN

    ElementPin *from = nullptr;
    ElementPin *to = nullptr;

    void CLEAR(){
        data = LOGIC::LOW; // ???
        from = nullptr;
        to = nullptr;
    };

    void SET(LOGIC d){
        data = d;
    };

    void UPDATE(LOGIC d){
        if(to != nullptr){
            to->SET(d);
        }
    };
    LOGIC NOT(){
        if(data == LOGIC::LOW)
            return LOGIC::HIGH;
        if(data == LOGIC::HIGH)
            return LOGIC::LOW;
        return LOGIC::UNKNOWN;
    };
    LOGIC SELF(){
        return data;
    };
}ElementPin;

struct Element;

typedef void (*ElementFunction)(std::vector<ElementPin> &inputs, std::vector<ElementPin> &outputs, int var1);

typedef struct ElementSymbol {
    std::vector<int> data;
}ElementSymbol;

typedef struct Element {
    int numInputs = 0;
    int numOutputs = 0;
    int var1 = 0;
    ElementSymbol symbol;
    std::vector<ElementPin> inputs;
    std::vector<ElementPin> outputs;
    ElementFunction callback = nullptr;

    void init(CU::ElementFunction func, ElementSymbol symb, int numIn, int numOut){
        inputs.clear();
        outputs.clear();

        numInputs = numIn;
        numOutputs = numOut;

        var1 = 0;

        inputs.resize(numInputs);
        for(ElementPin &p : inputs) p.CLEAR();
        outputs.resize(numOutputs);
        for(ElementPin &p : outputs) p.CLEAR();

        CU::debugWrite(std::string("Initzied Element:") +
                std::to_string(inputs.size()) +
                " vs. " + std::to_string(numInputs) +
                std::to_string(outputs.size()) +
                " vs. " + std::to_string(numOutputs)
                );

        setFunction(func);
        setSymbol(symb);
    };

    void setFunction(CU::ElementFunction func){
        callback = func;    
    };

    void setSymbol(ElementSymbol symb){
        symbol = symb;
    };

    void setOutput(CU::ElementPin *pin, int outPin){
        if(outPin >= 0 && outPin < outputs.size()){
            outputs[outPin].to = pin;
            //CU::debugWrite(CU::addr_string(pin));
//            pin->from = &outputs[outPin];
        }
    };

    void setInput(CU::ElementPin *pin, int inPin){
        if(inPin >= 0 && inPin < inputs.size()){
            inputs[inPin].from = pin;
            //pin->to = &inputs[inPin];
        }
    };

    int getInputs(){
        return numInputs; //inputs.size();
    };

    int getOutputs(){
        return numOutputs; //outputs.size();
    };

    void run(){
        if(callback != nullptr){
            for(CU::ElementPin &i : inputs){
                if(i.from != nullptr)
                    i.SET(i.from->SELF());
            }
            callback(inputs, outputs, var1);
            /*
            for(CU::ElementPin &o : outputs){
                if(o.to != nullptr){
                    o.to->SET(o.SELF());
                }
            }*/
        }else{
            // Uh oh!
            for(auto &o : outputs){
                o.SET(CU::LOGIC::UNKNOWN);
                if(o.to!=nullptr){
                    o.to->SET(o.SELF());
                }
            }
        }
    };

    ElementPin * outputPin(int index){
        if(index >= 0 && index < outputs.size()){
            return &outputs[index];
        }
        return nullptr; // Yikes!
    };

    ElementPin * inputPin(int index){
        CU::debugWrite(std::string("Inputs:")+std::to_string(inputs.size()));
        if(index >= 0 && index < inputs.size()){
            return &inputs[index];
        }
        return nullptr; // Yikes!
    };
    

}Element;


typedef struct Portal {
    ElementPin pin;
    std::string uuid;

    void SET(ElementPin *p, std::string n){
        pin.from = p;
        uuid = n;
    };
    void UPDATE(){
        if(pin.from != nullptr){
            pin.SET(pin.from->SELF());
        }
    };

}Portal;


void TickInternalClock();

Element MakeClock(int rate);

void GenerateElements();


};


// Default logic gates
extern CU::Element CUE_NotGate;
extern CU::Element CUE_AndGate;
extern CU::Element CUE_OrGate;
extern CU::Element CUE_XOrGate;
