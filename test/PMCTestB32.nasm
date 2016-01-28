;----------------------------------------------------------------------------
;                        PMCTestB32.nasm              © 2013-08-20 Agner Fog
;
;                PMC Test program for multiple threads
;                           NASM syntax
; Linux version (for Windows, put underscore prefix on all global names)
;
; This program is intended for testing the performance of a little piece of 
; code written in assembly language. 
; The code to test is inserted at the place marked "Test code start".
; All sections that can be modified by the user are marked with ###########. 
; 
; The code to test will be executed REPETITIONS times and the test results
; will be output for each repetition. This program measures how many clock
; cycles the code to test takes in each repetition. Furthermore, it is 
; possible to set a number of Performance Monitor Counters (PMC) to count 
; the number of micro-operations (uops), cache misses, branch mispredictions,
; etc.
; 
; The setup of the Performance Monitor Counters is microprocessor-specific.
; The specifications for PMC setup for each microprocessor family is defined
; in the tables CounterDefinitions and CounterTypesDesired.
; 
; See PMCTest.txt for instructions.
; 
; (c) Copyright 2000 - 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;----------------------------------------------------------------------------

; Define whether AVX and YMM registers used
%ifndef  USEAVX
%define  USEAVX   1
%endif

; Define cache line size (to avoid threads sharing cache lines):
%define CACHELINESIZE  64

; Define warmup count to get into max frequency state
%define WARMUPCOUNT 10000000

global TestLoop
global NumCounters
global MaxNumCounters
global EventRegistersUsed
global UsePMC
global Counters
global CounterTypesDesired
global PThreadData
global ClockResultsOS
global PMCResultsOS
global NumThreads
global ThreadDataSize
global RatioOut
global TempOut
global RatioOutTitle
global TempOutTitle


SECTION .data   align = CACHELINESIZE

;##############################################################################
;#
;#            List of desired counter types and other user definitions
;#
;##############################################################################
 
; Here you can select which performance monitor counters you want for your test.
; Select id numbers from the table CounterDefinitions[] in PMCTestA.cpp.

%define USE_PERFORMANCE_COUNTERS   1        ; Tell if you are using performance counters

; Maximum number of PMC counters
%define MAXCOUNTERS   6              ; must match value in PMCTest.h

; Number of PMC counters
%define NUM_COUNTERS  4              ; must match value in PMCTest.h

CounterTypesDesired:
    DD      1        ; core cycles (Intel only)
    DD      9        ; instructions
    DD    100        ; uops
    DD    311        ; data cache misses

times (MAXCOUNTERS - ($-CounterTypesDesired)/4)  DD 0

; Number of repetitions of test.
%define REPETITIONS  8

; Number of threads
%define NUM_THREADS  3

; Subtract overhead from clock counts (0 if not)
%define SUBTRACT_OVERHEAD  1

; Number of repetitions in loop to find overhead
%define OVERHEAD_REPETITIONS  4

; Define array sizes
%assign MAXREPEAT  REPETITIONS

;------------------------------------------------------------------------------
;
;                  global data
;
;------------------------------------------------------------------------------

; Per-thread data:
align   CACHELINESIZE, DB 0
; Data for first thread
ThreadData:                                                ; beginning of thread data block
CountTemp:     times  (MAXCOUNTERS + 1)          DD   0    ; temporary storage of counts
CountOverhead: times  (MAXCOUNTERS + 1)          DD  -1    ; temporary storage of count overhead
ClockResults:  times   REPETITIONS               DD   0    ; clock counts
PMCResults:    times  (REPETITIONS*MAXCOUNTERS)  DD   0    ; PMC counts
ALIGN   CACHELINESIZE, DB 0                                ; Make sure threads don't use same cache lines
THREADDSIZE  equ     ($ - ThreadData)                      ; size of data block for each thread

; Define data blocks of same size for remaining threads
%if  NUM_THREADS > 1
  times ((NUM_THREADS-1)*THREADDSIZE)            DB 0
%endif

