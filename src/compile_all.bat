@echo off

cd CombineSynthPopul
call compile.bat
copy combine.exe ..\..\bin-w64\CombineSynthPopul /y
del combine.exe
cd ..


cd GetAdminUnits
call compile.bat
copy GADM_Shps.class ..\..\bin-w64\GetAdminUnits /y
del GADM_Shps.class
cd ..

cd JobCreator
call compile.bat
copy gsimkit.jar ..\..\bin-w64\JobCreator /y
del gsimkit.jar
cd ..

cd MashAdminUnits
call compile.bat
copy mashadmin.exe ..\..\bin-w64\MashAdminUnits /y
del mashadmin.exe
cd ..

cd PatchFileMaker
call compile.bat
copy *.class ..\..\bin-w64\PatchFileMaker /y
del *.class
cd ..

cd Sim
call compile.bat
copy Sim.exe ..\..\bin-w64\Sim /y
del Sim.exe
cd ..

cd SynthPopul
call compile.bat
copy SynthPopul.exe ..\..\bin-w64\SynthPopul /y
del SynthPopul.exe
cd ..

cd ReadFlatFile
call compile.bat
copy *.class ..\..\bin-w64\ReadFlatFile /y
del *.class
cd ..
cd ..

