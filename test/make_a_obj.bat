rem              make_a_obj.bat                     2012-03-02 Agner Fog

rem  compiles PMCTestA.cpp into PMCTestA32.obj and PMCTestA64.obj

rem  System requirements:
rem  Windows 2000 or NT or later
rem  Microsoft Visual C++ compiler or other C++ compiler

rem  You have to change all paths to the actual paths on your computer

rem  Set path to 32 bit compiler
set VSroot=C:\Program Files\Microsoft Visual Studio 9.0
set SDKroot=C:\Program Files\Microsoft SDKs\Windows\v6.0A
set path1=%path%
set path=%VSroot%\VC\bin;%VSroot%\Common7\IDE;%path1%

rem  Set path to *.h include files.
set include=%VSroot%\VC\include;%SDKroot%\include

rem  Set path to *.lib library files. 
set lib="%VSroot%\VC\lib"

rem compile 32 bit version
cl /c /O2 /FoPMCTestA32.obj PMCTestA.cpp
if errorlevel 1 pause


rem  Set path to 64 bit compiler
set path=%VSroot%\VC\bin\x86_amd64;%VSroot%\Common7\IDE;%path1%

rem  Set path to *.lib library files. 
set lib="%VSroot%\VC\lib\amd64"

rem compile 64 bit version
cl /c /O2 /FoPMCTestA64.obj PMCTestA.cpp
if errorlevel 1 pause

pause
