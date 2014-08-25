sudo killall RfMeshMaster
sudo killall node
sudo service mongodb stop
sudo rm /var/lib/mongodb/mongod.lock
rm -rf ../log
mkdir -p ../log
sudo service mongodb start
cd out
nohup sudo ./RfMeshMaster >> ../../log/RfMeshMaster.log 2>&1 &
cd ../../web
nohup node RfMeshWeb.js >> ../log/RfMeshWeb.log 2>&1 &
tail -f ../log/RfMeshMaster.log
cd ../master
