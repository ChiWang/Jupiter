/*
 *  $Id: MyPlots.cxx, 2015-01-23 13:58:40 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 23/01/2015
*/

#include <vector>
#include <iostream>
#include <fstream>

#include "TCut.h"
#include "TCanvas.h"
#include "TH2F.h"
#include "TChain.h"
#include "TStyle.h"
#include "DmpEvtBgoShower.h"

#ifndef MyPlot_CXX
#define MyPlot_CXX
#define  __treeName "/Event/Rec0"

namespace Cuts
{
  TCut GlobalCut = "Bgo.fTotE > 0";
  TCut VerticalMips = "Bgo.GetFiredBarNumber() == 14 && Bgo.GetPileupRatio() == 0";
};

namespace Conf
{
  vector<TCanvas*>  can;
  vector<TString>   inputFileName;
  TChain *chain = 0;

  DmpEvtBgoShower *evt_bgo = 0;
  long nEvts = 0;
};

void PrintInputFile()
{
  for(unsigned int i =0;i<Conf::inputFileName.size();++i){
    cout<<Conf::inputFileName[i]<<endl;
  }
}

void AddInputFile(TString f)
{
  Conf::inputFileName.push_back(f);
  if(Conf::chain == 0){
    Conf::chain = new TChain(__treeName);
  }
  Conf::chain->AddFile(f);
  if(Conf::evt_bgo == 0) {
    Conf::chain->SetBranchAddress("Bgo",&Conf::evt_bgo);
  }
  Conf::nEvts = Conf::chain->GetEntries();
std::cout<<"===>: entries: "<<Conf::nEvts<<std::endl;
}

void ResetInputFile(TString f)
{
  Conf::inputFileName.clear();
  Conf::inputFileName.push_back(f);
  if(Conf::chain){
    delete Conf::chain;
  }
  Conf::chain = new TChain(__treeName);
  Conf::chain->AddFile(f);
  Conf::chain->SetBranchAddress("Bgo",&Conf::evt_bgo);
  Conf::nEvts = Conf::chain->GetEntries();
std::cout<<"===>: entries: "<<Conf::nEvts<<std::endl;
}

TChain *LinkTree()
{
  if(Conf::chain == 0 && Conf::inputFileName.size()){
    Conf::chain  = new TChain(__treeName);
    for(unsigned int i = 0;i<Conf::inputFileName.size();++i){
      Conf::chain->AddFile(Conf::inputFileName[i]);
    }
    Conf::chain->SetBranchAddress("Bgo",&Conf::evt_bgo);
    Conf::nEvts = Conf::chain->GetEntries();
std::cout<<"===>: entries: "<<Conf::nEvts<<std::endl;
  }
  return Conf::chain;
}

void MyDraw(TString exp, TCut cuts= Cuts::GlobalCut, TString opt="")
{
  TString cName = "c";
          cName +=Conf::can.size();
          cName +="--"+exp;

  Conf::can.push_back(new TCanvas(cName,cName));
  Conf::can[Conf::can.size()-1]->cd();

  LinkTree()->Draw(exp,cuts,opt);
}

