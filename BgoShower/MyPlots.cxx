/*
 *  $Id: MyPlots.cxx, 2015-01-23 14:23:31 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 23/01/2015
*/

#include <vector>
#include <iostream>
#include <fstream>

#include "TCut.h"
#include "TCanvas.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TChain.h"
#include "TStyle.h"
#include "DmpEvtBgoShower.h"

#ifndef MyPlot_CXX
#define MyPlot_CXX
#define  __treeName "/Event/Rec0"

namespace Plot
{
namespace Conf
{ 
  vector<TCanvas*>  can;
  vector<TString>   inputFileName;

  long nEvts = 0;
  TChain *chain = 0;
  DmpEvtBgoShower *evt_bgo = new DmpEvtBgoShower();

  TChain *LinkTree()
  {
    if(chain == 0 && inputFileName.size()){
      chain  = new TChain(__treeName);
      for(unsigned int i = 0;i<inputFileName.size();++i){
        chain->AddFile(inputFileName[i]);
      }
      chain->SetBranchAddress("Bgo",&evt_bgo);
      nEvts = chain->GetEntries();
      std::cout<<"===>  entries: "<<nEvts<<std::endl;
    }
    return chain;
  }
  void PrintCanvasInfor()
  {
    int n=can.size();
    for(int i=0;i<n;++i){
      cout<<"i = "<<i<<"\t"<<can[i]->GetName()<<endl;
    }
  }
  TCanvas *GetCanvas(int index)
  {
    return can.at(index);
  }
  void ActiveCanvas(int index)
  {
    can.at(index)->cd();
  }

};

namespace Cuts
{
  TCut GlobalCut = "Bgo.fTotE > 0";
  TCut VerticalMips = "Bgo.GetFiredBarNumber() == 14 && Bgo.GetPileupRatio() == 0";
  TCut MipsWindow = "Bgo.fTotE > 200 && Bgo.fTotE <450 && Bgo.fLRMS > 3.5 && Bgo.fLRMS < 4.4 && Bgo.GetTotalRMS()>-2 && Bgo.GetTotalRMS()<2";
};

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
std::cout<<"===>  entries: "<<Conf::nEvts<<std::endl;
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
    cout<<"\t\tPlot::ResetInputFile(filename)"<<endl;
    cout<<"\t\tPlot::AddInputFile(filename)\n"<<endl;
    return;
  }
  if(!opt.Contains("same")){
    TString cName = "c";
            cName +=Conf::can.size();
            cName +="--"+exp;

    Conf::can.push_back(new TCanvas(cName,cName));
    Conf::can[Conf::can.size()-1]->cd();
  }
  Conf::LinkTree()->Draw(exp,cuts,opt);
  gPad->SetGrid();
}

