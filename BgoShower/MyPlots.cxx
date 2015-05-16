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
#include "TH3D.h"
#include "TProfile.h"
#include "TChain.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TDirectory.h"
#include "TGraph.h"
#include "TMath.h"
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
    if(inputFileName.size() == 0){
      cout<<"\tWARNING:\t do not have any input files"<<endl;
      cout<<"\t\tPlot::ResetInputFile(filename)"<<endl;
      cout<<"\t\tPlot::AddInputFile(filename)\n"<<endl;
      return 0;
    }
    if(chain == 0){
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
  TCut VerticalMips = "Bgo.GetFiredBarNumber() < 16 && Bgo.ClusterNo(10) < 16";
  bool IsVerticalMips(){
    return (Conf::evt_bgo->GetFiredBarNumber() < 16 && Conf::evt_bgo->GetClusterNo(10) <16);
  }
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
  Conf::chain->SetBranchAddress("Bgo",&Conf::evt_bgo);
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

// *
// *  TODO:  how to use TCuts as TTree::Draw() ?? 
// *
void LongitudinalProfile(bool skipMips = true,TCut cuts=Cuts::GlobalCut,TString opt="profg")
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="Energy_In_Layer";
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *eInL = new TH2D(cName,"E_layer / E_total",14,0,14,5000,0,0.5);
    for(int ievt=0;ievt<Conf::nEvts;++ievt){
      Conf::chain->GetEntry(ievt);
      if(Cuts::IsVerticalMips() && skipMips){
        continue;
      }
      for(int il=0;il<BGO_LayerNO;++il){
        eInL->Fill(il,Conf::evt_bgo->GetTotalEnergy(il)/ Conf::evt_bgo->fTotE);
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    TCanvas *c = Conf::can[Conf::can.size()-1];

    c->cd(2)->Divide(1,3,0,0);
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
    c->cd(2);
    gPad->SetGrid();
    Conf::LinkTree()->Draw("Bgo.GetEnergyOfEMaxLayer():Bgo.GetLayerIDOfMaxE()",cuts,opt);
    c->cd(3);
    gPad->SetGrid();
    Conf::LinkTree()->Draw("Bgo.GetEnergyOfEMaxLayer()/Bgo.fTotE : Bgo.GetLayerIDOfMaxE()",cuts,opt);
}

void TransverseProfile()
{
  TString cName = "c";
         cName +=Conf::can.size();
         cName +="Transverse Profile";
         TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
         tmp.Remove(0,tmp.Last('/')+1);
         tmp.Remove(tmp.Last('.'),tmp.Length());
         cName +="--"+tmp;

  TH2D*  transInL[BGO_LayerNO];
  for(int i=0;i<BGO_LayerNO;++i){
    transInL[i] = new TH2D(Form("layer %d",i)+cName,Form("layer %d;BarID;EnergyRotaio",i),22,0,22,1100,0,1.1);
    //transInL[i]->SetMarkerStyle(20);
    //transInL[i]->SetMarkerSize(2);
    transInL[i]->SetMarkerColor(2);
  }

  for(int ievt=0;ievt<Conf::nEvts;++ievt){
    Conf::chain->GetEntry(ievt);
    double etInL[BGO_LayerNO]={0};
    for(int i=0;i<BGO_LayerNO;++i){
      etInL[i] = Conf::evt_bgo->GetTotalEnergy(i);
    }
    for(int ic = 0;ic < Conf::evt_bgo->fClusters->GetEntriesFast();++ic){
      DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(Conf::evt_bgo->fClusters->At(ic));
      int lid = aC->fLayer;
      for(int b = 0;b<aC->fFiredBar->GetEntriesFast();++b){
        DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(aC->fFiredBar->At(b));
        transInL[lid]->Fill(aB->fBar,aB->fE / etInL[lid]);
      }
    }
  }

  Conf::can.push_back(new TCanvas("Display_trans"+cName,"Display_Trans_"+cName));
  TCanvas *c1 = Conf::can[Conf::can.size()-1];
  c1->Divide(2,7,0,0);
  Conf::can.push_back(new TCanvas("Display_transo_pro"+cName,"Display_Trans_Pro"+cName));
  TCanvas *c2 = Conf::can[Conf::can.size()-1];
  c2->Divide(2,7,0,0);

  for(int i=0;i<BGO_LayerNO;++i){
    c1->cd(i+1);
    transInL[i]->Draw();
    c2->cd(i+1);
    transInL[i]->ProfileX()->Draw();
  }
}

//-------------------------------------------------------------------
TProfile *RMSFValueProfile(bool skipMips = true)
//TProfile *RMSFValueProfile(bool skipMips = true ,TCut cuts=Cuts::GlobalCut)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="RFG_In_Layer";
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *RMSInL = new TH2D(cName+"RMS","RMS_layer / RMS_max",14,0,14,11000,0,1.1);
    TH2D *FInL = new TH2D(cName+"FValue","FValue_layer / F_max",14,0,14,11000,0,1.1);
    TH2D *GInL = new TH2D(cName+"GValue","GValue_layer / G_max",14,0,14,11000,0,1.1);
    for(int ievt=0;ievt<Conf::nEvts;++ievt){
      Conf::chain->GetEntry(ievt);
      if(Cuts::IsVerticalMips() && skipMips){
        continue;
      }
      double MaxRMS = Conf::evt_bgo->GetMaxRMS();
      double MaxFV = Conf::evt_bgo->GetMaxFValue();
      double MaxGV = Conf::evt_bgo->GetMaxGValue();
      for(int i=0;i<BGO_LayerNO;++i){
        if(Conf::evt_bgo->fRMS[i]>=0){
          RMSInL->Fill(i,Conf::evt_bgo->fRMS[i]/MaxRMS);
          FInL->Fill(i,Conf::evt_bgo->fFValue[i]/MaxFV);
          if(Conf::evt_bgo->fRMS[i]!=0)GInL->Fill(i,Conf::evt_bgo->GetGValue(i)/MaxGV);
        }
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
    GInL->SetMarkerStyle(6);
    GInL->Draw();
    TProfile *GInL_proX = GInL->ProfileX();
    GInL_proX->SetMarkerStyle(30);
    GInL_proX->SetMarkerSize(2);
    GInL_proX->SetMarkerColor(2);
    GInL_proX->SetLineWidth(2);
    GInL_proX->SetLineColor(2);
    GInL_proX->Draw("sameeg");
    return RMSInL_proX;
}

//-------------------------------------------------------------------
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
      gPad->SetLogy();  gPad->SetGrid();
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
      gPad->SetLogy();  gPad->SetGrid();
    }
    c0->cd(23);
    name = "Bgo.GetTotalEnergy(";
    name += layer;
    name += ")";
    Conf::LinkTree()->Draw(name,cuts);
    gPad->SetLogy();  gPad->SetGrid();
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

//void // GValue by layer TH2D
// RMS value by layer,  RMS2RMSTotal by layer

void DrawThisEvent(long evtID)
{
  TString name = "EventID_";
          name += evtID;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          name +="--"+tmp;
  TH2F *xz =  new TH2F("XZ_"+name,"XZ",14,0,14,22,0,22);  xz->GetXaxis()->SetTitle("layer ID");   xz->GetYaxis()->SetTitle("bar ID");
  TH2F *yz =  new TH2F("YZ_"+name,"YZ",14,0,14,22,0,22);  yz->GetXaxis()->SetTitle("layer ID");   yz->GetYaxis()->SetTitle("bar ID");
  TH2D *eInL = new TH2D("energy profile"+name,"E_layer / E_total;layer ID;E_l/E_t",14,0,14,5000,0,0.5);
  vector<TH2D*> transInL(BGO_LayerNO);

  Conf::LinkTree()->GetEntry(evtID);

  cout<<"\n\n\tEvent ID "<<evtID<<endl;
  Conf::evt_bgo->MyPrint();

  double etInL[BGO_LayerNO]={0};
  for(int i=0;i<BGO_LayerNO;++i){
    etInL[i] = Conf::evt_bgo->GetTotalEnergy(i);
    TH2D *h = 0;
    if(etInL[i]>5){
      h = new TH2D(Form("layer %d",i)+name,Form("layer %d;BarID;EnergyRotaio",i),22,0,22,1000,0,1);
      h->SetMarkerColor(2);
      h->SetMarkerStyle(29);
    }
    transInL[i] = h;
  }
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
      if(transInL[lid]){
        transInL[lid]->Fill(aB->fBar,aB->fE / etInL[lid]);
      }
    }
  }
  double MaxRMS = Conf::evt_bgo->GetMaxRMS();
  double MaxFV = Conf::evt_bgo->GetMaxFValue();
  double MaxGV = Conf::evt_bgo->GetMaxGValue();
  double L_rms[BGO_LayerNO];
  double L_f[BGO_LayerNO];
  double L_g[BGO_LayerNO];
  double layerID[BGO_LayerNO];
  for(int i=0;i<BGO_LayerNO;++i){
    eInL->Fill(i,Conf::evt_bgo->GetTotalEnergy(i)/ Conf::evt_bgo->fTotE);
    if(Conf::evt_bgo->fRMS[i] >=0){
      L_rms[i] = Conf::evt_bgo->fRMS[i] / MaxRMS;
      L_f[i] = Conf::evt_bgo->fFValue[i] / MaxFV;
      L_g[i] = Conf::evt_bgo->GetGValue(i) / MaxGV;
    }else{
      L_rms[i] = L_f[i] = L_g[i] = Conf::evt_bgo->fRMS[i];
    }
    layerID[i]  = i;
  }

  Conf::can.push_back(new TCanvas("Display_"+name,"Display_"+name));
  TCanvas *c0 = Conf::can[Conf::can.size()-1];
  c0->Divide(1,2);
  c0->cd(1)->Divide(2,1);
  c0->cd(1)->cd(1)->Divide(1,2,0,0);
  c0->cd(1)->cd(1)->cd(1); gPad->SetGrid(); xz->Draw("colz");
  c0->cd(1)->cd(1)->cd(2); gPad->SetGrid(); yz->Draw("colz");
  c0->cd(1)->cd(2); gPad->SetGrid(); eInL->SetMarkerStyle(30);  eInL->Draw();
  c0->cd(2); gPad->SetGrid();
  TMultiGraph *mg = new TMultiGraph();
  TLegend *lg = new TLegend(0.55,0.65,0.66,0.82);

  TGraph *RMSInL = new TGraph(BGO_LayerNO,layerID,L_rms);
  RMSInL->SetMarkerStyle(33);
  RMSInL->SetMarkerColor(2);
  mg->Add(RMSInL);
  lg->AddEntry(RMSInL,"RMS_layer / RMS_max");

  TGraph *FInL = new TGraph(BGO_LayerNO,layerID,L_f);
  FInL->SetMarkerStyle(29);
  FInL->SetMarkerColor(4);
  mg->Add(FInL);
  lg->AddEntry(FInL,"F_layer / F_max");

  TGraph *GInL = new TGraph(BGO_LayerNO,layerID,L_g);
  GInL->SetMarkerStyle(28);
  GInL->SetMarkerColor(6);
  mg->Add(GInL);
  lg->AddEntry(GInL,"G_layer / G_max");
  mg->Draw("apl");
  lg->Draw("same");
  //gStyle->SetOptStat(0);
  Conf::can.push_back(new TCanvas("Display_trans"+name,"Display_Trans_"+name));
  TCanvas *c1 = Conf::can[Conf::can.size()-1];
  c1->Divide(2,7,0,0);

  for(int i=0;i<BGO_LayerNO;++i){
    c1->cd(i+1);
    if(transInL[i]){
      transInL[i]->Draw();
    }
  }

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

namespace Compare
{
void EGInLayer(TString electronFile, TString hadronFile,double low_e,double up_e,TCut cuts="")
{
  for(int lid=0;lid<BGO_LayerNO;++lid){
    Plot::ResetInputFile(electronFile);
    MyDraw(Form("Bgo.GetTotalEnergy(%d)/Bgo.fTotE : Bgo.GetGValue(%d)",lid,lid),Form("Bgo.GetTotalEnergy(%d)>5",lid) && cuts);
    Plot::ResetInputFile(hadronFile);
    MyDraw(Form("Bgo.GetTotalEnergy(%d)/Bgo.fTotE : Bgo.GetGValue(%d)",lid,lid),Form("Bgo.fTotE > %f && Bgo.fTotE<%f && Bgo.GetTotalEnergy(%d)>5",low_e,up_e,lid)&&cuts,"same*");
  }
  for(int lid=0;lid<BGO_LayerNO;++lid){
    Plot::ResetInputFile(electronFile);
    MyDraw(Form("Bgo.fFValue[%d] : Bgo.GetGValue(%d)",lid,lid),Form("Bgo.GetTotalEnergy(%d)>5",lid)&&cuts);
    Plot::ResetInputFile(hadronFile);
    MyDraw(Form("Bgo.fFValue[%d] : Bgo.GetGValue(%d)",lid,lid),Form("Bgo.fTotE > %f && Bgo.fTotE<%f && Bgo.GetTotalEnergy(%d)>5",low_e,up_e,lid)&&cuts,"same*");
  }
}

};

namespace Isolate
{
TH1D *ESpectrumOfIsolatedBar(int layerID,int BarID,double E_low, double E_high,double TotEnergyCut)
{
  TString cName = "c";
          cName +=Conf::can.size();
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
          cName +="--IoslatedBars-l";
          cName += layerID;
          cName +="-b";
          cName += BarID;
          cName += "ELow_";
          cName += E_low;
          cName += "EHigh_";
          cName += E_high;
          cName +="TotECut_";
          cName +=TotEnergyCut;

  TH1D *h1D_aIos = new TH1D(cName,Form("layer_%dbar_%d;E / MeV;Counts",layerID,BarID),(E_high-E_low+30)/0.05,E_low-10,E_high+40);
  h1D_aIos->SetMarkerStyle(7);
  h1D_aIos->SetMarkerStyle(5);
  h1D_aIos->SetLabelSize(0.05,"X");
  h1D_aIos->SetLabelSize(0.05,"Y");

  for(long i=0;i<Conf::nEvts;++i){
    Conf::LinkTree()->GetEntry(i);
    if(!Conf::evt_bgo->Group3_0000(0.2)){
      continue;
    }
    if(Conf::evt_bgo->fTotE < TotEnergyCut){
      continue;
    }
    for(int iL = 0;iL<BGO_LayerNO;++iL){
      std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBar(iL);
      for(unsigned int i=0;i<isoBars.size();++i){
        if(isoBars[i]->fLayer != layerID || isoBars[i]->fBar != BarID){
          continue;
        }
        double e = isoBars[i]->fE;
        if(e >E_low && e <E_high){
          h1D_aIos->Fill(e);
        }
      }
    }
  }
  return h1D_aIos;
}

void DrawESpecOfIoslatedBar(int layerID,double E_low=18,double E_high=30,double TotEnergyCut=350)
{
  TString cName = "c";
          cName +=Conf::can.size();
          cName +="--ESpectrumOfIoslatedBar-Layer";
          cName +=layerID;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
  TCanvas *c = new TCanvas(cName,cName);
  Conf::can.push_back(c);
  c->Divide(4,6);
  for(int ib=0;ib<BGO_BarNO;++ib){
    c->cd(ib+1);
    TH1D *h = ESpectrumOfIsolatedBar(layerID,ib,E_low,E_high,TotEnergyCut);
    h->Draw();
    gPad->SetGrid();
    gStyle->SetOptStat("uoieMR");
  }
}

void HowManyIsolateBarFit(double TotEnergyCut_low = 0,double TotEnergyCut_hi = 5000, int fromLayerID = 0, double E_low = 20,double E_high = 40,double Noise_E0 = 2)    // Bar E < E0 is noise, low
{
  TString cName = Form("c%d-IsolateBarStatic-",(int)Conf::can.size());
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
  TString totERange = Form("TotE %f to %f",TotEnergyCut_low,TotEnergyCut_hi);
  TString bar_ERange = Form("E Range of Bar%f to %f",E_low,E_high);
  TString layerCut = Form("From Layer %d",fromLayerID);

  TString  title = "NLayers VS NBars-"+layerCut;

  TH2F *h2D_NBar_NLayer = new TH2F(title+"-"+totERange,title+";N Isolated Bars;N Layers",50,0,25,28,0,14);
  h2D_NBar_NLayer->SetLabelSize(0.05,"X");
  h2D_NBar_NLayer->SetLabelSize(0.05,"Y");

  title = "First layer ID VS NBars-"+layerCut;
  TH2F *h2D_NBar_FirstLID = new TH2F(title+"-"+totERange,title+";N Isolated Bars;first layer ID",50,0,25,28,0,14);
  h2D_NBar_FirstLID->SetLabelSize(0.05,"X");
  h2D_NBar_FirstLID->SetLabelSize(0.05,"Y");

  title = "Layer RMS VS NLayers-"+layerCut;
  TH2F *h2D_NLayer_LRMS = new TH2F(title+"-"+totERange,title+";N Layers;layer RMS",28,0,14,70,0,7);
  h2D_NLayer_LRMS->SetLabelSize(0.05,"X");
  h2D_NLayer_LRMS->SetLabelSize(0.05,"Y");

  title = "First layer ID VS NLayers-"+layerCut;
  TH2F *h2D_NLayer_FirstLID = new TH2F(title+"-"+totERange,title+";N Layers;First layer ID",28,0,14,28,0,14);
  h2D_NLayer_FirstLID->SetLabelSize(0.05,"X");
  h2D_NLayer_FirstLID->SetLabelSize(0.05,"Y");

  title = "Bar RMS VS NBars-"+layerCut;
  TH2F *h2D_NBar_BarRMS = new TH2F(title+"-"+totERange,title+";N Isolated Bars;Bar RMS",50,0,25,100,0,10);
  h2D_NBar_BarRMS->SetLabelSize(0.05,"X");
  h2D_NBar_BarRMS->SetLabelSize(0.05,"Y");

  title = "NBarID VS NBars-"+layerCut;
  TH2F *h2D_NBar_NBarID = new TH2F(title+"-"+totERange,title+";N Isolated Bars;N Bar ID",50,0,25,20,0,10);
  h2D_NBar_NBarID->SetLabelSize(0.05,"X");
  h2D_NBar_NBarID->SetLabelSize(0.05,"Y");

  title = "First layer ID VS NBarID-"+layerCut;
  TH2F *h2D_NBarID_FirstLID = new TH2F(title+"-"+totERange,title+";N Bar ID;First layer ID",20,0,10,28,0,14);
  h2D_NBarID_FirstLID->SetLabelSize(0.05,"X");
  h2D_NBarID_FirstLID->SetLabelSize(0.05,"Y");

  title = "Mean energy of isolated bar VS NBars-"+layerCut;
  TH2F *h2D_NBar_MeanE = new TH2F(title+"-"+totERange,title+";N Isolated Bars;Mean Energy / MeV",50,0,25,140,0,70);
  h2D_NBar_MeanE->SetLabelSize(0.05,"X");
  h2D_NBar_MeanE->SetLabelSize(0.05,"Y");

  title = "Mean energy of isolated bar VS total E-"+layerCut;
  TH2F *h2D_ETot_MeanE = new TH2F(title+"-"+totERange,title+";Total E;Mean Energy Isolated Bar / MeV",(TotEnergyCut_hi-TotEnergyCut_low)/50,TotEnergyCut_low,TotEnergyCut_hi,140,0,70);
  h2D_ETot_MeanE->SetLabelSize(0.05,"X");
  h2D_ETot_MeanE->SetLabelSize(0.05,"Y");

  title = "total energy of isolated bar VS total E-"+layerCut;
  TH2F *h2D_ETot_TotEOfIsoBar = new TH2F(title+"-"+totERange,title+";Total E;toal energy of Isolated Bar",(TotEnergyCut_hi-TotEnergyCut_low)/50,TotEnergyCut_low,TotEnergyCut_hi,2000,0,1000);
  h2D_ETot_TotEOfIsoBar->SetLabelSize(0.05,"X");
  h2D_ETot_TotEOfIsoBar->SetLabelSize(0.05,"Y");

  title = "total energy of isolated bar VS In ranged bar Number-"+bar_ERange+"-"+layerCut;
  TH2F *h2D_TotEInRangeIsoBar_InRangeNumber = new TH2F(title+"-"+totERange,title+";in ranged bars; toal energy of Isolated Bar",40,0,20,2000,0,1000);
  h2D_TotEInRangeIsoBar_InRangeNumber->SetLabelSize(0.05,"X");
  h2D_TotEInRangeIsoBar_InRangeNumber->SetLabelSize(0.05,"Y");

  title = "NBars VS total E-"+layerCut;
  TH2F *h2D_ETot_NBar = new TH2F(title+"-"+totERange,title+";Total E;Number of Isolated Bar",(TotEnergyCut_hi-TotEnergyCut_low)/50,TotEnergyCut_low,TotEnergyCut_hi,50,0,25);
  h2D_ETot_NBar->SetLabelSize(0.05,"X");
  h2D_ETot_NBar->SetLabelSize(0.05,"Y");


  title = "Mean energy of isolated bar VS Bar ratio-"+bar_ERange+"-"+layerCut;
  TH2F *h2D_BarRatio_MeanE = new TH2F(title+"-"+totERange,title+";BarRatio;Mean_E / MeV",100,0,1,140,0,70);
  h2D_BarRatio_MeanE->SetLabelSize(0.05,"X");
  h2D_BarRatio_MeanE->SetLabelSize(0.05,"Y");

  title = "In energy range number VS Isolated bar number-"+bar_ERange+"-"+layerCut;
  TH2F *h2D_BarRatio = new TH2F(title+"-"+totERange,title+";Isolated Bar number;Bars in energy range",50,0,25,40,0,20);
  h2D_BarRatio->SetLabelSize(0.05,"X");
  h2D_BarRatio->SetLabelSize(0.05,"Y");

  title = "Bar RMS VS Bar ratio-"+bar_ERange+"-"+layerCut;
  TH2F *h2D_BarRatio_BarRMS = new TH2F(title+"-"+totERange,title+";BarRatio;Bar RMS",100,0,1,100,0,10);
  h2D_BarRatio_BarRMS->SetLabelSize(0.05,"X");
  h2D_BarRatio_BarRMS->SetLabelSize(0.05,"Y");

  title = "Layer RMS VS Bar ratio-"+bar_ERange+"-"+layerCut;
  TH2F *h2D_BarRatio_LRMS = new TH2F(title+"-"+totERange,title+";BarRatio;Layer RMS",100,0,1,70,0,7);
  h2D_BarRatio_LRMS->SetLabelSize(0.05,"X");
  h2D_BarRatio_LRMS->SetLabelSize(0.05,"Y");

  title = "First layer ID VS Bar ratio-"+bar_ERange+"-"+layerCut;
  TH2F *h2D_BarRatio_FirstLID = new TH2F(title+"-"+totERange,title+";BarRatio;First Layer ID",100,0,1,28,0,14);
  h2D_BarRatio_FirstLID->SetLabelSize(0.05,"X");
  h2D_BarRatio_FirstLID->SetLabelSize(0.05,"Y");

  long passTrigger = 0;
  long inTotERange = 0;

  for(long ievt=0;ievt<Conf::nEvts;++ievt){
    Conf::LinkTree()->GetEntry(ievt);
    if(!Conf::evt_bgo->Group3_0000(0.2)){
      continue;
    }
    ++passTrigger;
    if(Conf::evt_bgo->fTotE < TotEnergyCut_low || Conf::evt_bgo->fTotE > TotEnergyCut_hi){
      continue;
    }
    ++inTotERange;
    std::vector<int>  isoLayerID;
    std::vector<int>  isoBarsID;
    std::vector<double>  isoEnergy;
    std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBarFromLayer(fromLayerID,Noise_E0);
    int nisobar = isoBars.size();
    for(int ib=0;ib<nisobar;++ib){
      isoLayerID.push_back(isoBars[ib]->fLayer);
      isoBarsID.push_back(isoBars[ib]->fBar);
      isoEnergy.push_back(isoBars[ib]->fE);
    }
    int lid[nisobar];
    int bid[nisobar];
    double elist[nisobar];
    double nBarsInERange =0 ;   // type double
    double eSum = 0;
    double eSumBiggerThanERangeLow = 0;
    for(int i = 0;i<nisobar;++i){
      lid[i] = isoLayerID[i];
      bid[i] = isoBarsID[i];
      elist[i] = isoEnergy[i];
      if(isoEnergy[i] >= E_low ){
        if( isoEnergy[i] <= E_high){
          eSumBiggerThanERangeLow += isoEnergy[i];
          ++nBarsInERange;
        }
      }
      eSum += isoEnergy[i];
    }
    //std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
    double lRMS = TMath::RMS(nisobar,lid);
    double bRMS = TMath::RMS(nisobar,bid);
    double eMean = TMath::Mean(nisobar,elist);
    for(std::vector<int>::iterator it = isoBarsID.begin();it != isoBarsID.end();){
      std::vector<int>::iterator aaa = find(isoBarsID.begin(),it,*it);
      if(aaa == it){
        ++it;
      }else{
        it = isoBarsID.erase(it);
      }
    }
    int nBarID = isoBarsID.size();
    for(std::vector<int>::iterator it = isoLayerID.begin();it != isoLayerID.end();){
      std::vector<int>::iterator aaa = find(isoLayerID.begin(),it,*it);
      if(aaa == it){
        ++it;
      }else{
        it = isoLayerID.erase(it);
      }
    }
    int nLayerID=isoLayerID.size();
    int lid_u[nLayerID];
    for(int i=0;i<nLayerID;++i){
      lid_u[i] = isoLayerID[i];
    }
    int firstLID = TMath::MinElement(nLayerID,lid_u);

    h2D_NBar_NBarID->Fill(nisobar,nBarID);
    h2D_NBar_BarRMS->Fill(nisobar,bRMS);
    h2D_NBar_NLayer->Fill(nisobar,nLayerID);
    h2D_NBar_MeanE->Fill(nisobar,eMean);

    h2D_NBar_FirstLID->Fill(nisobar,firstLID);
    h2D_NBarID_FirstLID->Fill(nBarID,firstLID);
    h2D_NLayer_LRMS->Fill(nLayerID,lRMS);
    h2D_NLayer_FirstLID->Fill(nLayerID,firstLID);

    h2D_BarRatio->Fill(nisobar,nBarsInERange);

    if(nisobar > 0){
      h2D_BarRatio_MeanE->Fill(nBarsInERange / nisobar, eMean);
      h2D_BarRatio_BarRMS->Fill(nBarsInERange / nisobar, bRMS);
      h2D_BarRatio_LRMS->Fill(nBarsInERange / nisobar, lRMS);
      h2D_BarRatio_FirstLID->Fill(nBarsInERange / nisobar, firstLID);
    }

    h2D_ETot_NBar->Fill(Conf::evt_bgo->fTotE,nisobar);
    h2D_ETot_MeanE->Fill(Conf::evt_bgo->fTotE,eMean);
    h2D_ETot_TotEOfIsoBar->Fill(Conf::evt_bgo->fTotE,eSum);
    h2D_TotEInRangeIsoBar_InRangeNumber->Fill(nBarsInERange,eSumBiggerThanERangeLow);

  }

  cName += totERange+"_"+bar_ERange+"_"+layerCut+"_E0_";
  cName += Noise_E0;
  TCanvas *c1 = new TCanvas(cName+"-a",cName+"a");
  Conf::can.push_back(c1);
  c1->Divide(2,2);
  c1->cd(1); gPad->SetGrid();   h2D_NBar_NBarID->Draw("colz");
  c1->cd(2); gPad->SetGrid();   h2D_NBar_BarRMS->Draw("colz");
  c1->cd(3); gPad->SetGrid();   h2D_NBar_NLayer->Draw("colz");
  c1->cd(4); gPad->SetGrid();   h2D_NBar_MeanE->Draw("colz"); gPad->SetLogy();

  TCanvas *c2 = new TCanvas(cName+"-b",cName+"b");
  Conf::can.push_back(c2);
  c2->Divide(2,2);
  c2->cd(1); gPad->SetGrid();   h2D_NBar_FirstLID->Draw("colz");
  c2->cd(2); gPad->SetGrid();   h2D_NBarID_FirstLID->Draw("colz");
  c2->cd(3); gPad->SetGrid();   h2D_NLayer_LRMS->Draw("colz");
  c2->cd(4); gPad->SetGrid();   h2D_NLayer_FirstLID->Draw("colz");

  TCanvas *c3 = new TCanvas(cName+"-c",cName+"c");
  Conf::can.push_back(c3);
  c3->Divide(2,2);
  c3->cd(1); gPad->SetGrid();   h2D_BarRatio_MeanE->Draw("colz");   gPad->SetLogy();
  c3->cd(2); gPad->SetGrid();   h2D_BarRatio_BarRMS->Draw("colz");
  c3->cd(3); gPad->SetGrid();   h2D_BarRatio_FirstLID->Draw("colz");
  c3->cd(4); gPad->SetGrid();   h2D_BarRatio_LRMS->Draw("colz");

  TCanvas *c4 = new TCanvas(cName+"-d",cName+"d");
  Conf::can.push_back(c4);
  c4->Divide(2,2);
  c4->cd(1); gPad->SetGrid();   h2D_ETot_NBar->Draw("colz");
  c4->cd(2); gPad->SetGrid();   h2D_ETot_MeanE->Draw("colz");   gPad->SetLogy();
  c4->cd(3); gPad->SetGrid();   h2D_ETot_TotEOfIsoBar->Draw("colz"); gPad->SetLogy();
  c4->cd(4); gPad->SetGrid();   h2D_TotEInRangeIsoBar_InRangeNumber->Draw("colz"); gPad->SetLogy();

  TCanvas *c5 = new TCanvas(cName+"-e",cName+"e");
  Conf::can.push_back(c5);
  h2D_BarRatio->Draw("colz");


  std::cout<<"input entries = "<<Conf::nEvts<<",\tpassed trigger = "<<passTrigger<<",\tin total energy range ("<<TotEnergyCut_low<<","<<TotEnergyCut_hi<<") = "<<inTotERange<<std::endl;
  std::cout<<fromLayerID<<"\t"<<h2D_BarRatio->GetMean(1)<<"\t"<<h2D_BarRatio->GetMeanError(1)<<"\t"<<h2D_BarRatio->GetRMS(1)<<"\t"<<h2D_BarRatio->GetRMSError(1)<<"\t\t"<<h2D_BarRatio->GetMean(2)<<"\t"<<h2D_BarRatio->GetMeanError(2)<<"\t"<<h2D_BarRatio->GetRMS(2)<<"\t"<<h2D_BarRatio->GetRMSError(2)<<std::endl;
}

void IsolatedBar(double eRangeLo=5, double eRangeHi = 10000, int layerCut = 4,double eCut = 2)
{
  TString cName = "c";
          cName +=Conf::can.size();
          cName +="--IoslatedBars-";
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

  TH3D *h3d_EOfIsoBar = new TH3D("Iosbar"+cName,"E_spectrum of Isolated bar;Layer ID;bar ID; E / MeV",28,0,14,22,0,22,120,0,60);
  TH2D *h2D_EL = new TH2D("Iosbar_E_L"+cName,";Layer;E / MeV",28,0,14,120,0,60);
  TH2D *h2D_EL3MeV = new TH2D("Iosbar_E_L_3MeV"+cName,";Layer;E / MeV",28,0,14,60,0,3);
  TH1D *h1D_E120MeV = new TH1D("Iosbar_E_120MeV"+cName,"E_spectrum < 120MeV;E / MeV;Counts",1200,0,120);
  TH1D *h1D_E20MeV = new TH1D("Iosbar_E_20MeV"+cName,"E_spectrum < 20MeV;E / MeV;Counts",400,0,20);
  h1D_E120MeV->SetMarkerStyle(7);
  h1D_E20MeV->SetMarkerStyle(5);

  for(long i=0;i<Conf::nEvts;++i){
    Conf::LinkTree()->GetEntry(i);
    if(!Conf::evt_bgo->Group3_0000(0.2)){
      continue;
    }
    if(Conf::evt_bgo->fTotE < eRangeLo || Conf::evt_bgo->fTotE > eRangeHi){
      continue;
    }
    if(Conf::evt_bgo->fTotE > 200 && Conf::evt_bgo->fTotE<450 && Conf::evt_bgo->fLRMS > 3.5 && Conf::evt_bgo->fLRMS < 4.4 && Conf::evt_bgo->GetTotalRMS() > -2 && Conf::evt_bgo->GetTotalRMS() < 2){
            // Mips cut
      continue; 
    }
    if(Cuts::IsVerticalMips()){
      continue;
    }

      std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBarFromLayer(layerCut,eCut);
      for(unsigned int i=0;i<isoBars.size();++i){
        double e = isoBars[i]->fE;
        h3d_EOfIsoBar->Fill(isoBars[i]->fLayer,isoBars[i]->fBar,e);
        h2D_EL->Fill(isoBars[i]->fLayer,e);
        if(e<=3){
          h2D_EL3MeV->Fill(isoBars[i]->fLayer,e);
        }
        h1D_E20MeV->Fill(e);
        h1D_E120MeV->Fill(e);
      }
  }

  TCanvas *c = new TCanvas(cName,cName);
  Conf::can.push_back(c);
  c->Divide(2,2);

  c->cd(1);
  h3d_EOfIsoBar->Draw("LEGO2Z");
  c->cd(2)->Divide(1,2,0,0);
  c->cd(2)->cd(1);
  h2D_EL->SetLabelSize(0.1,"X");
  h2D_EL->SetLabelSize(0.1,"Y");
  h2D_EL->SetTitleSize(0.05,"X");
  h2D_EL->Draw("colz");
  c->cd(2)->cd(2);
  h2D_EL3MeV->Draw("colz");
  h2D_EL3MeV->SetLabelSize(0.1,"X");
  h2D_EL3MeV->SetLabelSize(0.1,"Y");

  c->cd(3);
  gPad->SetGrid();
  gPad->SetLogy();
  h1D_E120MeV->SetLabelSize(0.05,"X");
  h1D_E120MeV->SetLabelSize(0.05,"Y");
  h1D_E120MeV->Draw("normE");
  c->cd(4);
  gPad->SetGrid();
  gPad->SetLogy();
  h1D_E20MeV->SetLabelSize(0.05,"X");
  h1D_E20MeV->SetLabelSize(0.05,"Y");
  h1D_E20MeV->Draw("normE");
}

void LongitudinalDevelopment(double TotEnergyCut_low = 0,double TotEnergyCut_hi = 5000, double E_low = 20,double E_high = 40,double Noise_E0 = 2)    // Bar E < E0 is noise, low
{
  TString cName = Form("c%d-IsolateLayerDevelopment-",(int)Conf::can.size());
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
  TString totERange = Form("TotE %f to %f",TotEnergyCut_low,TotEnergyCut_hi);
  TString bar_ERange = Form("E Range of Bar%f to %f",E_low,E_high);

  TString  title = "BarID-CoG VS layer ID-";

  TH2F *h2D_CoGDis_LID = new TH2F(title+totERange+bar_ERange+tmp,title+bar_ERange+";Layer ID;Bar ID - CoG",28,0,14,80,-20,20);
  h2D_CoGDis_LID->SetLabelSize(0.05,"X");
  h2D_CoGDis_LID->SetLabelSize(0.05,"Y");

  title = "Mean E VS layer ID-";
  TH2F *h2D_MeanE_LID = new TH2F(title+totERange+bar_ERange+tmp,title+bar_ERange+";Layer ID;Mean E / MeV",28,0,14,E_high,0,E_high);
  h2D_MeanE_LID->SetLabelSize(0.05,"X");
  h2D_MeanE_LID->SetLabelSize(0.05,"Y");

  title = "E RMS VS layer ID-";
  TH2F *h2D_ERMS_LID = new TH2F(title+totERange+bar_ERange+tmp,title+bar_ERange+";Layer ID;RMS of E / MeV",28,0,14,60,0,30);
  h2D_ERMS_LID->SetLabelSize(0.05,"X");
  h2D_ERMS_LID->SetLabelSize(0.05,"Y");

  title = "NBars VS layer ID-";
  TH2F *h2D_NBars_LID = new TH2F(title+totERange+bar_ERange+tmp,title+bar_ERange+";Layer ID;Bar number",28,0,14,50,0,25);
  h2D_NBars_LID->SetLabelSize(0.05,"X");
  h2D_NBars_LID->SetLabelSize(0.05,"Y");

  long passTrigger = 0;
  long inTotERange = 0;

  for(long ievt=0;ievt<Conf::nEvts;++ievt){
    Conf::LinkTree()->GetEntry(ievt);
    if(!Conf::evt_bgo->Group3_0000(0.2)){
      continue;
    }
    ++passTrigger;
    if(Conf::evt_bgo->fTotE < TotEnergyCut_low || Conf::evt_bgo->fTotE > TotEnergyCut_hi){
      continue;
    }
    ++inTotERange;
    for(int l=0;l<BGO_LayerNO;++l){
      std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBar(l,E_low,E_high,Noise_E0);
      int nB = isoBars.size();
      h2D_NBars_LID->Fill(l,nB);
      if(nB == 0){
        continue;
      }
      double  CoG = Conf::evt_bgo->GetCoGBarIDInLayer(l);
      double elist[nB];
      for(int ib = 0;ib<nB;++ib){
        elist[ib] = isoBars[ib]->fE;
        h2D_CoGDis_LID->Fill(l,isoBars[ib]->fBar - CoG);
      }
      h2D_MeanE_LID->Fill(l,TMath::Mean(nB,elist));
      h2D_ERMS_LID->Fill(l,TMath::RMS(nB,elist));
    }
  }

  cName += totERange+"-"+bar_ERange;
  TCanvas *c1 = new TCanvas(cName+"a",cName+"a");
  Conf::can.push_back(c1);
  c1->Divide(2,2);
  c1->cd(1); gPad->SetGrid();   h2D_NBars_LID->Draw("colz");
  c1->cd(2); gPad->SetGrid();   h2D_MeanE_LID->Draw("colz");//   gPad->SetLogy();
  c1->cd(3); gPad->SetGrid();   h2D_CoGDis_LID->Draw("colz");
  c1->cd(4); gPad->SetGrid();   h2D_ERMS_LID->Draw("colz");// gPad->SetLogy();


  std::cout<<"input entries = "<<Conf::nEvts<<",\tpassed trigger = "<<passTrigger<<",\tin total energy range ("<<TotEnergyCut_low<<","<<TotEnergyCut_hi<<") = "<<inTotERange<<std::endl;

}

void BarNumbers(double TotEnergyCut_low =3000, double TotEnergyCut_hi = 5000,double Noise_E0 = 2)
{
  TString cName = Form("c%d-IsolateLayerDevelopment-",(int)Conf::can.size());
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp+"--TotERange_";
          cName +="--Noise_E0-";
          cName +=Noise_E0;
  TString totERange = Form("TotE %f to %f",TotEnergyCut_low,TotEnergyCut_hi);
  cName +=totERange;

  TString title = "Total number of isolated bars_NoiseCut-";
  title+=Noise_E0;

  TH1D *h1D_TotNO =new TH1D(title+tmp+totERange,title+";total number;count",80,0,40);
  h1D_TotNO->SetLabelSize(0.05,"X");
  h1D_TotNO->SetLabelSize(0.05,"Y");

  title = "isolated bar number in layer ID_NoiseCut-";
  title += Noise_E0;
  TH2F *h2D_TotNo_LID = new TH2F(title+tmp+totERange,title+";Layer ID;Ratio",28,0,14,110,0,1.1);
  h2D_TotNo_LID->SetLabelSize(0.05,"X");
  h2D_TotNo_LID->SetLabelSize(0.05,"Y");

  title = "isolated bar number-E<20MeV in layer ID_NoiseCut-";
  title += Noise_E0;
  TH2F *h2D_NoL20MeV_LID = new TH2F(title+tmp+totERange,title+";Layer ID;Ratio",28,0,14,110,0,1.1);
  h2D_NoL20MeV_LID->SetLabelSize(0.05,"X");
  h2D_NoL20MeV_LID->SetLabelSize(0.05,"Y");

  title = "isolated bar number-E>20MeV in layer ID_NoiseCut-";
  title += Noise_E0;
  TH2F *h2D_NoH20MeV_LID = new TH2F(title+tmp+totERange,title+";Layer ID;Ratio",28,0,14,110,0,1.1);
  h2D_NoH20MeV_LID->SetLabelSize(0.05,"X");
  h2D_NoH20MeV_LID->SetLabelSize(0.05,"Y");

  long passTrigger = 0;
  long inTotERange = 0;

  for(long ievt=0;ievt<Conf::nEvts;++ievt){
    Conf::LinkTree()->GetEntry(ievt);
    if(!Conf::evt_bgo->Group3_0000(0.2)){
      continue;
    }
    ++passTrigger;
    if(Conf::evt_bgo->fTotE < TotEnergyCut_low || Conf::evt_bgo->fTotE > TotEnergyCut_hi){
      continue;
    }
    ++inTotERange;
    std::vector<DmpBgoFiredBar*>  allEvt = Conf::evt_bgo->GetIsolatedBarFromLayer(0,Noise_E0);
    double allIsoBars = allEvt.size();
    h1D_TotNO->Fill(allIsoBars);
    for(int l=0;l<BGO_LayerNO;++l){
      std::vector<DmpBgoFiredBar*>  allIsoL = Conf::evt_bgo->GetIsolatedBar(l,Noise_E0);
      int nal = allIsoL.size();
      h2D_TotNo_LID->Fill(l,nal / allIsoBars);
      std::vector<DmpBgoFiredBar*>  allIsoLL20MeV = Conf::evt_bgo->GetIsolatedBar(l,Noise_E0,20,Noise_E0);
      int n = allIsoLL20MeV.size();
      h2D_NoL20MeV_LID->Fill(l,n / allIsoBars);
      h2D_NoH20MeV_LID->Fill(l,(nal-n) / allIsoBars);
    }
  }


  TCanvas *c2 = new TCanvas(cName+"b",cName+"b");
  Conf::can.push_back(c2);
  c2->Divide(2,2);
  c2->cd(1); gPad->SetGrid();   h1D_TotNO->Draw();
  c2->cd(2); gPad->SetGrid();   h2D_TotNo_LID->Draw("colz");
  c2->cd(3); gPad->SetGrid();   h2D_NoL20MeV_LID->Draw("colz");
  c2->cd(4); gPad->SetGrid();   h2D_NoH20MeV_LID->Draw("colz");
}

};

};

#endif