namespace Plot
{
void EnergySpectrum(int layer=-1, int barID=-1, TCut cuts=Cuts::GlobalCut)
{
        // (-1,-1) total. (layer, -1) each bar in this layer. (-1,bar) this barID in each layer
  if(layer < 0 && barID < 0){
    MyDraw("Bgo.fTotE",cuts);
    gPad->SetGrid();
    MyDraw("Bgo.fTotE:EventHeader.fDeltaTime",cuts,"colz");
    gPad->SetGrid();
  }else if(layer < 0 && barID >= 0){
    TString name = "c";
    name += Conf::can.size();
    name += "--Bar ";
    name += barID;
    name += " for each layer";
    Conf::can.push_back(new TCanvas(name,name));
    TCanvas *c0 = Conf::can[Conf::can.size()-1];
    c0->Divide(2,7);
    for(int i=0;i<14;++i){
      c0->cd(i+1);
      name = "Bgo.GetEnergyOfBar(";
      name +=i;
      name +=",";
      name +=barID;
      name +=")";
      LinkTree()->Draw(name,cuts);
    }
  }else if(layer >=0 && barID < 0){
    TString name = "c";
    name += Conf::can.size();
    name += "--Each bar in layer";
    name += layer;
    Conf::can.push_back(new TCanvas(name,name));
    TCanvas *c0 = Conf::can[Conf::can.size()-1];
    c0->Divide(4,6);
    for(int i=0;i<22;++i){
      c0->cd(i+1);
      name = "Bgo.GetEnergyOfBar(";
      name +=layer;
      name +=",";
      name +=i;
      name +=")";
      LinkTree()->Draw(name,cuts);
    }
    c0->cd(23);
    name = "Bgo.GetTotalEnergy(";
    name += layer;
    name += ")";
    LinkTree()->Draw(name,cuts);
    c0->cd(24);
    name = "Bgo.GetMaxClusterInLayer(";
    name += layer;
    name += ")->fSeedBarID";
    LinkTree()->Draw(name,cuts);
  }else{
    TString ex= "Bgo.GetEnergyOfBar(";
    ex +=layer;
    ex +=",";
    ex +=barID;
    ex +=")";
    MyDraw(ex,cuts);
  }
}

void Direction(TCut cuts=Cuts::GlobalCut)
{
  gStyle->SetStatStyle();
  MyDraw("Bgo.GetTrackDirection().Theta()",cuts);
  gPad->SetGridx();
  MyDraw("Bgo.GetTrackDirection().Theta():Bgo.GetMaxEnergyLayerID()",cuts,"*");
  gPad->SetGrid();
  MyDraw("Bgo.GetTrackDirection().Theta():Bgo.fTotE",cuts,"*");
  gPad->SetGrid();
}

void EntryPoint(TCut cuts = Cuts::VerticalMips)
{
  MyDraw("Bgo.GetEntryPoint().y():Bgo.GetEntryPoint().x()",cuts,"colz");
  gPad->SetGrid();
}

void EntryBarID(TCut cuts = Cuts::VerticalMips)
{
  MyDraw("Bgo.GetCoGBarIDInLayer(0):Bgo.GetCoGBarIDInLayer(1)",cuts,"colz");
  gPad->SetGrid();
}

void MaxEnergyLayer(TCut cuts = Cuts::GlobalCut)
{
  MyDraw("Bgo.GetMaxEnergyLayerID()",cuts);
  gPad->SetGridx();
}

void DrawEvent(long evtID)
{
  TString name = "EventID_";
          name += evtID;
  TH2F *xz =  new TH2F("XZ_"+name,"XZ",14,0,14,22,0,22);  xz->GetXaxis()->SetTitle("layer ID");   xz->GetYaxis()->SetTitle("bar ID");
  TH2F *yz =  new TH2F("YZ_"+name,"YZ",14,0,14,22,0,22);  yz->GetXaxis()->SetTitle("layer ID");   yz->GetYaxis()->SetTitle("bar ID");

  LinkTree()->GetEntry(evtID);

  cout<<"\n\n\tEvent ID "<<evtID<<endl;
  Conf::evt_bgo->MyPrint();

  for(int ic = 0;ic < Conf::evt_bgo->fClusters->GetEntriesFast();++ic){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(Conf::evt_bgo->fClusters->At(ic));
    int lid = aC->fLayer;
    for(int b = 0;b<aC->fFiredBar->GetEntriesFast();++b){
      DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(aC->fFiredBar->At(b));
      if(lid % 2 == 0){
        yz->Fill(lid,aB->fBar,aB->fE/Conf::evt_bgo->fTotE);
      }else{
        xz->Fill(lid,aB->fBar,aB->fE/Conf::evt_bgo->fTotE);
      }
    }
  }

  Conf::can.push_back(new TCanvas("Display_"+name,"Display_"+name));
  TCanvas *c0 = Conf::can[Conf::can.size()-1];
  c0->Divide(2,1);
  c0->cd(1); gPad->SetGrid(); xz->Draw("colz");
  c0->cd(2); gPad->SetGrid(); yz->Draw("colz");
}

//-------------------------------------------------------------------
void DrawEventByEnergy(double e0,double e1)
{
  for(long i=0;i<Conf::nEvts;++i){
    LinkTree()->GetEntry(i);
    if(Conf::evt_bgo->fTotE >= e0 && Conf::evt_bgo->fTotE <= e1){
      DrawEvent(i);
    }
  }
}

};

#endif



