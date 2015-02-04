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
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TGraph.h"
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
  TCut GlobalCut = "Bgo.T0()";
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
      if(Conf::evt_bgo->GetFiredBarNumber() == 14 && Conf::evt_bgo->GetPileupRatio() == 0 && skipMips){
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
TProfile *RMSFValueProfile(bool skipMips = true ,TCut cuts=Cuts::GlobalCut)
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
      if(Conf::evt_bgo->GetFiredBarNumber() == 14 && Conf::evt_bgo->GetPileupRatio() == 0 && skipMips){
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

};


#endif



