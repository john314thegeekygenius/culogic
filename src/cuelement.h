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
    LOGIC data = LOGIC::UNKNOWN;
    void SET(LOGIC d){
        data = d;
    };
    LOGIC NOT(){
        if(data == LOGIC::LOW)
            return LOGIC::HIGH;
        return LOGIC::LOW;
    };
    LOGIC SELF(){
        return data;
    };
    ElementPin *from = nullptr;
    ElementPin *to = nullptr;
}ElementPin;

typedef void (*ElementFunction)(std::vector<ElementPin> &inputs, std::vector<ElementPin> &outputs);

typedef struct ElementSymbol {
    std::vector<int> data;
}ElementSymbol;

class Element {
private:
    std::vector<ElementPin> inputs;
    std::vector<ElementPin> outputs;

    int numInputs;
    int numOutputs;

    ElementSymbol symbol;
public:

    Element();

    // Copy constructor
    Element(const Element& e);

    ~Element();
    
    void init(ElementFunction func, ElementSymbol symb, int numIn = 0, int numOut = 1);

    ElementFunction callback = nullptr;
    void setFunction(ElementFunction func);
    void run();

    void setOutput(ElementPin *pin, int outPin);
    void setInput(ElementPin *pin, int inPin);

    int getInputs();
    int getOutputs();

    ElementPin *outputPin(int index);
    void setSymbol(ElementSymbol symb);
};

typedef struct Portal {
    ElementPin* pin = nullptr;
    std::string uuid = "";
}Portal;


/*
class Component {
private:
    std::vector<Element> ElementList;
public:


};
*/

void GenerateElements();

};


// Default logic gates
extern CU::Element CUE_NotGate;
extern CU::Element CUE_AndGate;
extern CU::Element CUE_OrGate;
extern CU::Element CUE_XOrGate;
// Default Special Gates
extern CU::Element CUE_ClockSynced;
//extern CU::Element CUE_ClockASynced;
