// Standalone tool to list available PMC counters for the current CPU
//
// This tool has minimal dependencies - it only needs:
// 1. CPU detection code (vendor/family/scheme)
// 2. Access to CounterDefinitions array

#include "PMCTest.h"
#include <stdio.h>

// CounterDefinitions array is defined in PMCTestA.cpp
extern SCounterDefinition CounterDefinitions[];

// Simple CPU detection - subset of CCounters functionality
class SimpleCPU {
private:
    EProcVendor vendor;
    EProcFamily family;
    EPMCScheme scheme;

    void DetectVendor() {
        int CpuIdOutput[4];
        int VendorNum = 0;

        Cpuid(CpuIdOutput, 0);
        if (CpuIdOutput[0] == 0) {
            vendor = VENDOR_UNKNOWN;
            return;
        }

        // Check vendor string
        if (CpuIdOutput[1] == 0x756E6547 && CpuIdOutput[3] == 0x49656E69 && CpuIdOutput[2] == 0x6C65746E) {
            VendorNum = 1;  // Intel
        }
        else if (CpuIdOutput[1] == 0x68747541 && CpuIdOutput[3] == 0x69746E65 && CpuIdOutput[2] == 0x444D4163) {
            VendorNum = 2;  // AMD
        }
        else if (CpuIdOutput[1] == 0x746E6543 && CpuIdOutput[3] == 0x48727561 && CpuIdOutput[2] == 0x736C7561) {
            VendorNum = 3;  // VIA
        }

        vendor = (EProcVendor)VendorNum;
    }

    void DetectFamily() {
        int CpuIdOutput[4];
        int Family, Model;

        family = P_UNKNOWN;

        Cpuid(CpuIdOutput, 0);
        if (CpuIdOutput[0] == 0) return;

        Cpuid(CpuIdOutput, 1);
        Family = ((CpuIdOutput[0] >> 8) & 0x0F) + ((CpuIdOutput[0] >> 20) & 0xFF);
        Model  = ((CpuIdOutput[0] >> 4) & 0x0F) | ((CpuIdOutput[0] >> 12) & 0xF0);

        if (vendor == INTEL) {
            if (Family <  5)    family = P_UNKNOWN;
            if (Family == 5)    family = INTEL_P1MMX;
            if (Family == 0x0F) family = INTEL_P4;
            if (Family == 6) {
                family = INTEL_P23;
                if (Model == 0x09) family = INTEL_PM;
                if (Model == 0x0D) family = INTEL_PM;
                if (Model == 0x0E) family = INTEL_CORE;
                if (Model == 0x0F) family = INTEL_CORE2;
                if (Model == 0x16) family = INTEL_CORE2;
                if (Model == 0x17) family = INTEL_CORE2;
                if (Model == 0x1A) family = INTEL_7;
                if (Model == 0x1C) family = INTEL_ATOM;
                if (Model >= 0x1D) family = INTEL_7;
                if (Model == 0x3A) family = INTEL_IVY;
                if (Model == 0x3E) family = INTEL_IVY;
                if (Model >= 0x3F) family = INTEL_HASW;
            }
        }
        else if (vendor == AMD) {
            family = P_UNKNOWN;
            if (Family == 6)    family = AMD_ATHLON;
            if (Family >= 0x0F && Family <= 0x14) family = AMD_ATHLON64;
            if (Family >= 0x15) family = AMD_BULLD;
        }
        else if (vendor == VIA) {
            if (Family == 6 && Model >= 0x0F) family = VIA_NANO;
        }
    }

    void DetectScheme() {
        scheme = S_UNKNOWN;

        if (vendor == AMD) {
            scheme = S_AMD;
        }
        else if (vendor == VIA) {
            scheme = S_VIA;
        }
        else if (vendor == INTEL) {
            int CpuIdOutput[4];
            Cpuid(CpuIdOutput, 0);
            if (CpuIdOutput[0] >= 0x0A) {
                Cpuid(CpuIdOutput, 0x0A);
                int pmc_version = CpuIdOutput[0] & 0xFF;
                if (pmc_version > 0 && pmc_version <= 4) {
                    // Valid PMC architecture version (1-4 maps to S_ID1 through S_ID3+)
                    scheme = EPMCScheme(S_ID1 << (pmc_version - 1));
                }
            }
            // Use family-based fallback if CPUID didn't give us a valid scheme
            if (scheme == S_UNKNOWN || (int)scheme > (int)S_ID3) {
                switch (family) {
                case INTEL_P1MMX: scheme = S_P1; break;
                case INTEL_P23: case INTEL_PM: scheme = S_P2; break;
                case INTEL_P4: scheme = S_P4; break;
                case INTEL_CORE: scheme = S_ID1; break;
                case INTEL_CORE2: scheme = S_ID2; break;
                case INTEL_7: case INTEL_ATOM: case INTEL_IVY: case INTEL_HASW: scheme = S_ID3; break;
                default: break;
                }
            }
        }
    }

public:
    SimpleCPU() {
        DetectVendor();
        DetectFamily();
        DetectScheme();
    }

    EPMCScheme GetScheme() const { return scheme; }
    EProcFamily GetFamily() const { return family; }
};

int main() {
    SimpleCPU cpu;
    EPMCScheme scheme = cpu.GetScheme();
    EProcFamily family = cpu.GetFamily();

    // Debug output (to stderr so it doesn't interfere with CSV)
    fprintf(stderr, "Detected CPU - Scheme: 0x%x, Family: 0x%x\n", scheme, family);

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
