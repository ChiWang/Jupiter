
#include "DMP_BGO_Classification.cxx"

void Run(int doit=0){
  gSystem->Load("$ROOTSYS/lib/libTMVA.so");
  gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
  gSystem->Load("$DMPSWWORK/lib/libDmpEvtBgoShower.so");
  if(doit){
    DMP_BGO_Classification("MLP,TMlpANN");
  }
}


