#! /bin/bash

sudo apt update -y
sudo apt install -y cmake
sudo apt install -y build-essential
sudo apt install -y flex bison
sudo apt install -y python2
sudo apt install -y libboost-dev

python2 -m pip install dpkt

git clone https://github.com/eniac/FP4.git

cd $HOME/FP4/instrumentation
rm -rf build && mkdir build && cd build
cmake ..
make
cd ..
mkdir -p sample_in_hw
mkdir -p sample_out
mkdir -p out
cd ..

