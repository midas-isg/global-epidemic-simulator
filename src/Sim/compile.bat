set LIBS= -L..\..\lib\win32
set INCLUDE=..\..\include\win
set COMPILE=g++ -I%INCLUDE% -fopenmp -O2 -Wall -c -fmessage-length=0  -o
set LINK=g++ -Wl,--enable-auto-import %LIBS% -fopenmp -o

call %COMPILE%sim.o sim.cpp
call %COMPILE%world.o world.cpp
call %COMPILE%gps_math.o gps_math.cpp
call %COMPILE%person.o person.cpp
call %COMPILE%place.o place.cpp
call %COMPILE%DBOpsPar.o DBOpsPar.cpp
call %COMPILE%patch.o patch.cpp
call %COMPILE%initialise.o initialise.cpp
call %COMPILE%randlib_par.o randlib_par.cpp
call %COMPILE%intervention.o intervention.cpp
call %COMPILE%unit.o unit.cpp
call %COMPILE%messages.o messages.cpp
call %COMPILE%params.o params.cpp
call %COMPILE%lodepng.o lodepng.cpp
call %COMPILE%household.o household.cpp
call %COMPILE%output.o output.cpp

call %LINK%Sim.exe world.o unit.o sim.o randlib_par.o place.o person.o patch.o params.o output.o messages.o lodepng.o intervention.o initialise.o household.o gps_math.o DBOpsPar.o -lmsmpi -lodbc32

del *.o /Q
