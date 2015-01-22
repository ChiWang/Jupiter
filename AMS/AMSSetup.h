/*
 *  $Id: AMSSetup.h, 2015-01-02 21:46:46 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 01/01/2015
*/

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include "TFile.h"
#include "TTree.h"
//#include "TString.h"

#include "DmpEvtBTAnc.h"

#ifndef AMSSetup_H
#define AMSSetup_H

#define NLadder     5
#define s_side      0
#define k_side      1
#define NLocation   2
#define NSide       2

#define XAxis       s_side
#define YAxis       k_side
#define ZAxis       2
#define Vertical    XAxis
#define Horizaontal YAxis
#define BeamDirction ZAxis

#define AMSTreeName "Event/Rec0"
#define AMSBranchName "Anc"

namespace AMS{
//using namespace std;
namespace Conf{
  int       SensorsNumber[NLadder] = {4,4,4,12,12};  // same for PS and SPS. from upstream to downstream
  int       FirstSide[NLocation] = {s_side,k_side};   // @ PS, s side at upstream. @ SPS k side at upstream
  float     Pitch[NSide] = {0.011,0.0208};      // unit cm. first is s side, then k side
  float     SensorGap_kSide = 0.1392 + 0;       // unit cm.     1392 = 676*2 + 40 (um);  40: sensor distance,  676: active area to edge of sensor
  float     LadderDistance[NLocation][NLadder-1] = {
            {119.5,   112,    45,     95},      // PS
            {113.3,   761.1,  39.0,   96.5}};   // SPS.     unit cm. beam --->

namespace EnableCut{
  bool TotSN_Cut = true;
  bool Length_Cut = false;
  bool Cut_2 = false;
  bool Cut_3 = false;
  bool Cut_4 = false;
  bool Cut_5 = false;
  bool Cut_6 = false;
};
};
};

// common functions
namespace AMS{
namespace Common{
  enum Location{
    PS = 0,
    SPS = 1,
  };
  short     ExHall = SPS;// or PS
  TString  _file = "NO";     // file includes 5 ladders data
  TTree    *_tree = 0;
  DmpEvtBTAnc    *_evt_AMS = 0;
  long     _entries = -1;
  long     _evtID = -1;
  float    _offset[NLocation][NLadder][NSide+1] = {{{0}}};
  namespace PATH{
    TString  _input = "./Rec0/ALL/";
    TString  _alignment = "./Alignment/"; 
  };

  bool UpdateOffset(TString filename = "Default"){
    if(filename == "Default"){
      filename = "alignment_PS.txt";
      if(ExHall == SPS){
        filename = "alignment_SPS.txt";
      }
    }
    ifstream f(PATH::_alignment+filename);
    if(!f.good()){
      cout<<"open "<<filename<<"\tfailed..."<<endl;
      return false;
    }else{
      cout<<"loading new alignment parameter from file: "<<filename<<endl;
      char tmp[256]="x";
      f.getline(tmp,256);
      for(int i=0;i<NLadder;++i){
        f.getline(tmp,256);
        istringstream par(tmp);
        int tmp =-1;
        par>>tmp;
        par>>_offset[ExHall][i][XAxis]>>tmp>>_offset[ExHall][i][YAxis]>>tmp>>_offset[ExHall][i][ZAxis];    // not use error at here.
      }
    }
    return true;
  }

  void PrintCutStatus(){
// *
// *  TODO: 
// *
  }

bool LoadInput(TString fName){
  _file = fName;
  //gStyle->SetOptStat(11111111);
  //gStyle->SetOptFit(111111111);
  TFile *f = TFile::Open(PATH::_input+_file,"READ");
  if(!f){
    return false;
  }
  //sss
  _tree = (TTree*)(f->Get(AMSTreeName));
  _entries = _tree->GetEntries();
  if(_evt_AMS){
    delete _evt_AMS;
    _evt_AMS = 0;
  }
  _evt_AMS = new DmpEvtBTAnc();
  _tree->SetBranchAddress(AMSBranchName,&_evt_AMS);
  return true;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void LoadEvent(){
  _tree->GetEntry(_evtID);
}

//-------------------------------------------------------------------
bool GoodClusterCheck(Cluster *aC){
  if(Conf::EnableCut::TotSN_Cut){
    if(aC->GetTotSN()< 3.8 || aC->GetTotSN()>14.0){
      return false;
    }
  }
  if(Conf::EnableCut::Length_Cut){
// *
// *  TODO:  check cut value
// *
    if(aC->GetLength()< 1 || aC->GetTotSN()>10){
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------
std::vector<int> ClusterNumberInLadder(bool choosingGoodCluster=true){
  std::vector<int> r;
  r.resize(NLadder*2,0);
  int n_cls = _evt_AMS->fAMSCls->GetEntriesFast();
  for(short ic=0;ic<n_cls;++ic){
    Cluster *aCluster = dynamic_cast<Cluster*>(_evt_AMS->fAMSCls->At(ic));
    if(choosingGoodCluster){
      if(! GoodClusterCheck(aCluster)){
        continue;
      }
    }
    ++r[aCluster->ladder*2 + aCluster->side];
  }
  return r;
}

//-------------------------------------------------------------------
bool N_ClustersInLadder_I(int N, short I, bool OnlyGoodCluster=true){
  // both sides has n clusters
  vector<int> clusBN = ClusterNumberInLadder(OnlyGoodCluster);
  if(clusBN[I*2+0] !=N || clusBN[I*2+1] != N){
    return false;
  }
  return true;
}

//-------------------------------------------------------------------
bool ClusterNumberLessThan2_forAllS_Side(){
  vector<int> clusBN = ClusterNumberInLadder();
  for(short id=1;id<NLadder;++id){
    if(clusBN[id*2+0]>1){
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------
float GetPosition(Cluster *aCluster, bool includeOffset=true){
        /*
         *  include offset defautly,
         *
         *  but for alignment, must set the flag false
         *
         */
  float posi=0.0;
  short side = aCluster->side;
  float CoG = aCluster->GetCoG();
  if(side == 0){    // s-side
    posi = CoG * Conf::Pitch[side];
  }else{    // k-side
    CoG = CoG-640;
    posi = CoG*Conf::Pitch[side] + (CoG<192?0:Conf::SensorGap_kSide);
  }
  return posi + (includeOffset ? _offset[ExHall][aCluster->ladder][side] : 0);
}

};
};

#endif

