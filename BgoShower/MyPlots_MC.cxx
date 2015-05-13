/*
 *  $Id: MyPlots_MC.cxx, 2015-02-11 11:01:09 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 23/01/2015
*/

#include <vector>
#include <iostream>
#include <fstream>

#include "TCut.h"
#include "TCanvas.h"
#include "TH2F.h"
#include "TH3D.h"
#include "TProfile.h"
#include "TChain.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TDirectory.h"
#include "TGraph.h"
//#include "DmpEvtBgoShower.h"
#include "DmpEvtMCTrack.h"

#ifndef MySimuPlot_CXX
#define MySimuPlot_CXX
#define  __treeName "/Event/MCTruth"

namespace Particle{
enum{
  electron = 11,
  positron = -11,
  muon_n = 13,
  muon_p = 13,
  gamma = 22,
  pi0 = 111,
  pi_p = 211,
  pi_n = -211,
};
};

namespace Plot_MC
{
namespace Conf
{
  vector<TCanvas*>  can;
  vector<TString>   inputFileName;

  long nEvts = 0;
  TChain *chain = 0;
  DmpEvtMCTrack *evt_track = new DmpEvtMCTrack();

  TChain *LinkTree()
  {
    if(inputFileName.size() == 0){
      cout<<"\tWARNING:\t do not have any input files"<<endl;
      cout<<"\t\tPlot_MC::ResetInputFile(filename)"<<endl;
      cout<<"\t\tPlot_MC::AddInputFile(filename)\n"<<endl;
      return 0;
    }
    if(chain == 0){
      chain  = new TChain(__treeName);
      for(unsigned int i = 0;i<inputFileName.size();++i){
        chain->AddFile(inputFileName[i]);
      }
      chain->SetBranchAddress("TrackVertex",&evt_track);
      nEvts = chain->GetEntries();
      std::cout<<"===>  entries: "<<nEvts<<std::endl;
    }
    return chain;
  }
};

namespace Steer
{
  void PrintCanvasInfor()
  {
    int n=Conf::can.size();
    for(int i=0;i<n;++i){
      cout<<"i = "<<i<<"\t"<<Conf::can[i]->GetName()<<endl;
    }
  }
  TCanvas *GetCanvas(int index)
  {
    return Conf::can.at(index);
  }
  void ActiveCanvas(int index,int sub0=0,int sub1=0)
  {
    Conf::can.at(index)->cd(sub0)->cd(sub1);
  }
  void ListAllObjects()
  {
    gDirectory->GetList()->Print();
  }
  TObject* Get(TString objectName)
  {
    return gDirectory->Get(objectName);
  }
  TLegend *GetNewLegend()
  {
    TLegend *lg = new TLegend(0.55,0.65,0.66,0.82);
    return lg;
  }

};

namespace Cuts
{
  TCut GlobalCut = "Bgo.T0()";
  TCut VerticalMips = "Bgo.GetFiredBarNumber() == 14 && Bgo.GetPileupRatio() == 0";
  TCut MipsWindow = "Bgo.fTotE > 200 && Bgo.fTotE <450 && Bgo.fLRMS > 3.5 && Bgo.fLRMS < 4.4 && Bgo.GetTotalRMS()>-2 && Bgo.GetTotalRMS()<2";
  TCut Trig3_0000 = "Bgo.Group3_0000(0.2)";
};

void ResetInputFile(TString f)
{
  Conf::inputFileName.clear();
  Conf::inputFileName.push_back(f);
  if(Conf::chain){
    delete Conf::chain;
  }
  gStyle->SetOptStat("ouiRMe");
  Conf::chain = new TChain(__treeName);
  Conf::chain->AddFile(f);
  Conf::chain->SetBranchAddress("TrackVertex",&Conf::evt_track);
  Conf::nEvts = Conf::chain->GetEntries();
std::cout<<"===>  entries: "<<Conf::nEvts<<std::endl;
}

void AddInputFile(TString f)
{
  Conf::inputFileName.push_back(f);
  if(Conf::chain == 0){
    Conf::chain = new TChain(__treeName);
   gStyle->SetOptStat("ouiRMe");
  }
  Conf::chain->AddFile(f);
  if(Conf::evt_track == 0) {
    Conf::chain->SetBranchAddress("TrackVertex",&Conf::evt_track);
  }
  Conf::nEvts = Conf::chain->GetEntries();
std::cout<<"===>  entries: "<<Conf::nEvts<<std::endl;
}

void PrintInputFile()
{
  if(Conf::inputFileName.size()){
    for(unsigned int i =0;i<Conf::inputFileName.size();++i){
      cout<<Conf::inputFileName[i]<<endl;
    }
  }else{
    cout<<"Do NOT have any input files"<<endl;
  }
}

void MyDraw(TString exp, TCut cuts= Cuts::GlobalCut, TString opt="")
{
  if(Conf::inputFileName.size() == 0){
    cout<<"\tWARNING:\t do not have any input files"<<endl;
    cout<<"\t\tPlot_MC::ResetInputFile(filename)"<<endl;
    cout<<"\t\tPlot_MC::AddInputFile(filename)\n"<<endl;
    return;
  }
  if(!opt.Contains("same")){
    TString cName = "c";
            cName +=Conf::can.size();
            cName +="--"+exp;
            TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
            tmp.Remove(0,tmp.Last('/')+1);
            tmp.Remove(tmp.Last('.'),tmp.Length());
            cName +="--"+tmp;

    Conf::can.push_back(new TCanvas(cName,cName));
    Conf::can[Conf::can.size()-1]->cd();
  }
  Conf::LinkTree()->Draw(exp,cuts,opt);
  gPad->SetGrid();
}

void PDGCodeOfVertex(double z_low=-400,double z_high=600,int PDGCode_low=-240,int PDGCode_high = 240)
{
  MyDraw("TrackVertex.fPDGCode:TrackVertex.fPosition.z()",Form("Bgo.TotalEnergy()>10 && TrackVertex.fPosition.z() > %f && TrackVertex.fPosition.z()<%f && TrackVertex.fPDGCode > %d && TrackVertex.fPDGCode < %d",z_low,z_high,-PDGCode_low,PDGCode_high));
}

void ESpecOfPDGCode(int PDGCode = 11, double z_low=-400,double z_high=600)
{
 MyDraw("TrackVertex.fEnergy:TrackVertex.fPosition.z()",Form("Bgo.TotalEnergy()>10 && (TrackVertex.fPDGCode == %d || TrackVertex.fPDGCode == -%d) && TrackVertex.fPosition.z() >%f && TrackVertex.fPosition.z()<%f",PDGCode,0-PDGCode,z_low,z_high));
}



};


#endif

