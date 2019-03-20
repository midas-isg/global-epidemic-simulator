@echo off

rem This is the script to generate the synthetic population for one country.

rem 1. Note the appropriate population/countries hdr/flt files must exist in the RequiredData directory.
rem 2. Change the REPO so it points to a network accessible folder containing the Gloabl Sim directory structure.
rem 3. Change CLUSTER_CMD to include other cluster-specific requirements, such as /jobtemplate, 
rem 4. Run generate_synth_popul.bat

set REPO=\\fi--didef2\GLOBALSIM\RELEASE
set CLUSTER_CMD=echo /scheduler:fi--didemrchn

set SCRIPT_ROOT=%REPO%\bin-w64\SynthPopul\scripts
set EXE_PATH=%REPO%\bin-w64\SynthPopul\SynthPopul.exe
set POP_HDR=%REPO%\data\RequiredData\population.hdr
set POP_FLT=%REPO%\data\RequiredData\population.flt
set CTR_HDR=%REPO%\data\AdminUnits\cbr.hdr
set CTR_FLT=%REPO%\data\AdminUnits\cbr.flt
set POP_OUT=%REPO%\data\SyntheticPopulation
set DATA_IN=%REPO%\data\SyntheticPopulation\params

call %CLUSTER_CMD% %SCRIPT_ROOT%\%1\%2\run__SynthPopul__%2.bat %EXE_PATH% %POP_HDR% %POP_FLT% %CTR_HDR% %CTR_FLT% %POP_OUT%\popul__%2 %POP_OUT%\est__%2 %DATA_IN%\%1\%2 %SCRIPT_ROOT%\%1\%2