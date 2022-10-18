/*

	CU Logic

	Written by: Jonathan Clevenger

	10/16/2022
*/

#pragma once


namespace CU {

const int OscilloscopeMaxTicks = 32; // Max of 32 ticks

typedef struct OsciliTick {
    std::vector<std::string> variables;
    std::vector<LOGIC> variableData;
}OsciliTick;

class Project {
private:
    std::vector<Element> elementList;
    std::vector<Portal> fromPins;
    std::vector<Portal> toPins;
    std::vector<OsciliTick> osciliMemory;

    int currentTick = 0;
    int simulationSpeed = 250; // Number of milliseconds per gate
public:
    Project();

    void create(std::string name);

    void addElement(Element &element);

    void advance();

    int getTick();

    int getSimuSpeed();

    void printOscilloscope(CU::Driver &videoDriver);
};

};
