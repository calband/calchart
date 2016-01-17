#!/usr/bin/env bash

apt-get update

apt-get install -y bison
apt-get install -y flex

sudo apt-get install -y clang-format-3.6

#  Copy bash_profile to the home directory (/home/vagrant/), so that 
#+ the script will run each time we ssh into the virtual machine
cp /vagrant/vagrant/bash_profile /home/vagrant/.bash_profile
