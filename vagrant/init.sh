#!/usr/bin/env bash

apt-get update

#  Install clang-format
#  This tool is used for formatting our code
apt-get install -y clang-format-3.6

#  Install bison
apt-get install -y bison

#  Install flex
apt-get install -y flex

#  Install doxygen
#  This tool is used to generate html documention of our code
apt-get install -y doxygen

#  Install graphviz to draw graphs in documentation
apt-get install -y graphviz

# Install mscgen to allow us to use message sequence charts in our documentation
apt-get install -y mscgen

# Install dia to allow us to use diagrams in our documentation
apt-get install -y dia

#  Copy bash_profile to the home directory (/home/vagrant/), so that 
#+ the script will run each time we ssh into the virtual machine
cp /vagrant/vagrant/bash_profile /home/vagrant/.bash_profile
