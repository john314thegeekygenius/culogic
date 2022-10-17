/*

	CU Logic

	Written by: Jonathan Clevenger

	10/16/2022
*/

#pragma once


namespace CU {

enum class LOGIC {
    LOW = false,
    HIGH = true,
};

typedef struct ElementPin {
    LOGIC data;
    LOGIC NOT(){
        if(data == LOGIC::LOW)
            return LOGIC::HIGH;
        return LOGIC::LOW;
    };
    LOGIC SELF(){
        return data;
    };
}ElementPin;

typedef LOGIC (*ElementFunction)(std::vector<ElementPin*> &inputs);

typedef struct ElementSymbol {
    std::vector<int> data;
}ElementSymbol;

class Element {
private:
    std::vector<ElementPin*> inputs;
    ElementSymbol symbol;
public:
    Element();
    Element(ElementFunction func, ElementSymbol symb);

    ElementFunction result = nullptr;
    void setFunction(ElementFunction func);
    void setSymbol(ElementSymbol symb);
};
/*
class Component {
private:
    std::vector<Element> ElementList;
public:


};
*/

};


// Default logic gates
extern CU::Element CUE_NotGate;
extern CU::Element CUE_AndGate;
extern CU::Element CUE_OrGate;
extern CU::Element CUE_XOrGate;


