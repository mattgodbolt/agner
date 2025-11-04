// Standalone tool to list available PMC counters for the current CPU
//
// This tool has minimal dependencies - it only needs:
// 1. CPU detection code (vendor/family/scheme)
// 2. Access to CounterDefinitions array

#include "PMCTest.h"
#include "CPUDetection.h"
#include <stdio.h>

int main() {
    CPUDetection cpu;
    EPMCScheme scheme = cpu.GetScheme();
    EProcFamily family = cpu.GetFamily();
    int model = cpu.GetModel();

    // Debug output (to stderr so it doesn't interfere with CSV)
    fprintf(stderr, "Detected CPU - Model: 0x%x, Scheme: 0x%x, Family: 0x%x\n", model, scheme, family);

    // Print CSV header
    printf("counter_id,name,supported,scheme,family\n");

    // List all counters
    for (int i = 0; CounterDefinitions[i].CounterType || CounterDefinitions[i].ProcessorFamily; i++) {
        SCounterDefinition* def = &CounterDefinitions[i];
        int supported = (def->PMCScheme & scheme) && (def->ProcessorFamily & family);
        printf("%d,%s,%d,0x%x,0x%x\n",
            def->CounterType,
            def->Description,
            supported,
            def->PMCScheme,
            def->ProcessorFamily);
    }

    return 0;
}
