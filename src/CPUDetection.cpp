#include "CPUDetection.h"
#include "MSRDriver.h"

CPUDetection::CPUDetection() {
    DetectVendor();
    DetectFamily();
    DetectScheme();
}

void CPUDetection::DetectVendor() {
    int CpuIdOutput[4];
    int VendorNum = 0;

    Cpuid(CpuIdOutput, 0);
    if (CpuIdOutput[0] == 0) {
        vendor = VENDOR_UNKNOWN;
        return;
    }

    if (CpuIdOutput[1] == 0x756E6547 && CpuIdOutput[3] == 0x49656E69 && CpuIdOutput[2] == 0x6C65746E) {
        VendorNum = 1;
    }
    else if (CpuIdOutput[1] == 0x68747541 && CpuIdOutput[3] == 0x69746E65 && CpuIdOutput[2] == 0x444D4163) {
        VendorNum = 2;
    }
    else if (CpuIdOutput[1] == 0x746E6543 && CpuIdOutput[3] == 0x48727561 && CpuIdOutput[2] == 0x736C7561) {
        VendorNum = 3;
    }

    vendor = (EProcVendor)VendorNum;
}

void CPUDetection::DetectFamily() {
    int CpuIdOutput[4];
    int Family;

    family = P_UNKNOWN;
    model = 0;

    Cpuid(CpuIdOutput, 0);
    if (CpuIdOutput[0] == 0) return;

    Cpuid(CpuIdOutput, 1);
    Family = ((CpuIdOutput[0] >> 8) & 0x0F) + ((CpuIdOutput[0] >> 20) & 0xFF);
    model  = ((CpuIdOutput[0] >> 4) & 0x0F) | ((CpuIdOutput[0] >> 12) & 0xF0);

    if (vendor == INTEL) {
        if (Family <  5)    family = P_UNKNOWN;
        if (Family == 5)    family = INTEL_P1MMX;
        if (Family == 0x0F) family = INTEL_P4;
        if (Family == 6) {
            family = INTEL_P23;
            if (model == 0x09) family = INTEL_PM;
            if (model == 0x0D) family = INTEL_PM;
            if (model == 0x0E) family = INTEL_CORE;
            if (model == 0x0F) family = INTEL_CORE2;
            if (model == 0x16) family = INTEL_CORE2;
            if (model == 0x17) family = INTEL_CORE2;
            if (model == 0x1A) family = INTEL_7;
            if (model == 0x1C) family = INTEL_ATOM;
            if (model >= 0x1D) family = INTEL_7;
            if (model == 0x3A || model == 0x3E) family = INTEL_IVY;

            if (model == 0x3C || model == 0x3F || model == 0x45 || model == 0x46)
                family = INTEL_HASW;

            if (model == 0x3D || model == 0x47 || model == 0x4F || model == 0x56)
                family = INTEL_BROADWELL;

            if (model == 0x4E || model == 0x5E || model == 0x55)
                family = INTEL_SKYLAKE;

            if (model == 0x8E || model == 0x9E || model == 0xA5 || model == 0xA6)
                family = INTEL_KABYLAKE;

            if (model == 0x7D || model == 0x7E || model == 0x6A || model == 0x6C)
                family = INTEL_ICELAKE;

            if (model == 0x8C || model == 0x8D)
                family = INTEL_TIGERLAKE;

            if (family == INTEL_P23 && model >= 0x3F)
                family = INTEL_HASW;
        }
    }
    else if (vendor == AMD) {
        family = P_UNKNOWN;
        if (Family == 6)    family = AMD_ATHLON;
        if (Family >= 0x0F && Family <= 0x14) family = AMD_ATHLON64;
        if (Family >= 0x15) family = AMD_BULLD;
    }
    else if (vendor == VIA) {
        if (Family == 6 && model >= 0x0F) family = VIA_NANO;
    }
}

void CPUDetection::DetectScheme() {
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
                scheme = EPMCScheme(S_ID1 << (pmc_version - 1));
            }
        }
        if (scheme == S_UNKNOWN || (int)scheme > (int)S_ID3) {
            switch (family) {
            case INTEL_P1MMX: scheme = S_P1; break;
            case INTEL_P23: case INTEL_PM: scheme = S_P2; break;
            case INTEL_P4: scheme = S_P4; break;
            case INTEL_CORE: scheme = S_ID1; break;
            case INTEL_CORE2: scheme = S_ID2; break;
            case INTEL_7: case INTEL_IVY: case INTEL_HASW: case INTEL_BROADWELL:
            case INTEL_SKYLAKE: case INTEL_KABYLAKE: case INTEL_ICELAKE: case INTEL_TIGERLAKE:
            case INTEL_ATOM:
                scheme = S_ID3; break;
            default: break;
            }
        }
    }
}
