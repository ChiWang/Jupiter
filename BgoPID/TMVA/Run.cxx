
#include "DMP_BGO_Classification.cxx"

void Run(){
  gSystem->Load("$ROOTSYS/lib/libTMVA.so");
  DMP_BGO_Classification("MLP,TMlpANN");
}