; Global data
PThreadData     DD    ThreadData                ; Pointer to measured data for all threads
NumCounters     DD    0                         ; Will be number of valid counters
MaxNumCounters  DD    NUM_COUNTERS              ; Tell PMCTestA.CPP length of CounterTypesDesired
UsePMC          DD    USE_PERFORMANCE_COUNTERS  ; Tell PMCTestA.CPP if RDPMC used. Driver needed
NumThreads      DD    NUM_THREADS               ; Number of threads
ThreadDataSize  DD    THREADDSIZE               ; Size of each thread data block
ClockResultsOS  DD    ClockResults-ThreadData   ; Offset to ClockResults
PMCResultsOS    DD    PMCResults-ThreadData     ; Offset to PMCResults
Counters              times MAXCOUNTERS   DD 0  ; Counter register numbers used will be inserted here
EventRegistersUsed    times MAXCOUNTERS   DD 0  ; Set by MTMonA.cpp
RatioOut        DD    0, 0, 0, 0                ; optional ratio output. Se PMCTest.h
TempOut         DD    0                         ; optional arbitrary output. Se PMCTest.h
RatioOutTitle   DD    0                         ; optional column heading
TempOutTitle    DD    0                         ; optional column heading


%if NUM_THREADS == 1
ESP_SAVE         dd    0                         ; Save stack pointer if only one thread
%endif


;##############################################################################
;#
;#                 User data
;#
;##############################################################################
ALIGN   CACHELINESIZE, DB 0

; Put any data definitions your test code needs here

UserData           times 10000H  DB 0


;------------------------------------------------------------------------------
;
;                  Macro definitions
;
;------------------------------------------------------------------------------

%macro SERIALIZE 0             ; serialize CPU
       xor     eax, eax
       cpuid
%endmacro

%macro CLEARXMMREG 1           ; clear one xmm register
   pxor xmm%1, xmm%1
%endmacro 

%macro CLEARALLXMMREG 0        ; set all xmm or ymm registers to 0
   %if  USEAVX
      VZEROALL                 ; set all ymm registers to 0
   %else
      %assign i 0
      %rep 8
         CLEARXMMREG i         ; set all 8 xmm registers to 0
         %assign i i+1
      %endrep
   %endif
%endmacro


;------------------------------------------------------------------------------
;
;                  Test Loop
;
;------------------------------------------------------------------------------
SECTION .text   align = 16

