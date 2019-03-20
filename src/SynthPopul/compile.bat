@echo off

SET PARALLEL=1

SET LIBS= -L..\..\lib\win32

IF %PARALLEL% EQU 1 (ECHO PARALLEL IS defined) ELSE (ECHO PARALLEL is NOT defined)

IF %PARALLEL% EQU 1 (SET COMPILE=g++ -fopenmp -O2 -c -fmessage-length=0 -o) ELSE (SET COMPILE=g++ -O2 -c -fmessage-length=0 -o)

SET LINK=g++ -Wl,--enable-auto-import %LIBS% -fopenmp -o

CALL %COMPILE%Itoa.o Itoa.cpp
CALL %COMPILE%Country.o Country.cpp
CALL %COMPILE%EstablishmentDistr.o EstablishmentDistr.cpp
CALL %COMPILE%EstablishmentStruct.o EstablishmentStruct.cpp
CALL %COMPILE%HouseholdStruct.o HouseholdStruct.cpp
CALL %COMPILE%LandScan.o LandScan.cpp
CALL %COMPILE%Main.o Main.cpp
CALL %COMPILE%PopulDistr.o PopulDistr.cpp
CALL %COMPILE%Stubs.o Stubs.cpp
CALL %COMPILE%Util.o Util.cpp
CALL %COMPILE%VacancyAndEstablishmentMap.o VacancyAndEstablishmentMap.cpp

IF %PARALLEL% EQU 1 (
CALL %COMPILE%randlib_par.o randlib_par\randlib_par.cpp
CALL %LINK%SynthPopul.exe Itoa.o Country.o EstablishmentDistr.o EstablishmentStruct.o HouseholdStruct.o LandScan.o PopulDistr.o Stubs.o Util.o VacancyAndEstablishmentMap.o randlib_par.o Main.o -lodbc32
) ELSE (
CALL %COMPILE%com.o randlib\com.cpp
CALL %COMPILE%linpack.o randlib\linpack.cpp
CALL %COMPILE%randlib.o randlib\randlib.cpp
CALL %LINK%SynthPopul.exe Itoa.o Country.o EstablishmentDistr.o EstablishmentStruct.o HouseholdStruct.o LandScan.o PopulDistr.o Stubs.o Util.o VacancyAndEstablishmentMap.o com.o linpack.o randlib.o Main.o -lodbc32
)


DEL *.o /Q
