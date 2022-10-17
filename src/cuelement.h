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

class Element {
private:
    std::vector<ElementPin*> inputs;
public:
    Element();
    Element(ElementFunction func);

    ElementFunction result = nullptr;
    void setFunction(ElementFunction func);
};

class Component {
private:
    std::vector<Element> ElementList;
public:


};


// Default logic gates
extern Element CUE_NotGate;
extern Element CUE_AndGate;
extern Element CUE_OrGate;
extern Element CUE_XOrGate;

};



