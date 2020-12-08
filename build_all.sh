pwd=`pwd`
rm game_server db_Server login_server gate_server

cd ${pwd}/ProtoSrc
sh build.sh

cd ${pwd}/Proto
make clean && make -j8

cd ${pwd}/Common
make clean && make -j8

cd ${pwd}/Package
make clean && make -j8

cd ${pwd}/GameServer
make clean && make -j8

cd ${pwd}/GateServer
make clean && make -j8

cd ${pwd}/LoginServer
make clean && make -j8

cd ${pwd}/DBServer
make clean && make -j8

mv ../Bin/* ../


