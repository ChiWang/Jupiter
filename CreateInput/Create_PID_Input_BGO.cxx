#include <iostream>

#include "TFile.h"
#include "TTree.h"

#include "DmpEvtBgoHits.h"
#include "DmpEvtBgoPIDVar.h"
#include "DmpBgoBase.h"

#ifndef PID_Input_BGO_C
#define PID_Input_BGO_C

#define InputTree "/Event/Rec0"

namespace PID{
namespace Bgo{

        /*
short GetLayerID(const short &gid){
  return ((gid>>11) &0x000f);
}

short GetBarID(const short &gid){
  return ((gid>>6) &0x001f);
}
*/

  namespace PATH{
    TString  input = "./Data/";
  };

  bool CreateInput(TString file_Rec0="Rec0_H0-ANC_209_DMP_20141106_141222.root"){
    TFile *input_f = TFile::Open(PATH::input+file_Rec0);
    if(input_f == 0){
      cout<<"open file failed...\t"<<PATH::input+file_Rec0<<endl;
      return false;
    }
    TTree *tree_i = (TTree*)(input_f->Get(InputTree));
    DmpEvtBgoHits *event_bgo = new DmpEvtBgoHits();
    tree_i->SetBranchAddress("Bgo",&event_bgo);

std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
    TString outName = file_Rec0;
    outName.Replace(0,7,"PID_I_BGO");
    TFile *output_f = new TFile(PATH::input+outName,"RECREATE");
    output_f->mkdir("Event");
    TTree *tree_o = new TTree("PID","PID");
    DmpEvtBgoPIDVar *PID_v_bgo = new DmpEvtBgoPIDVar(); 
    tree_o->Branch("Bgo",PID_v_bgo->GetName(),&PID_v_bgo);

    long et = tree_i->GetEntries();
    for(long ievt = 0;ievt<et;++ievt){
      tree_i->GetEntry(ievt);
      for(unsigned short ib =0;ib<event_bgo->fGlobalBarID.size();++ib){
        short id = event_bgo->fGlobalBarID[ib];
        int lid = DmpBgoBase::GetLayerID(id);
        if(lid >13){
std::cout<<"xxx__DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
        }
        int bid =DmpBgoBase::GetBarID(id);
        (PID_v_bgo->fE_LB[lid]).insert(std::make_pair(bid,event_bgo->fEnergy[ib]));
      }
      for(int il = 0;il<BGO_LayerNO;++il){
        PID_v_bgo->fRMS2[il] = event_bgo->RMS2(il);
        PID_v_bgo->fFValue[il] = event_bgo->FValue(il);
      }
      PID_v_bgo->fTotalE = event_bgo->TotalEnergy();
      PID_v_bgo->fPre_Direction = event_bgo->PositionInPlane(1) - event_bgo->PositionInPlane(0);   //toy dagta
      tree_o->Fill();
      PID_v_bgo->Reset();
    }
std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
    output_f->cd("Event");
    tree_o->Write("",TObject::kOverwrite);
    output_f->Close();

    delete event_bgo;
    delete PID_v_bgo;
    delete output_f;
    //delete tree_o;
    return true;
  }
};
};


#endif


