PARALLEL="1"
LIBS="../../lib/linux"

if [ $PARALLEL -eq "1" ]
then
  echo PARALLEL IS defined
  COMPILE="g++ -fopenmp -O2 -c -fmessage-length=0 "
  LINK="g++ -L$LIBS -fopenmp "
else
  echo PARALLEL is NOT defined
  COMPILE="g++ -O2 -c -fmessage-length=0 "
  LINK="g++ -L$LIBS "
fi

$COMPILE -oItoa.o Itoa.cpp
$COMPILE -oCountry.o Country.cpp
$COMPILE -oEstablishmentDistr.o EstablishmentDistr.cpp
$COMPILE -oEstablishmentStruct.o EstablishmentStruct.cpp
$COMPILE -oHouseholdStruct.o HouseholdStruct.cpp
$COMPILE -oLandScan.o LandScan.cpp
$COMPILE -oMain.o Main.cpp
$COMPILE -oPopulDistr.o PopulDistr.cpp
$COMPILE -oStubs.o Stubs.cpp
$COMPILE -oUtil.o Util.cpp
$COMPILE -oVacancyAndEstablishmentMap.o VacancyAndEstablishmentMap.cpp

if [ $PARALLEL -eq "1" ]
then
  $COMPILE -orandlib_par.o randlib_par/randlib_par.cpp
  $LINK -oSynthPopul Itoa.o Country.o EstablishmentDistr.o EstablishmentStruct.o HouseholdStruct.o LandScan.o PopulDistr.o Stubs.o Util.o VacancyAndEstablishmentMap.o randlib_par.o Main.o
else
  $COMPILE -ocom.o randlib/com.cpp
  $COMPILE -olinpack.o randlib/linpack.cpp
  $COMPILE -orandlib.o randlib/randlib.cpp
  $LINK -oSynthPopul Itoa.o Country.o EstablishmentDistr.o EstablishmentStruct.o HouseholdStruct.o LandScan.o PopulDistr.o Stubs.o Util.o VacancyAndEstablishmentMap.o com.o linpack.o randlib.o Main.o
fi

rm *.o
