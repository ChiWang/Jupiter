
#include "DMP_BGO_Classification.cxx"

void Run(){
  gSystem->Load("$ROOTSYS/lib/libTMVA.so");
  DMP_BGO_Classification("BDT,BDTG,BDTB,BDTD,BDTF,MLPBNN,MLP,TMlpANN");
}


