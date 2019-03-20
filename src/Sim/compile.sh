INCLUDE="../../include/linux"
LIB="../../lib/linux"
COMPILE="g++ -I$INCLUDE -Wl,-Bsymbolic-functions -L$LIB -lmpich -lopa -lpthread -lrt -lodbc -fopenmp -O2  -c "
LINK="g++ -I$INCLUDE -L$LIB -fopenmp -lodbc -lmpich -lopa -lpthread -lrt -Wl,-Bsymbolic-functions"

echo Sim
$COMPILE -osim.o sim.cpp
echo World
$COMPILE -oworld.o world.cpp
echo GPS_Math
$COMPILE -ogps_math.o gps_math.cpp
echo Person
$COMPILE -operson.o person.cpp
echo Place
$COMPILE -oplace.o place.cpp
echo DBOpsPar
$COMPILE -oDBOpsPar.o DBOpsPar.cpp
echo Patch
$COMPILE -opatch.o patch.cpp
echo Initialise
$COMPILE -oinitialise.o initialise.cpp
echo RandLib
$COMPILE -orandlib_par.o randlib_par.cpp
echo Intevention
$COMPILE -ointervention.o intervention.cpp
echo Unit
$COMPILE -ounit.o unit.cpp
echo Messages
$COMPILE -omessages.o messages.cpp
echo Params
$COMPILE -oparams.o params.cpp
echo LodePng
$COMPILE -olodepng.o lodepng.cpp
echo Household
$COMPILE -ohousehold.o household.cpp
echo Output
$COMPILE -ooutput.o output.cpp

echo Link

$LINK -oSim world.o unit.o sim.o randlib_par.o place.o person.o patch.o params.o output.o messages.o lodepng.o intervention.o initialise.o household.o gps_math.o DBOpsPar.o

rm *.o