// *
// *  TODO:  how to use TCuts as TTree::Draw() ?? 
// *
void EnergyProfile(bool skipMips = true,TCut cuts=Cuts::GlobalCut)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="Energy_In_Layer";

    TH2D *eMaxInL = new TH2D(cName+"MaxE","EMax In Layer",14,0,14,500,0,8000);
    TH2D *eInL = new TH2D(cName,"Energy In Layer",14,0,14,500,0,8000);
    for(int ievt=0;ievt<Conf::nEvts;++ievt){
      Conf::chain->GetEntry(ievt);
      if(Conf::evt_bgo->GetFiredBarNumber() == 14 && Conf::evt_bgo->GetPileupRatio() == 0 && skipMips){
        continue;
      }
      eMaxInL->Fill(Conf::evt_bgo->GetLayerIDOfMaxE(),Conf::evt_bgo->GetEnergyOfEMaxLayer());
      for(int il=0;il<BGO_LayerNO;++il){
        eInL->Fill(il,Conf::evt_bgo->GetTotalEnergy(il));
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    TCanvas *c = Conf::can[Conf::can.size()-1];

    c->cd(2)->Divide(1,2,0,0);
    c->cd(2);
    gPad->SetGrid();
    eMaxInL->SetMarkerStyle(6);
    eMaxInL->Draw();
    TProfile *eMaxInL_proX = eMaxInL->ProfileX();
    eMaxInL_proX->SetMarkerStyle(30);
    eMaxInL_proX->SetMarkerSize(2);
    eMaxInL_proX->SetMarkerColor(2);
    eMaxInL_proX->SetLineWidth(2);
    eMaxInL_proX->SetLineColor(2);
    eMaxInL_proX->Draw("esame");
    c->cd(1);
    gPad->SetGrid();
    eInL->SetMarkerStyle(6);
    eInL->Draw();
    TProfile *eInL_proX = eInL->ProfileX();
    eInL_proX->SetMarkerStyle(30);
    eInL_proX->SetMarkerSize(2);
    eInL_proX->SetMarkerColor(2);
    eInL_proX->SetLineWidth(2);
    eInL_proX->SetLineColor(2);
    eInL_proX->Draw("samee");
}

//-------------------------------------------------------------------
TProfile *RMSFValueProfile(bool skipMips = true ,TCut cuts=Cuts::GlobalCut)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="RF_In_Layer";

    TH2D *RMSInL = new TH2D(cName+"RMS","RMS In Layer",14,0,14,500,0,1);
    TH2D *FInL = new TH2D(cName+"FValue","FValue In Layer",14,0,14,500,0,10);
    TH2D *RFRatioInL = new TH2D(cName+"RFRatioValue","RFRatio In Layer",14,0,14,5000,0,100);
    for(int ievt=0;ievt<Conf::nEvts;++ievt){
      Conf::chain->GetEntry(ievt);
      if(Conf::evt_bgo->GetFiredBarNumber() == 14 && Conf::evt_bgo->GetPileupRatio() == 0 && skipMips){
        continue;
      }
      double totRMS = Conf::evt_bgo->GetTotalRMS();
      for(int i=0;i<BGO_LayerNO;++i){
        RMSInL->Fill(i,Conf::evt_bgo->fRMS[i]/totRMS);
        FInL->Fill(i,Conf::evt_bgo->fFValue[i]);
        RFRatioInL->Fill(i,Conf::evt_bgo->GetRFRatio(i));
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    TCanvas *c = Conf::can[Conf::can.size()-1];

    c->Divide(1,3,0,0);
    c->cd(1);
    gPad->SetGrid();
    RMSInL->SetMarkerStyle(6);
    RMSInL->Draw();
    TProfile *RMSInL_proX = RMSInL->ProfileX();
    RMSInL_proX->SetMarkerStyle(30);
    RMSInL_proX->SetMarkerSize(2);
    RMSInL_proX->SetMarkerColor(2);
    RMSInL_proX->SetLineWidth(2);
    RMSInL_proX->SetLineColor(2);
    RMSInL_proX->Draw("esame");
    c->cd(2);
    gPad->SetGrid();
    FInL->SetMarkerStyle(6);
    FInL->Draw();
    TProfile *FInL_proX = FInL->ProfileX();
    FInL_proX->SetMarkerStyle(30);
    FInL_proX->SetMarkerSize(2);
    FInL_proX->SetMarkerColor(2);
    FInL_proX->SetLineWidth(2);
    FInL_proX->SetLineColor(2);
    FInL_proX->Draw("samee");
    c->cd(3);
    gPad->SetGrid();
    RFRatioInL->SetMarkerStyle(6);
    RFRatioInL->Draw();
    TProfile *RFRatioInL_proX = RFRatioInL->ProfileX();
    RFRatioInL_proX->SetMarkerStyle(30);
    RFRatioInL_proX->SetMarkerSize(2);
    RFRatioInL_proX->SetMarkerColor(2);
    RFRatioInL_proX->SetLineWidth(2);
    RFRatioInL_proX->SetLineColor(2);
    RFRatioInL_proX->Draw("samee");
    return RMSInL_proX;
}

void EnergySpectrum(int layer=-1, int barID=-1, TCut cuts=Cuts::GlobalCut)
{
        // (-1,-1) total. (layer, -1) each bar in this layer. (-1,bar) this barID in each layer
  if(layer < 0 && barID < 0){
    MyDraw("Bgo.fTotE",cuts);
    MyDraw("Bgo.fTotE:EventHeader.fDeltaTime",cuts,"colz");
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
      Conf::LinkTree()->Draw(name,cuts);
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
      Conf::LinkTree()->Draw(name,cuts);
    }
    c0->cd(23);
    name = "Bgo.GetTotalEnergy(";
    name += layer;
    name += ")";
    Conf::LinkTree()->Draw(name,cuts);
    c0->cd(24);
    name = "Bgo.GetMaxClusterInLayer(";
    name += layer;
    name += ")->fSeedBarID";
    Conf::LinkTree()->Draw(name,cuts);
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
  MyDraw("Bgo.GetTrackDirection().Theta():Bgo.GetLayerIDOfMaxE()",cuts,"*");
  MyDraw("Bgo.GetTrackDirection().Theta():Bgo.fTotE",cuts,"*");
}

void EntryPoint(TCut cuts = Cuts::VerticalMips)
{
  MyDraw("Bgo.GetEntryPoint().y():Bgo.GetEntryPoint().x()",cuts,"colz");
}

void EntryBarID(TCut cuts = Cuts::VerticalMips)
{
  MyDraw("Bgo.GetCoGBarIDInLayer(0):Bgo.GetCoGBarIDInLayer(1)",cuts,"colz");
}

//void // RFRatio by layer TH2D
// RMS value by layer,  RMS2RMSTotal by layer

void DrawThisEvent(long evtID)
{
  TString name = "EventID_";
          name += evtID;
  TH2F *xz =  new TH2F("XZ_"+name,"XZ",14,0,14,22,0,22);  xz->GetXaxis()->SetTitle("layer ID");   xz->GetYaxis()->SetTitle("bar ID");
  TH2F *yz =  new TH2F("YZ_"+name,"YZ",14,0,14,22,0,22);  yz->GetXaxis()->SetTitle("layer ID");   yz->GetYaxis()->SetTitle("bar ID");
  TH2D *eInL = new TH2D("energy profile"+name,"Energy In Layer",14,0,14,100,0,0.5);
  TH2D *RMSInL = new TH2D("RMS in layer"+name,"RMS In Layer",14,0,14,100,0,0.5);

  Conf::LinkTree()->GetEntry(evtID);

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
  for(int il=0;il<BGO_LayerNO;++il){
    eInL->Fill(il,Conf::evt_bgo->GetTotalEnergy(il)/Conf::evt_bgo->fTotE);
    RMSInL->Fill(il,Conf::evt_bgo->fRMS[il]/Conf::evt_bgo->GetTotalRMS());
  }

  Conf::can.push_back(new TCanvas("Display_"+name,"Display_"+name));
  TCanvas *c0 = Conf::can[Conf::can.size()-1];
  c0->Divide(2,2);
  c0->cd(1); gPad->SetGrid(); xz->Draw("colz");
  c0->cd(2); gPad->SetGrid(); yz->Draw("colz");
  c0->cd(3); gPad->SetGrid(); eInL->SetMarkerStyle(30);  eInL->Draw();
  c0->cd(4); gPad->SetGrid(); RMSInL->SetMarkerStyle(27);  RMSInL->Draw();
}

void DrawEventByEnergy(double e0,double e1)
{
  for(long i=0;i<Conf::nEvts;++i){
    Conf::LinkTree()->GetEntry(i);
    if(Conf::evt_bgo->fTotE >= e0 && Conf::evt_bgo->fTotE <= e1){
      DrawThisEvent(i);
    }
  }
}

void DrawEventByCondition(bool con)
{
// *
// *  TODO:  not finish...
//              How does root recongnize TCut which is a string??
// *

  for(long i=0;i<Conf::nEvts;++i){
    Conf::LinkTree()->GetEntry(i);
    if(con){
      DrawThisEvent(i);
    }
  }
}

};

#endif