;extern "C" ;extern "C" int TestLoop (int thread) {
; This function runs the code to test REPETITIONS times
; and reads the counters before and after each run:

TestLoop:
        push    ebx
        push    esi
        push    edi
        push    ebp
        mov     eax, [esp+16+4]            ; Thread number
        
; local variables:
;   [esp]:   thread number
;   [esp+4]: pointer to thread data block
;   [esp+8]: loop counter

        push    0
        push    0
        push    eax
        
%if NUM_THREADS == 1
        mov     [ESP_SAVE], esp            ; Save stack pointer if only one thread
%endif

        
;##############################################################################
;#
;#                 Warm up
;#
;##############################################################################
; Get into max frequency state

%if WARMUPCOUNT

        mov ecx, WARMUPCOUNT / 10
        mov eax, 1
        align 16
Warmuploop:
        %rep 10
        imul eax, ecx
        %endrep
        dec ecx
        jnz Warmuploop

%endif


;##############################################################################
;#
;#                 User Initializations 
;#
;##############################################################################
; You may add any initializations your test code needs here.
; Registers esi, edi, ebp and r8 - r12 will be unchanged from here to the 
; Test code start.

        finit                ; clear all FP registers
        
        CLEARALLXMMREG       ; clear all xmm or ymm registers

        imul eax, [esp], 2020h ; separate data for each thread
        lea esi, [eax+UserData]
        lea edi, [esi+120h]
        xor ebp, ebp
      
        

;##############################################################################
;#
;#                 End of user Initializations 
;#
;##############################################################################

        mov     ebx, ThreadData               ; address of first thread data block
        imul    eax, [esp], THREADDSIZE       ; offset to thread data block
;        DB      69H, 04H, 24H                 ; fix bug in ml.exe
;        DD	    THREADDSIZE
        add     ebx, eax                      ; address of current thread data block
        mov     [esp+4], ebx                  ; save on stack

%if  SUBTRACT_OVERHEAD
; First test loop. Measure empty code
        mov     dword [esp+8], 0              ; Loop counter

TEST_LOOP_1:

        SERIALIZE

        mov     ebx, [esp+4]      
        ; Read counters
%assign i  0
%rep    NUM_COUNTERS
        mov     ecx, [Counters + i*4]
        rdpmc
        mov     [ebx + i*4 + 4 + (CountTemp-ThreadData)], eax
%assign i  i+1
%endrep

        SERIALIZE

        mov     ebx, [esp+4]      
        ; read time stamp counter
        rdtsc
        mov     [ebx + (CountTemp-ThreadData)], eax

        SERIALIZE

        ; Empty. Test code goes here in next loop

        SERIALIZE

        mov     ebx, [esp+4]      
        ; read time stamp counter
        rdtsc
        sub     [ebx + (CountTemp-ThreadData)], eax        ; CountTemp[0]

        SERIALIZE

        mov     ebx, [esp+4]      
        ; Read counters
%assign i  0
%rep    NUM_COUNTERS
        mov     ecx, [Counters + i*4]
        rdpmc
        sub     [ebx + i*4 + 4 + (CountTemp-ThreadData)], eax  ; CountTemp[i+1]
%assign i  i+1
%endrep

        SERIALIZE

        mov     ebx, [esp+4]      
        ; find minimum counts
%assign i  0
%rep    NUM_COUNTERS + 1
        mov     eax, [ebx+i*4+(CountTemp-ThreadData)]      ; -count
        neg     eax
        mov     edx, [ebx+i*4+(CountOverhead-ThreadData)]  ; previous count
        cmp     eax, edx
        cmovb   edx, eax
        mov     [ebx+i*4+(CountOverhead-ThreadData)], edx  ; minimum count        
%assign i  i+1
%endrep
        
        ; end second test loop
        inc     dword [esp+8]
        cmp     dword [esp+8], OVERHEAD_REPETITIONS
        jb      TEST_LOOP_1

%endif   ; SUBTRACT_OVERHEAD

        
; Second test loop. Measure user code
        mov     dword [esp+8], 0          ; Loop counter

TEST_LOOP_2:

        SERIALIZE
      
        mov     ebx, [esp+4]      
        ; Read counters
%assign i  0
%rep    NUM_COUNTERS
        mov     ecx, [Counters + i*4]
        rdpmc
        mov     [ebx + i*4 + 4 + (CountTemp-ThreadData)], eax
%assign i  i+1
%endrep

        SERIALIZE

        mov     ebx, [esp+4]      
        ; read time stamp counter
        rdtsc
        mov     [ebx + (CountTemp-ThreadData)], eax

        SERIALIZE

;##############################################################################
;#
;#                 Test code start
;#
;##############################################################################

; Put the assembly code to test here

; ½½


%REP 100        ; example: 100 shift instructions

        shr eax, 5

%ENDREP


;##############################################################################
;#
;#                 Test code end
;#
;##############################################################################

        SERIALIZE

        mov     ebx, [esp+4]      
        ; read time stamp counter
        rdtsc
        sub     [ebx + (CountTemp-ThreadData)], eax        ; CountTemp[0]

        SERIALIZE

        mov     ebx, [esp+4]      
        ; Read counters
%assign i  0
%rep    NUM_COUNTERS
        mov     ecx, [Counters + i*4]
        rdpmc
        sub     [ebx + i*4 + 4 + (CountTemp-ThreadData)], eax  ; CountTemp[i+1]
%assign i  i+1
%endrep        

        SERIALIZE

        mov     ebx, [esp+4]
        mov     ecx, [esp+8]
        ; subtract counts before from counts after
        mov     eax, [ebx + (CountTemp-ThreadData)]            ; -count
        neg     eax
%if     SUBTRACT_OVERHEAD
        sub     eax, [ebx+(CountOverhead-ThreadData)]   ; overhead clock count        
%endif  ; SUBTRACT_OVERHEAD        
        mov     [ebx+ecx*4+(ClockResults-ThreadData)], eax      ; save clock count
        
%assign i  0
%rep    NUM_COUNTERS
        mov     eax, [ebx + i*4 + 4 + (CountTemp-ThreadData)]
        neg     eax
%if     SUBTRACT_OVERHEAD
        sub     eax, [ebx+i*4+4+(CountOverhead-ThreadData)]   ; overhead pmc count        
%endif  ; SUBTRACT_OVERHEAD        
        mov     [ebx+ecx*4+i*4*REPETITIONS+(PMCResults-ThreadData)], eax      ; save count        
%assign i  i+1
%endrep        
        
        ; end second test loop
        inc     dword [esp+8]
        cmp     dword [esp+8], REPETITIONS
        jb      TEST_LOOP_2

        ; clean up
        finit
        cld
%if USEAVX
        VZEROALL                       ; clear all ymm registers
%endif

        push    ds
        pop     es
%if NUM_THREADS == 1
        mov     esp, [ESP_SAVE]        ; Restore stack pointer if only one thread
%endif

        ; return REPETITIONS;
        mov     eax, REPETITIONS
        add     esp, 12
        pop     ebp
        pop     edi
        pop     esi
        pop     ebx
        ret
        
; End of TestLoop
