comment & ---------------------------------------------------------------------
                          PMCTestB64.asm              © 2013-08-20 Agner Fog

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

; Operating system: 0 = Linux, 1 = Windows
WINDOWS  EQU    1

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

CounterTypesDesired label DWORD
    DD      1        ; core cycles (Intel only)
    DD      9        ; instructions
    DD    100        ; uops
    DD    311        ; data cache misses

    
; Number of counters defined
IF USE_PERFORMANCE_COUNTERS
NUM_COUNTERS = ($ - CounterTypesDesired) / 4
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
OVERHEAD_REPETITIONS = 4

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

public NumCounters, MaxNumCounters, EventRegistersUsed
public UsePMC, Counters, CounterTypesDesired
public PThreadData, ClockResultsOS, PMCResultsOS, NumThreads, ThreadDataSize
public RatioOut, TempOut, RatioOutTitle, TempOutTitle


; Per-thread data:
ALIGN   CACHELINESIZE
; Data for first thread
ThreadData label dword                                     ; beginning of thread data block
CountTemp        DD    MAXCOUNTERS + 1            dup (0)  ; temporary storage of counts
CountOverhead    DD    MAXCOUNTERS + 1            dup (-1) ; temporary storage of count overhead
ClockResults     DD    REPETITIONS                dup (0)  ; clock counts
PMCResults       DD    REPETITIONS * MAXCOUNTERS  dup (0)  ; PMC counts
align 8
RSPSave          DQ    0                                 ; save stack pointer
ALIGN   CACHELINESIZE                  ; Make sure threads don't use same cache lines
THREADDSIZE = (offset $ - offset ThreadData)          ; size of data block for each thread

; Define data blocks of same size for remaining threads
IF NUM_THREADS GT 1
DB (NUM_THREADS - 1) * THREADDSIZE DUP (0)
ENDIF

; Global data
PThreadData     DQ    ThreadData               ; Pointer to measured data for all threads
NumCounters     DD    0                        ; Will be number of valid counters
MaxNumCounters  DD    NUM_COUNTERS             ; Tell PMCTestA.CPP length of CounterTypesDesired
UsePMC          DD    USE_PERFORMANCE_COUNTERS ; Tell PMCTestA.CPP if RDPMC used. Driver needed
NumThreads      DD    NUM_THREADS              ; Number of threads
ThreadDataSize  DD    THREADDSIZE              ; Size of each thread data block
ClockResultsOS  DD    ClockResults-ThreadData  ; Offset to ClockResults
PMCResultsOS    DD    PMCResults-ThreadData    ; Offset to PMCResults
Counters        DD    MAXCOUNTERS dup (0)      ; Counter register numbers used will be inserted here
EventRegistersUsed DD MAXCOUNTERS dup (0)      ; Set by MTMonA.cpp


; optional extra output column definitions
RatioOut      DD   0, 0, 0, 0                ; optional ratio output. Se PMCTest.h
TempOut       DD   0, 0                      ; optional arbitrary output. Se PMCTest.h
RatioOutTitle DQ   0                         ; optional column heading
TempOutTitle  DQ   0                         ; optional column heading


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
		REPT 16
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

