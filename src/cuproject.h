/*

	CU Logic

	Written by: Jonathan Clevenger

	10/16/2022
*/

#pragma once


namespace CU {

class Project {
private:
    std::vector<Component> components;
public:
    Project();

    void addComponent();
};

};
