cd CombineSynthPopul
compile.sh
chmod 755 combine
cp combine ../../bin-linux/CombineSynthPopul
rm combine
cd ..

cd GetAdminUnits
compile.sh
cp GADM_Shps.class ../../bin-linux/GetAdminUnits
rm GADM_Shps.class
cd ..

cd JobCreator
compile.sh
chmod 755 gsimkit.jar
cp gsimkit.jar ../../bin-linux/JobCreator
rm gsimkit.jar
cd ..

cd MashAdminUnits
compile.sh
chmod 755 mashadmin
cp mashadmin ../../bin-linux/MashAdminUnits
rm mashadmin
cd ..

cd PatchFileMaker
compile.sh
cp *.class ../../bin-linux/PatchFileMaker
rm *.class
cd ..

cd Sim
compile.sh
chmod 755 sim
cp sim ../../bin-linux/Sim
rm sim
cd ..

cd SynthPopul
compile.sh
chmod 755 SynthPopul
cp SynthPopul ../../bin-linux/SynthPopul
rm SynthPopul
cd ..

cd ReadFlatFile
compile.sh
cp *.class ../../bin-linux/ReadFlatFile
rm *.class
cd ..

cd ..

