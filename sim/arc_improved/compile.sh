rm out
rm mine
g++ -Wall Chameleon.cpp ConfigFile.cpp sim.cpp -o mine -std=c++11 2> out;
vi out
