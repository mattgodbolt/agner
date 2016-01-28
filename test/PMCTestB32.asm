comment & ---------------------------------------------------------------------
                          PMCTestB32.asm              © 2013-08-20 Agner Fog

                PMC Test program for multiple threads

This program is intended for testing the performance of a little piece of 
code written in assembly language. 
The code to test is inserted at the place marked "Test code start".
All sections that can be modified by the user are marked with ###########. 

The code to test will be executed REPETITIONS times and the test results
will be output for each repetition. This program measures how many clock
cycles the code to test takes in each repetition. Furthermore, it is 
possible to set a number of Performance Monitor Counters (PMC) to count 
the number of micro-operations (uops), cache misses, branch mispredictions,
etc.

The setup of the Performance Monitor Counters is microprocessor-specific.
The specifications for PMC setup for each microprocessor family is defined
in the tables CounterDefinitions and CounterTypesDesired.

See PMCTest.txt for instructions.

© 2000-2013 GNU General Public License www.gnu.org/licenses

----------------------------------------------------------------------------- &

; Define platform
.686
.xmm
.model flat

; Define whether AVX and YMM registers used
USEAVX        = 0

; Define cache line size (to avoid threads sharing cache lines):
CACHELINESIZE = 64

DATA SEGMENT ALIGN(CACHELINESIZE)

;##############################################################################
;#
;#            List of desired counter types and other user definitions
;#
;##############################################################################
 
; Here you can select which performance monitor counters you want for your test.
; Select id numbers from the table CounterDefinitions[] in PMCTestA.cpp.

USE_PERFORMANCE_COUNTERS  equ  1        ; Tell if you are using performance counters

_CounterTypesDesired label DWORD
    DD      1        ; core cycles (Intel only)
    DD      9        ; instructions
    DD    100        ; uops
    DD    311        ; data cache misses

    
; Number of counters defined
IF USE_PERFORMANCE_COUNTERS
NUM_COUNTERS = ($ - _CounterTypesDesired) / 4
ELSE
NUM_COUNTERS = 0
ENDIF

; Number of repetitions of test.
REPETITIONS = 12

; Number of threads
NUM_THREADS = 1

; Subtract overhead from clock counts (0 if not)
SUBTRACT_OVERHEAD = 1

; Number of repetitions in loop to find overhead
OVERHEAD_REPETITIONS = 5

; Maximum number of PMC counters
MAXCOUNTERS = 6              ; must match value in PMCTest.h

IF NUM_COUNTERS GT MAXCOUNTERS
   NUM_COUNTERS = MAXCOUNTERS
ENDIF

; Define array sizes
MAXREPEAT = REPETITIONS

;------------------------------------------------------------------------------
;
;                  global data
;
;------------------------------------------------------------------------------

public _NumCounters, _MaxNumCounters, _EventRegistersUsed
public _UsePMC, _Counters, _CounterTypesDesired
public _PThreadData, _ClockResultsOS, _PMCResultsOS
public _NumThreads, _ThreadDataSize
public _RatioOut, _TempOut, _RatioOutTitle, _TempOutTitle


; Per-thread data:
ALIGN   CACHELINESIZE
; Data for first thread
ThreadData      label dword
CountTemp        DD    MAXCOUNTERS + 1          dup (0)  ; temporary storage of counts
CountOverhead    DD    MAXCOUNTERS + 1          dup (-1) ; temporary storage of count overhead
ClockResults     DD    MAXREPEAT                dup (0)  ; clock counts
PMCResults       DD    MAXREPEAT * MAXCOUNTERS  dup (0)  ; PMC counts
ALIGN   CACHELINESIZE                  ; Make sure threads don't use same cache lines
THREADDSIZE = (offset $ - offset CountTemp)          ; size of data block for each thread

; Define data blocks of same size for remaining threads
IF NUM_THREADS GT 1
DB (NUM_THREADS - 1) * THREADDSIZE DUP (0)
ENDIF

