/*

	CU Logic

	Written by: Jonathan Clevenger

	10/16/2022
*/

#pragma once


namespace CU {

class Project {
private:
    std::vector<Element> elementList;
public:
    Project();

    void create(std::string name);

    void addElement(Element &element);

    void advance();
};

};
