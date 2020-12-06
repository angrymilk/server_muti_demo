pwd=`pwd`
cd ${pwd}/ProtoSrc
sh build.sh

cd ${pwd}/Proto
make clean && make

cd ${pwd}/Common
make clean && make

cd ${pwd}/Package
make clean && make

cd ${pwd}/GameServer
make clean && make

cd ${pwd}/GateServer
make clean && make

cd ${pwd}LoginServer
make clean && make

cd ${pwd}DBServer
make clean && make

mv ../Bin/* ../


