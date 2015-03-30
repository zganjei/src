# Reachability Analyzer

prerequisites:

install gcc 4.7 :

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-4.7

set default gcc4.7:

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.6 
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.7 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.7 
sudo update-alternatives --config gcc

###################
get g++ 4.7 
sudo add-apt-repository ppa:ubuntu-toolchain-r/test

Then, to install it use

sudo apt-get update
sudo apt-get install g++-4.7

To change the default compiler use update-alternatives

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.6
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.7 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.7
sudo update-alternatives --config gcc


###################################
intall cmake,subversion, boost, gmp,flex, bison,
install ppl (download and then follow its INSTALL file for installation) 
then in /usr/local/include/ppl.hh look for mp_size_field_t and in it's declaration, 
change typedef typeof to typedef decltype

download z3,unzip, copy include/* and bin/* to usr/include and usr/bin

download mathsat,unzip, copy include/* and li/* to usr/include and usr/lib

install cplex(download the binary file and execute it.note that it's licensed)

-----------------------------------------------------------
OPTIONAL:

install jre/jdk in case of using eclipse for development e.g.
[[
sudo add-apt-repository ppa:webupd8team/java
sudo apt-get update
sudo apt-get install oracle-java7-installer
]]

intall eclipse cdt

sudo apt-get install eclipse eclipse-cdt g++

then
sudo update-alternatives --config java

This brings up a list of the different types of Java. Simply select the Open JDK.

---------------------------------------------------------------------

COMPILING:

1. cd to directory containing src-patch1/
2. mkdir build; cd build
3. cmake ../src-patch1
4. make
5. now you can see the directory build/zaama-v2, cd zaama-v2/
6. to run a test in zaama: ./zaama example-input.txt