;extern "C" int TestLoop (int thread) {
; This function runs the code to test REPETITIONS times
; and reads the counters before and after each run:

TestLoop PROC
        push    rbx
        push    rbp
        push    r12
        push    r13
        push    r14
        push    r15
IF      WINDOWS                    ; These registers must be saved in Windows, not in Linux
        push    rsi
        push    rdi
        sub     rsp, 0A8H           ; Space for saving xmm6 - 15 and align
        movaps  [rsp], xmm6
        movaps  [rsp+10H], xmm7
        movaps  [rsp+20H], xmm8
        movaps  [rsp+30H], xmm9
        movaps  [rsp+40H], xmm10
        movaps  [rsp+50H], xmm11
        movaps  [rsp+60H], xmm12
        movaps  [rsp+70H], xmm13
        movaps  [rsp+80H], xmm14
        movaps  [rsp+90H], xmm15        
        mov     r15d, ecx          ; Thread number
ELSE    ; Linux
        mov     r15d, edi          ; Thread number
ENDIF
        
; Register use:
;   r13: pointer to thread data block
;   r14: loop counter
;   r15: thread number
;   rax, rbx, rcx, rdx: scratch
;   all other registers: available to user program


;##############################################################################
;#
;#                 User Initializations 
;#
;##############################################################################
; You may add any initializations your test code needs here.
; Registers esi, edi, ebp and r8 - r12 will be unchanged from here to the 
; Test code start.
; 

        finit                ; clear all FP registers
        
        CLEARALLXMMREG       ; clear all xmm or ymm registers

        lea rsi, d0
        lea rdi,[rsi+120h]
        xor ebp,ebp
        

;##############################################################################
;#
;#                 End of user Initializations 
;#
;##############################################################################

        lea     r13, [ThreadData]             ; address of first thread data block
        ;imul    eax, r15d, THREADDSIZE       ; offset to thread data block
        DB      41H, 69H, 0C7H                ; fix bug in ml64
        DD      THREADDSIZE
        add     r13, rax                      ; address of current thread data block
        mov     [r13+(RSPSave-ThreadData)],rsp ; save stack pointer

IF  SUBTRACT_OVERHEAD
; First test loop. Measure empty code
        xor     r14d, r14d                    ; Loop counter

TEST_LOOP_1:

        SERIALIZE
      
        ; Read counters
        I = 0
REPT    NUM_COUNTERS
        mov     ecx, [Counters + I*4]
        rdpmc
        mov     [r13 + I*4 + 4 + (CountTemp-ThreadData)], eax
        I = I + 1
ENDM       

        SERIALIZE

        ; read time stamp counter
        rdtsc
        mov     [r13 + (CountTemp-ThreadData)], eax

        SERIALIZE

        ; Empty. Test code goes here in next loop

        SERIALIZE

        ; read time stamp counter
        rdtsc
        sub     [r13 + (CountTemp-ThreadData)], eax        ; CountTemp[0]

        SERIALIZE

        ; Read counters
        I = 0
REPT    NUM_COUNTERS
        mov     ecx, [Counters + I*4]
        rdpmc
        sub     [r13 + I*4 + 4 + (CountTemp-ThreadData)], eax  ; CountTemp[I+1]
        I = I + 1
ENDM       

        SERIALIZE

        ; find minimum counts
        I = 0
REPT    NUM_COUNTERS + 1
        mov     eax, [r13+I*4+(CountTemp-ThreadData)]       ; -count
        neg     eax
        mov     ebx, [r13+I*4+(CountOverhead-ThreadData)]   ; previous count
        cmp     eax, ebx
        cmovb   ebx, eax
        mov     [r13+I*4+(CountOverhead-ThreadData)], ebx   ; minimum count        
        I = I + 1
ENDM       
        
        ; end second test loop
        inc     r14d
        cmp     r14d, OVERHEAD_REPETITIONS
        jb      TEST_LOOP_1

ENDIF   ; SUBTRACT_OVERHEAD

        
; Second test loop. Measure user code
        xor     r14d, r14d                    ; Loop counter

TEST_LOOP_2:

        SERIALIZE
      
        ; Read counters
        I = 0
REPT    NUM_COUNTERS
        mov     ecx, [Counters + I*4]
        rdpmc
        mov     [r13 + I*4 + 4 + (CountTemp-ThreadData)], eax
        I = I + 1
ENDM       

        SERIALIZE

        ; read time stamp counter
        rdtsc
        mov     [r13 + (CountTemp-ThreadData)], eax

        SERIALIZE

;##############################################################################
;#
;#                 Test code start
;#
;##############################################################################

; Put the assembly code to test here
; Don't modify r13, r14, r15!

; ½½

rept 100        ; example: 100 shift instructions

shr eax,5

endm



;##############################################################################
;#
;#                 Test code end
;#
;##############################################################################

        SERIALIZE

        ; read time stamp counter
        rdtsc
        sub     [r13 + (CountTemp-ThreadData)], eax        ; CountTemp[0]

        SERIALIZE

        ; Read counters
        I = 0
REPT    NUM_COUNTERS
        mov     ecx, [Counters + I*4]
        rdpmc
        sub     [r13 + I*4 + 4 + (CountTemp-ThreadData)], eax  ; CountTemp[I+1]
        I = I + 1
ENDM       

        SERIALIZE

        ; subtract counts before from counts after
        mov     eax, [r13 + (CountTemp-ThreadData)]            ; -count
        neg     eax
IF      SUBTRACT_OVERHEAD
        sub     eax, [r13+(CountOverhead-ThreadData)]   ; overhead clock count        
ENDIF   ; SUBTRACT_OVERHEAD        
        mov     [r13+r14*4+(ClockResults-ThreadData)], eax      ; save clock count
        
        I = 0
REPT    NUM_COUNTERS
        mov     eax, [r13 + I*4 + 4 + (CountTemp-ThreadData)]
        neg     eax
IF      SUBTRACT_OVERHEAD
        sub     eax, [r13+I*4+4+(CountOverhead-ThreadData)]   ; overhead pmc count        
ENDIF   ; SUBTRACT_OVERHEAD        
        mov     [r13+r14*4+I*4*REPETITIONS+(PMCResults-ThreadData)], eax      ; save count        
        I = I + 1
ENDM       
        
        ; end second test loop
        inc     r14d
        cmp     r14d, REPETITIONS
        jb      TEST_LOOP_2

        ; clean up
        mov     rsp, [r13+(RSPSave-ThreadData)]   ; restore stack pointer        
        finit
        cld
IF USEAVX
        VZEROALL                 ; clear all ymm registers
ENDIF        

        ; return REPETITIONS;
        mov     eax, REPETITIONS   ; return value
        
IF      WINDOWS                    ; Restore registers saved in Windows
        movaps  xmm6, [rsp]
        movaps  xmm7, [rsp+10H]
        movaps  xmm8, [rsp+20H]
        movaps  xmm9, [rsp+30H]
        movaps  xmm10, [rsp+40H]
        movaps  xmm11, [rsp+50H]
        movaps  xmm12, [rsp+60H]
        movaps  xmm13, [rsp+70H]
        movaps  xmm14, [rsp+80H]
        movaps  xmm15, [rsp+90H]
        add     rsp, 0A8H           ; Free space for saving xmm6 - 15
        pop     rdi
        pop     rsi
ENDIF
        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     rbp
        pop     rbx
        ret
        
TestLoop ENDP

END