; Global data
_PThreadData     DD    ThreadData               ; Pointer to measured data for all threads
_NumCounters     DD    0                        ; Will be number of valid counters
_MaxNumCounters  DD    NUM_COUNTERS             ; Tell PMCTestA.CPP length of CounterTypesDesired
_UsePMC          DD    USE_PERFORMANCE_COUNTERS ; Tell PMCTestA.CPP if RDPMC used. Driver needed
_NumThreads      DD    NUM_THREADS              ; Number of threads
_ThreadDataSize  DD    THREADDSIZE              ; Size of each thread data block
_ClockResultsOS  DD    ClockResults-ThreadData  ; Offset to ClockResults
_PMCResultsOS    DD    PMCResults-ThreadData    ; Offset to PMCResults
_Counters        DD    MAXCOUNTERS dup (0)      ; Counter register numbers used will be inserted here
_EventRegistersUsed DD MAXCOUNTERS dup (0)      ; Set by MTMonA.cpp

IF NUM_THREADS EQ 1
ESP_SAVE         DD    0                        ; Save stack pointer if only one thread
ENDIF

; optional extra output column definitions
_RatioOut        DD   0, 0, 0, 0                ; optional ratio output. Se PMCTest.h
_TempOut         DD   0                         ; optional arbitrary output. Se PMCTest.h
_RatioOutTitle   DD   0                         ; optional column heading
_TempOutTitle    DD   0                         ; optional column heading


;##############################################################################
;#
;#                 User data
;#
;##############################################################################
ALIGN   CACHELINESIZE

; Put any data definitions your test code needs here

d0 label dword
q0 label qword
UserData         DD    1000H dup (0)


;------------------------------------------------------------------------------
;
;                  Macro definitions
;
;------------------------------------------------------------------------------

SERIALIZE MACRO             ; serialize CPU
       xor     eax, eax
       cpuid
ENDM

CLEARXMMREG MACRO N         ; set xmm(N) register to 0
        pxor xmm&N, xmm&N
ENDM		 

CLEARALLXMMREG MACRO        ; set all xmm registers to 0
IF  USEAVX
        VZEROALL            ; clear all ymm registers
ELSE        
		I = 0
		REPT 8
        CLEARXMMREG %I      ; clear all xmm registers
		I = I + 1
		ENDM
ENDIF
ENDM		 


;------------------------------------------------------------------------------
;
;                  Test Loop
;
;------------------------------------------------------------------------------
.code

