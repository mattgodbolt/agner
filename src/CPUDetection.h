#pragma once

#include "PMCTest.h"

class CPUDetection {
public:
    CPUDetection();

    EPMCScheme GetScheme() const { return scheme; }
    EProcFamily GetFamily() const { return family; }
    EProcVendor GetVendor() const { return vendor; }
    int GetModel() const { return model; }

private:
    EProcVendor vendor;
    EProcFamily family;
    EPMCScheme scheme;
    int model;

    void DetectVendor();
    void DetectFamily();
    void DetectScheme();
};