;extern "C" ;extern "C" int MonTestLoop (int thread) {
; This function runs the code to test REPETITIONS times
; and reads the counters before and after each run:

_TestLoop PROC NEAR PUBLIC
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
        
IF NUM_THREADS EQ 1
        mov     [ESP_SAVE], esp              ; Save stack pointer if only one thread
ENDIF
        

;##############################################################################
;#
;#                 User Initializations 
;#
;##############################################################################
; You may add any initializations your test code needs here.
; Registers esi, edi and ebp will be unchanged from here to the test code start.

        finit                ; clear all FP registers
        
        CLEARALLXMMREG       ; clear all xmm or ymm registers

        lea esi, d0
        lea edi, [esi+120h]
        xor ebp, ebp
        

;##############################################################################
;#
;#                 End of user Initializations 
;#
;##############################################################################

        mov     ebx, offset [ThreadData]      ; address of first thread data block
;       imul    eax, [esp], THREADDSIZE       ; offset to thread data block
        DB      69H, 04H, 24H                 ; fix bug in ml.exe by hardcoding imul instruction
        DD	    THREADDSIZE
        add     ebx, eax                      ; address of current thread data block
        mov     [esp+4], ebx                  ; save on stack

IF  SUBTRACT_OVERHEAD
; First test loop. Measure empty code
        mov     dword ptr [esp+8], 0          ; Loop counter

TEST_LOOP_1:

        SERIALIZE

        mov     ebx, [esp+4]      
        ; Read counters
        I = 0
REPT    NUM_COUNTERS
        mov     ecx, [_Counters + I*4]
        rdpmc
        mov     [ebx + I*4 + 4 + (CountTemp-ThreadData)], eax
        I = I + 1
ENDM       

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
        I = 0
REPT    NUM_COUNTERS
        mov     ecx, [_Counters + I*4]
        rdpmc
        sub     [ebx + I*4 + 4 + (CountTemp-ThreadData)], eax  ; CountTemp[I+1]
        I = I + 1
ENDM       

        SERIALIZE

        mov     ebx, [esp+4]      
        ; find minimum counts
        I = 0
REPT    NUM_COUNTERS + 1
        mov     eax, [ebx+I*4+(CountTemp-ThreadData)]            ; -count
        neg     eax
        mov     edx, [ebx+I*4+(CountOverhead-ThreadData)]   ; previous count
        cmp     eax, edx
        cmovb   edx, eax
        mov     [ebx+I*4+(CountOverhead-ThreadData)], edx   ; minimum count        
        I = I + 1
ENDM       
        
        ; end second test loop
        inc     dword ptr [esp+8]
        cmp     dword ptr [esp+8], OVERHEAD_REPETITIONS
        jb      TEST_LOOP_1

ENDIF   ; SUBTRACT_OVERHEAD

        
; Second test loop. Measure user code
        mov     dword ptr [esp+8], 0          ; Loop counter

TEST_LOOP_2:

        SERIALIZE
      
        mov     ebx, [esp+4]      
        ; Read counters
        I = 0
REPT    NUM_COUNTERS
        mov     ecx, [_Counters + I*4]
        rdpmc
        mov     [ebx + I*4 + 4 + (CountTemp-ThreadData)], eax
        I = I + 1
ENDM       

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

REPT 100    ; Example: 100 shift instructions

shr eax,5

ENDM


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
        I = 0
REPT    NUM_COUNTERS
        mov     ecx, [_Counters + I*4]
        rdpmc
        sub     [ebx + I*4 + 4 + (CountTemp-ThreadData)], eax  ; CountTemp[I+1]
        I = I + 1
ENDM       

        SERIALIZE

        mov     ebx, [esp+4]
        mov     ecx, [esp+8]
        ; subtract counts before from counts after
        mov     eax, [ebx + (CountTemp-ThreadData)]            ; -count
        neg     eax
IF      SUBTRACT_OVERHEAD
        sub     eax, [ebx+(CountOverhead-ThreadData)]   ; overhead clock count        
ENDIF   ; SUBTRACT_OVERHEAD        
        mov     [ebx+ecx*4+(ClockResults-ThreadData)], eax      ; save clock count
        
        I = 0
REPT    NUM_COUNTERS
        mov     eax, [ebx + I*4 + 4 + (CountTemp-ThreadData)]
        neg     eax
IF      SUBTRACT_OVERHEAD
        sub     eax, [ebx+I*4+4+(CountOverhead-ThreadData)]   ; overhead pmc count        
ENDIF   ; SUBTRACT_OVERHEAD        
        mov     [ebx+ecx*4+I*4*REPETITIONS+(PMCResults-ThreadData)], eax      ; save count        
        I = I + 1
ENDM       
        
        ; end second test loop
        inc     dword ptr [esp+8]
        cmp     dword ptr [esp+8], REPETITIONS
        jb      TEST_LOOP_2

        ; clean up
        finit
        cld
IF USEAVX
        VZEROALL                       ; clear all ymm registers
ENDIF        

        push    ds
        pop     es
IF NUM_THREADS EQ 1
        mov     esp, [ESP_SAVE]        ; Restore stack pointer if only one thread
ENDIF        

        ; return REPETITIONS;
        mov     eax, REPETITIONS
        add     esp, 12
        pop     ebp
        pop     edi
        pop     esi
        pop     ebx
        ret
        
_TestLoop ENDP

END
