/*
 *  $Id: MyPlots.cxx, 2015-07-09 17:24:01 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 23/01/2015
//-------------------------------------------------------------------
//for Bgo Shower
 *
*/

#include <vector>
#include <iostream>
#include <fstream>
#include <map>

#include "TCut.h"
#include "TCanvas.h"
#include "TH2F.h"
#include "TH3D.h"
#include "TProfile.h"
#include "TChain.h"
#include "TStyle.h"
#include "TLine.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TDirectory.h"
#include "TGraph.h"
#include "TMath.h"
#include "DmpEvtBgoShower.h"
#include "DmpEvtMCTrack.h"
#include "DmpEvtMCPrimaryParticle.h"

#ifndef MyPlot_CXX
#define MyPlot_CXX
#define  __treeName "/Event/Rec0"
#define  __mcVertex  "TrackVertex"
#define  __mcPrim   "PrimaryParticle"

namespace Plot
{
namespace Conf
{
  vector<TCanvas*>  can;
  vector<TString>   inputFileName;

  long nEvts = 0;
  TChain *chain_BGOShower = 0;
  DmpEvtBgoShower *evt_bgo = new DmpEvtBgoShower();
  DmpEvtMCTrack  *evt_mcVertex = 0;
  DmpEvtMCPrimaryParticle *evt_mcPrim = 0;

  TChain *LinkTree()
  {
    if(inputFileName.size() == 0){
      cout<<"\tWARNING:\t do not have any input files"<<endl;
      cout<<"\t\tPlot::ResetInputFile(filename)"<<endl;
      cout<<"\t\tPlot::AddInputFile(filename)\n"<<endl;
      return 0;
    }
    if(chain_BGOShower == 0){
      chain_BGOShower  = new TChain(__treeName);
      for(unsigned int i = 0;i<inputFileName.size();++i){
        chain_BGOShower->AddFile(inputFileName[i]);
      }
      chain_BGOShower->SetBranchAddress("Bgo",&evt_bgo);
      if(chain_BGOShower->GetBranch(__mcVertex)){
        evt_mcVertex = new DmpEvtMCTrack();
        chain_BGOShower->SetBranchAddress(__mcVertex,&evt_mcVertex);
      }
      if(chain_BGOShower->GetBranch(__mcPrim)){
        evt_mcPrim = new DmpEvtMCPrimaryParticle();
        chain_BGOShower->SetBranchAddress(__mcPrim,&evt_mcPrim);
      }
      nEvts = chain_BGOShower->GetEntries();
      std::cout<<"===>  entries: "<<nEvts<<std::endl;
    }
    return chain_BGOShower;
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
    TLegend *lg = new TLegend(0.6,0.7,0.9,0.9);
    return lg;
  }

  TString eFile = "./Output/MC_eletron_150GeV_P43_Evts32000.root";
  TString pFile = "./Output/MC_proton_400GeV_P43_Evts72200.root";

  TString eFileD = "./Output/Data_e150GeV_P43_68_0-20141115_090724_093720_134904_Evts8792.root";
  TString pFileD = "./Output/Data_p400GeV_P43_68_0.1-20141114_142921-222051_Evts4131.root";

};

namespace Style
{
  void SetAxis(TH1* h,float xSize=0.06,float xOffset=0.8,float ySize=0.06,float yOffset=0.8,float xLabelS= 0.05,float yLabelS=0.05)
  {
    h->SetTitleSize(xSize,"X");
    h->SetTitleOffset(xOffset,"X");
    h->SetTitleSize(ySize,"Y");
    h->SetTitleOffset(yOffset,"Y");
    h->SetLabelSize(xLabelS,"X");
    h->SetLabelSize(yLabelS,"Y");
    h->SetLineWidth(2);
  }

  void SetMarker(TAttMarker* h,float style=30,float color=2, float size=1)
  {
    h->SetMarkerStyle(style);
    h->SetMarkerColor(color);
    h->SetMarkerSize(size);
  }
};

//-------------------------------------------------------------------
/*
namespace Cuts
{
  TString T0 = "Bgo.T0()";
  TString ECut0 = "Bgo.fTotE>100 &&Bgo.fTotE<99999999";
  TString ECut1 = "Bgo.fTotE>135000 &&Bgo.fTotE<150000";
  TString Trig3_0000 = "Bgo.Group3_0000(0.2)";

  TString Mips0 = "Bgo.GetMaxRMS()<0.55";
  TString Mips1 = "Bgo.fTotE > 200 && Bgo.fTotE <450 && Bgo.fLRMS > 3.5 && Bgo.fLRMS < 4.4 && Bgo.GetTotalRMS()>-2 && Bgo.GetTotalRMS()<2";
  TString Mips2 = "Bgo.GetFiredBarNumber() < 16 && Bgo.ClusterNo(10) < 16";
};
  //bool IsVerticalMips()
  //{
  //  //return (Conf::evt_bgo->GetFiredBarNumber() < 16 && Conf::evt_bgo->GetClusterNo(10) <16);
  //}
*/

std::map<TString,TString>  Cut;
#ifdef __MAKECINT__
#pragma link C++ class std::map<TString,TString>+;
#endif
void AddCut(TString a,TString b)
{
  Cut[a] = b;
}

void PrintCuts()
{
  cout<<"\nPrint Cuts:"<<endl;
  for(std::map<TString,TString>::iterator it=Cut.begin();it!=Cut.end();++it){
    cout<<"\t"<<it->first<<"\t\t"<<it->second<<endl;
  }
  cout<<endl;
}

void InitCuts()
{
  if(Cut.size()) return;
  cout<<"\nLoading Cuts..."<<endl;
    Cut["T0"]           =   "Bgo.T0()";
    Cut["ECut0"]        =   "Bgo.fTotE>10 &&Bgo.fTotE<99999999";
    Cut["ECut1"]        =   "Bgo.fTotE>135000 &&Bgo.fTotE<150000";
    Cut["Trig3_0000"]   =   "Bgo.Group3_0000(0.2)";
    Cut["Mips0"]        =   "Bgo.GetMaxRMS()<0.55";
    Cut["Mips1"]        =   "Bgo.fTotE > 200 && Bgo.fTotE <450 && Bgo.fLRMS > 3.5 && Bgo.fLRMS < 4.4 && Bgo.GetTotalRMS()>-2 && Bgo.GetTotalRMS()<2";
    Cut["Mips2"]        =   "Bgo.GetFiredBarNumber() < 16 && Bgo.ClusterNo(10) < 16";

    Cut["BT_e"]         =   "Bgo.T0() && Bgo.fLRMS > 1.9 && Bgo.fLRMS < 2.4 && Bgo.GetTotalRMS() > 12 && Bgo.GetTotalRMS()<17 && Bgo.GetEntryPoint().x() >37 && Bgo.GetEntryPoint().x()<45 && Bgo.GetEntryPoint().y()>64 && Bgo.GetEntryPoint().y()<74";
    Cut["BT_p"]         = Cut["T0"] +"&&" + Cut["ECut1"];
    PrintCuts();
}

//-------------------------------------------------------------------
void ResetInputFile(TString f)
{
  Conf::inputFileName.clear();
  Conf::inputFileName.push_back(f);
  if(Conf::chain_BGOShower){
    delete Conf::chain_BGOShower;
    Conf::chain_BGOShower = 0;
    gStyle->SetOptStat("ouiRMe");
  }
  InitCuts();
  Conf::LinkTree();
}

void AddInputFile(TString f)
{
  Conf::inputFileName.push_back(f);
  if(Conf::chain_BGOShower == 0){
    ResetInputFile(f);
  }else{
    Conf::chain_BGOShower->AddFile(f);
    Conf::nEvts = Conf::chain_BGOShower->GetEntries();
    std::cout<<"===>  entries: "<<Conf::nEvts<<std::endl;
  }
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

void MyDraw(TString exp, TString cuts= Cut["T0"], TString opt="",long entries=1000000000,long firstEntry = 0)
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
  if(opt.Contains("sameq")){
    opt = "";
  }
  Conf::LinkTree()->Draw(exp,cuts,opt,entries,firstEntry);
  gPad->SetGrid();
}

void DrawEventID(TString selection,TString opt="",long entries=1000000000,long firstEntry = 0)
{
  MyDraw("This->GetReadEntry()",selection,opt,entries,firstEntry);
}

long GetEntries(TString selection)
{
  TTree *t = Conf::LinkTree()->CopyTree(selection);
  return t->GetEntries();
}

void DrawThisEvent(long evtID)
{
  TString name = "EventID_";
          name += evtID;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          name +="--"+tmp;
  TH2F *xz =  new TH2F("XZ_"+name,"XZ;layer ID;bar ID",14,0,14,22,0,22); // xz->GetXaxis()->SetTitle("layer ID");   xz->GetYaxis()->SetTitle("bar ID");
  Style::SetAxis(xz);
  TH2F *yz =  new TH2F("YZ_"+name,"YZ;layer ID;bar ID",14,0,14,22,0,22); // yz->GetXaxis()->SetTitle("layer ID");   yz->GetYaxis()->SetTitle("bar ID");
  Style::SetAxis(yz);

  Conf::LinkTree()->GetEntry(evtID);

  double totalE = Conf::evt_bgo->fTotE;

  cout<<"\n\n\tEvent ID "<<evtID<<"\tRec E = "<<totalE/1000<<" GeV"<<endl;

  double etInL[BGO_LayerNO]={0};
  //double L_rms[BGO_LayerNO];
  //double L_f[BGO_LayerNO];
  double L_g[BGO_LayerNO];
  double L_EOfSeedBar[BGO_LayerNO];
  double L_Fractal[BGO_LayerNO];
  double layerID[BGO_LayerNO];
  double L_ERL3[BGO_LayerNO];
  double L_firedBarNo[BGO_LayerNO];

  for(int l=0;l<BGO_LayerNO;++l){
    std::vector<DmpBgoFiredBar*>  abb = Conf::evt_bgo->GetBars(l,2.5,10);
    if(l%2 == 0){
      for(size_t b=0;b<abb.size();++b){
        yz->Fill(l,abb[b]->fBar,abb[b]->fE / totalE);
      }
    }else{
      for(size_t b=0;b<abb.size();++b){
        xz->Fill(l,abb[b]->fBar,abb[b]->fE / totalE);
      }
    }
    L_ERL3[l] = Conf::evt_bgo->GetERL3(l);
    etInL[l] = Conf::evt_bgo->GetTotalEnergy(l) / totalE;
    L_g[l] = Conf::evt_bgo->GetGValue(l);
    layerID[l]  = l;
    DmpBgoFiredBar *b = Conf::evt_bgo->GetEMaxBarInLayer(l);
    if(b){
      L_EOfSeedBar[l] = b->fE / totalE;
    }else{
      L_EOfSeedBar[l] =0;
    }
    L_Fractal[l] = Conf::evt_bgo->GetFractal(l,1,4);
    L_firedBarNo[l] = Conf::evt_bgo->GetFiredBarNumber(l);
  }

  Conf::can.push_back(new TCanvas("Display_"+name,"Display_"+name));
  TCanvas *c0 = Conf::can[Conf::can.size()-1];
  gStyle->SetOptStat("");
  c0->Divide(1,2);
  c0->cd(1)->Divide(1,2,0,0);
  c0->cd(1)->cd(1);
  gPad->SetGrid();
  xz->Draw("colz");
  c0->cd(1)->cd(2);
  gPad->SetGrid();
  yz->Draw("colz");

  c0->cd(2);
  TGraph *eInL = new TGraph(BGO_LayerNO,layerID,etInL);
  gPad->SetGrid();
  eInL->SetTitle("Energy profile;layer ID; E ratio");
  Style::SetAxis(eInL->GetHistogram());
  Style::SetMarker(eInL,30,2);
  eInL->Draw();

  TGraph *ESeedInL = new TGraph(BGO_LayerNO,layerID,L_EOfSeedBar);
  ESeedInL->SetTitle("E of Seed Bar profile;layer ID;E ratio");
  gPad->SetGrid();
  Style::SetAxis(ESeedInL->GetHistogram());
  Style::SetMarker(ESeedInL,26,4);
  ESeedInL->Draw("same pl");

  double lCoG = Conf::evt_bgo->GetLayerIDOfCoG();
  TLine *li = new TLine(lCoG,0,lCoG,0.3);
  li->SetLineColor(kRed);
  li->SetLineWidth(2);
  li->Draw();

  double lTMax = Conf::evt_bgo->GetLayerIDOfMaxE();
  TLine *li2 = new TLine(lTMax,0,lTMax,0.3);
  li2->SetLineColor(kBlue);
  li2->SetLineWidth(2);
  li2->Draw();

  TLegend *l = Steer::GetNewLegend();
  l->AddEntry(eInL,"E ratio","p");
  l->AddEntry(ESeedInL,"E ratio of seed bar","p");
  l->Draw();

  Conf::can.push_back(new TCanvas("Display_trans"+name,"Display_Trans_"+name));
  TCanvas *c1 = Conf::can[Conf::can.size()-1];
  c1->Divide(2,2);
  c1->cd(1);
  TGraph *RMSInL = new TGraph(BGO_LayerNO,layerID,Conf::evt_bgo->fRMS);
  RMSInL->SetTitle("RMS profile;layer ID;RMS");
  gPad->SetGrid();
  Style::SetAxis(RMSInL->GetHistogram());
  Style::SetMarker(RMSInL,33,2);
  RMSInL->Draw();

  c1->cd(2);
  TGraph *FInL = new TGraph(BGO_LayerNO,layerID,Conf::evt_bgo->fFValue);
  FInL->SetTitle("F value profile ;layer ID;F");
  gPad->SetGrid();
  Style::SetAxis(FInL->GetHistogram());
  Style::SetMarker(FInL,29,4);
  FInL->Draw();

  c1->cd(3);
  TGraph *GInL = new TGraph(BGO_LayerNO,layerID,L_g);
  GInL->SetTitle("G value profile;layer ID;G");
  gPad->SetGrid();
  Style::SetAxis(GInL->GetHistogram());
  Style::SetMarker(GInL,28,6);
  GInL->Draw();
  //gStyle->SetOptStat(0);

  c1->cd(4);
  TGraph *FracInL = new TGraph(BGO_LayerNO,layerID,L_Fractal);
  FracInL->SetTitle("Fractal14;layer ID;F14");
  gPad->SetGrid();
  Style::SetAxis(FracInL->GetHistogram());
  Style::SetMarker(FracInL,23,3);
  FracInL->Draw();

  Conf::can.push_back(new TCanvas("Display_trans2"+name,"Display_Trans_"+name));
  TCanvas *c2 = Conf::can[Conf::can.size()-1];
  c2->Divide(2,2);

  c2->cd(1);
  TGraph *ERL3InL = new TGraph(BGO_LayerNO,layerID,L_ERL3);
  ERL3InL->SetTitle("ERL3 profile;layer ID;ERL3");
  gPad->SetGrid();
  Style::SetAxis(ERL3InL->GetHistogram());
  Style::SetMarker(ERL3InL,21,4);
  ERL3InL->Draw();
  //ESeedInL->Draw("same pl");

  c2->cd(2);
  TGraph *FBNoInL = new TGraph(BGO_LayerNO,layerID,L_firedBarNo);
  FBNoInL->SetTitle("fired bar No. profile;layer ID;fired bars");
  gPad->SetGrid();
  Style::SetAxis(FBNoInL->GetHistogram());
  Style::SetMarker(FBNoInL,27,4);
  FBNoInL->Draw();
}

void DrawEvent(TString selection)
{
  TTree *t = Conf::LinkTree()->CopyTree(selection);
  t->SetBranchAddress("Bgo",&Conf::evt_bgo);
  long entries = t->GetEntries();

  for(long i=0;i<entries;++i){
    DrawThisEvent(i);
  }

}
//-------------------------------------------------------------------
//-------------------------------------------------------------------


//-------------------------------------------------------------------
//-------------------------------------------------------------------
TProfile* ERatioProfile(TString selection,bool use2D=false)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-ERatio_In_Layer_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *ERatioInL = new TH2D(cName+"E ratio","E ratio in layer;layer ID;E ratio",14,0,14,200,0,1);

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      double eT = Conf::evt_bgo->GetTotalEnergy();
      for(int i=0;i<BGO_LayerNO;++i){
        if(Conf::evt_bgo->GetTotalEnergy(i) > 0){
          ERatioInL->Fill(i,Conf::evt_bgo->GetTotalEnergy(i)/eT);
        }
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *ERatioInL_proX = ERatioInL->ProfileX();
    ERatioInL_proX->GetYaxis()->SetTitle("E Ratio");
    Style::SetMarker(ERatioInL_proX,28,2,1);
    Style::SetAxis(ERatioInL_proX);
    gStyle->SetOptStat("");
    if(use2D){
      ERatioInL->Draw("colz");
    }else{
      ERatioInL_proX->Draw("e");
    }

    return ERatioInL_proX;
}

TProfile* EProfile(TString selection,bool use2D=false)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-E_In_Layer_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    double MaxE = 0;
    for(int i=0;i<10;++i){
      t->GetEntry(entries/(i+2) + i);
      double eT = Conf::evt_bgo->GetTotalEnergy();
      if(eT > MaxE) MaxE = eT;
    }

    TH2D *EInL = new TH2D(cName+"E","E in layer;layer ID;E / MeV",14,0,14,200,0,MaxE*0.5);

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      //double eT = Conf::evt_bgo->GetTotalEnergy();
      for(int i=0;i<BGO_LayerNO;++i){
        if(Conf::evt_bgo->GetTotalEnergy(i) > 0){
          EInL->Fill(i,Conf::evt_bgo->GetTotalEnergy(i));
        }
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *EInL_proX = EInL->ProfileX();
    EInL_proX->GetYaxis()->SetTitle("E / MeV");
    Style::SetMarker(EInL_proX,28,2,1);
    Style::SetAxis(EInL_proX);
    gStyle->SetOptStat("");
    if(use2D){
      EInL->Draw("colz");
    }else{
      EInL_proX->Draw("e");
    }

    return EInL_proX;
}

void ETransProfile(TString selection)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-E_Transvers";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    double MaxE = 0;
    for(int i=0;i<10;++i){
      t->GetEntry(entries/(i+2) + i);
      double eT = Conf::evt_bgo->GetTotalEnergy();
      if(eT > MaxE) MaxE = eT;
    }

    TH2D *EInBar_X = new TH2D(cName+"E_Bar_X","X layer(l0);Bar ID;E / MeV",22,0,22,200,0,MaxE*0.5);
    TH2D *EInBar_Y = new TH2D(cName+"E_Bar_Y","Y layer(l1);Bar ID;E / MeV",22,0,22,200,0,MaxE*0.5);

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      //double eT = Conf::evt_bgo->GetTotalEnergy();
      for(int b=0;b<BGO_BarNO;++b){
        double x=0,y=0;
        for(int i=0;i<BGO_LayerNO;++i){
          if(i%2 == 0){
            x += Conf::evt_bgo->GetEnergyOfBar(i,b);
          }else{
            y += Conf::evt_bgo->GetEnergyOfBar(i,b);
          }
        }
        EInBar_X->Fill(b,x);
        EInBar_Y->Fill(b,y);
      }
    }

    TCanvas *mc = new TCanvas(cName,cName);
    Conf::can.push_back(mc);
    mc->Divide(1,2);

    mc->cd(1);
    gPad->SetGrid();
    TProfile *ebx = EInBar_X->ProfileX();
    ebx->GetYaxis()->SetTitle("E / MeV");
    Style::SetMarker(ebx,28,2,1);
    Style::SetAxis(ebx);
    gStyle->SetOptStat("");
    ebx->Draw("e");

    mc->cd(2);
    gPad->SetGrid();
    TProfile *eby = EInBar_Y->ProfileX();
    eby->GetYaxis()->SetTitle("E / MeV");
    Style::SetMarker(eby,28,2,1);
    Style::SetAxis(eby);
    gStyle->SetOptStat("");
    eby->Draw("e");

    //return EInL_proX;
}

TProfile* RMSProfile(TString selection,bool use2D=false)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-RMS_In_Layer_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *RMSInL = new TH2D(cName+"RMS","RMS_layer;layer ID;RMS",14,0,14,11000,0,20);

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      for(int i=0;i<BGO_LayerNO;++i){
        if(Conf::evt_bgo->GetTotalEnergy(i) > 0){
          RMSInL->Fill(i,Conf::evt_bgo->fRMS[i]);
        }
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *RMSInL_proX = RMSInL->ProfileX();
    RMSInL_proX->GetYaxis()->SetTitle("RMS");
    Style::SetMarker(RMSInL_proX,23,2,1);
    Style::SetAxis(RMSInL_proX);
    gStyle->SetOptStat("");
    if(use2D){
      RMSInL->Draw("colz");
    }else{
      RMSInL_proX->Draw("e");
    }

    return RMSInL_proX;
}

TProfile* FProfile(TString selection,bool use2D=false)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-F_In_Layer_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *FInL = new TH2D(cName+"FValue","FValue_layer;layer ID; F",14,0,14,1600,0,16);

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      for(int i=0;i<BGO_LayerNO;++i){
        if(Conf::evt_bgo->GetTotalEnergy(i) > 0){
          FInL->Fill(i,Conf::evt_bgo->fFValue[i]);
        }
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *FInL_proX = FInL->ProfileX();
    FInL_proX->GetYaxis()->SetTitle("F Value");
    Style::SetMarker(FInL_proX,25,2,1);
    Style::SetAxis(FInL_proX);
    gStyle->SetOptStat("");
    if(use2D){
      FInL->Draw("colz");
    }else{
      FInL_proX->Draw("e");
    }

    return FInL_proX;
}

TProfile* GProfile(TString selection,bool use2D=false)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-G_In_Layer_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *GInL = new TH2D(cName+"GValue","GValue_layer;layer ID;G",14,0,14,500,-50,50);

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      for(int i=0;i<BGO_LayerNO;++i){
        if(Conf::evt_bgo->GetTotalEnergy(i) > 0){
          GInL->Fill(i,Conf::evt_bgo->GetGValue(i));
        }
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *GInL_proX = GInL->ProfileX();
    GInL_proX->GetYaxis()->SetTitle("G Value");
    Style::SetMarker(GInL_proX,27,2,1);
    Style::SetAxis(GInL_proX);
    gStyle->SetOptStat("");
    if(use2D){
      GInL->Draw("colz");
    }else{
      GInL_proX->Draw("e");
    }

    return GInL_proX;
}

TProfile* FractalProfile(TString selection,int nBar1=1,int nBar2=4,bool use2D=false)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-Fratcal_";
          cName +=nBar1;
          cName +=nBar2;
          cName +="_In_Layer_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *FraInL = new TH2D(cName+"Fractal",Form("F%d%d_layer;layer ID;F%d%d",nBar1,nBar2,nBar1,nBar2),14,0,14,100,0,1);

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      for(int i=0;i<BGO_LayerNO;++i){
        if(Conf::evt_bgo->GetTotalEnergy(i) > 0){
          FraInL->Fill(i,Conf::evt_bgo->GetFractal(i,nBar1,nBar2));
        }
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *FraInL_proX = FraInL->ProfileX();
    FraInL_proX->GetYaxis()->SetTitle(Form("F%d%d Value",nBar1,nBar2));
    Style::SetMarker(FraInL_proX,29,2,1);
    Style::SetAxis(FraInL_proX);
    gStyle->SetOptStat("");
    if(use2D){
      FraInL->Draw("colz");
    }else{
      FraInL_proX->Draw("e");
    }

    return FraInL_proX;
}

TProfile* FiredBarNoProfile(TString selection,double eCut = 2.5,bool use2D=false)// 2.5MeV for one bar
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-FiredBarNo_In_Layer_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *FNoInL = new TH2D(cName+"FiredBarNo","FiredBarNo;layer ID; Fired Bar No",14,0,14,16,0,16);

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      std::vector<int>  nbs = Conf::evt_bgo->GetFiredBarNumbers(eCut);
      for(int l=0;l<BGO_LayerNO;++l){
        FNoInL->Fill(l,nbs[l]);
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *FNoInL_proX = FNoInL->ProfileX();
    FNoInL_proX->GetYaxis()->SetTitle("Fired Bar Number");
    Style::SetMarker(FNoInL_proX,25,2,1);
    Style::SetAxis(FNoInL_proX);
    gStyle->SetOptStat("");
    if(use2D){
      FNoInL->Draw();
    }else{
      FNoInL_proX->Draw("e");
    }

    return FNoInL_proX;
}

TProfile* ERL3Profile(TString selection,int type=0,bool use2D=false)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-ERL3_In_Layer_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *ERL3InL = new TH2D(cName+"ERL3","ERL3;layer ID; ERL3",14,0,14,50,0,1.0);

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      for(int l=0;l<BGO_LayerNO;++l){
        ERL3InL->Fill(l,Conf::evt_bgo->GetERL3(l,type));
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *ERL3InL_proX = ERL3InL->ProfileX();
    ERL3InL_proX->GetYaxis()->SetTitle("ERL3");
    Style::SetMarker(ERL3InL_proX,25,2,1);
    Style::SetAxis(ERL3InL_proX);
    gStyle->SetOptStat("");
    if(use2D){
      ERL3InL->Draw();
    }else{
      ERL3InL_proX->Draw("e");
    }

    return ERL3InL_proX;
}

TProfile *TMaxZVSRecoE(TString selection,bool use2D =false)
{
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-tmax VS log(RecE)_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *TMax_LogE = new TH2D(cName+"TMax_LogE",";Log(E);TMax",1200,8,20,14*10,0,14);

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    for(int ievt=0;ievt<entries;++ievt){
      t->GetEntry(ievt);
      TMax_LogE->Fill(TMath::Log(Conf::evt_bgo->fTotE),Conf::evt_bgo->GetTMax());
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *TMax_LogE_proX = TMax_LogE->ProfileX();
    if(use2D){
      TMax_LogE->Draw("colz");
    }else{
      TMax_LogE_proX->GetYaxis()->SetTitle("TMax");
      Style::SetMarker(TMax_LogE_proX,25,2,1);
      Style::SetAxis(TMax_LogE_proX);
      gStyle->SetOptStat("");
      TMax_LogE_proX->Draw("e");
      }

    return TMax_LogE_proX;
}

TProfile *ERatioInLVSRecoE(TString selection,int layerID = -1,bool use2D = false)
{   // if layer id = -1, use DmpEvtBgoShower::GetERatioAtTail();
    TString cName = "c";
          cName +=Conf::can.size();
          cName +="-ERatio VS log(RecE)_";
          cName += selection;
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;

    TH2D *ER_LogE = new TH2D(cName+"ERatio_LogE",";Log(E);ERatio",1600,0,20,800,0,1.0);

    TTree *t = Conf::LinkTree()->CopyTree(selection);
    t->SetBranchAddress("Bgo",&Conf::evt_bgo);
    long entries = t->GetEntries();
    cout<<"\t\tchoosed events:\t"<<entries<<endl;

    if(layerID == -1){
      for(int ievt=0;ievt<entries;++ievt){
        t->GetEntry(ievt);
        ER_LogE->Fill(TMath::Log(Conf::evt_bgo->fTotE),Conf::evt_bgo->GetERatioAtTail());
      }
    }else{
      for(int ievt=0;ievt<entries;++ievt){
        t->GetEntry(ievt);
        ER_LogE->Fill(TMath::Log(Conf::evt_bgo->fTotE),Conf::evt_bgo->GetERatioOfLayer(layerID));
      }
    }

    Conf::can.push_back(new TCanvas(cName,cName));
    gPad->SetGrid();
    TProfile *ER_LogE_proX = ER_LogE->ProfileX();
    if(use2D){
      ER_LogE->Draw("colz");
    }else{
      ER_LogE_proX->GetYaxis()->SetTitle("E Ratio");
      Style::SetMarker(ER_LogE_proX,25,2,1);
      Style::SetAxis(ER_LogE_proX);
      gStyle->SetOptStat("");
      ER_LogE_proX->Draw("e");
    }

    return ER_LogE_proX;
}


//-------------------------------------------------------------------
//-------------------------------------------------------------------


//-------------------------------------------------------------------
void ESpectrum(TString cuts = Cut["T0"],int layer=-1, int barID=-1)
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
    name = "Bgo.GetEMaxBarInLayer(";
    name += layer;
    name += ")->fBar";
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

void Direction(TString cuts=Cut["T0"])
{
  gStyle->SetStatStyle();
  MyDraw("Bgo.GetTrackDirection().Theta()",cuts);
  MyDraw("Bgo.GetTrackDirection().Theta():Bgo.GetLayerIDOfMaxE()",cuts,"*");
  MyDraw("Bgo.GetTrackDirection().Theta():Bgo.fTotE",cuts,"*");
}

void EntryPoint(TString cuts = Cut["Mips2"])
{
  MyDraw("Bgo.GetEntryPoint().y():Bgo.GetEntryPoint().x()",cuts,"colz");
}

void EntryBarID(TString cuts = Cut["Mips2"]) 
{
  MyDraw("Bgo.GetCoGBarIDInLayer(0):Bgo.GetCoGBarIDInLayer(1)",cuts,"colz");
}

namespace MCTrack
{

//DmpEvtMCTrack  *holder =new DmpEvtMCTrack(); 

void DrawThisEventTrack(long evtID,double TrackECut=5,int trackID = -1)
{
  TString name = "EventID_";
          name += evtID;
          name += Form("ECut%f_TrkID%d",TrackECut,trackID);
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          name +="--"+tmp;
  TH2F *xz =  new TH2F("XZ_"+name,"XZ;Z;X",275,0,550,40,-40,40); // xz->GetXaxis()->SetTitle("layer ID");   xz->GetYaxis()->SetTitle("bar ID");
  Style::SetAxis(xz);
  TH2F *yz =  new TH2F("YZ_"+name,"YZ;Z;y",275,0,550,40,-40,40); // yz->GetXaxis()->SetTitle("layer ID");   yz->GetYaxis()->SetTitle("bar ID");
  Style::SetAxis(yz);

  Conf::LinkTree()->GetEntry(evtID);

  cout<<"\n\nEvent ID: "<<evtID<<endl;
  cout<<"\tprimary particle:\t"<<Conf::evt_mcPrim->PDGcode()<<"\t("<<Conf::evt_mcPrim->Position().x()<<","<<Conf::evt_mcPrim->Position().y()<<","<<Conf::evt_mcPrim->Position().z()<<")\ttheta = "<<Conf::evt_mcPrim->Direction().Theta()<<endl;
  cout<<"\tReconstructed E_BGO = "<<Conf::evt_bgo->fTotE/1000<<" GeV"<<endl;

  int nVertex = Conf::evt_mcVertex->fTrackID.size();
  for(int it = 0;it<nVertex;++it){
    if(Conf::evt_mcVertex->fEnergy.at(it) < TrackECut) continue;
    int prtID = Conf::evt_mcVertex->fTrackID.at(it);
    if( trackID == it || trackID == -1){
      for(int p=0;p<nVertex;++p){
        if(Conf::evt_mcVertex->fEnergy.at(p) < TrackECut) continue;
        if(Conf::evt_mcVertex->fParentID.at(p) == prtID){
          xz->Fill(Conf::evt_mcVertex->fPosition.at(p).z(),Conf::evt_mcVertex->fPosition.at(p).x());
          yz->Fill(Conf::evt_mcVertex->fPosition.at(p).z(),Conf::evt_mcVertex->fPosition.at(p).y());
        }
      }
    }
  }

  Conf::can.push_back(new TCanvas("Display_"+name,"Display_"+name));
  TCanvas *c0 = Conf::can[Conf::can.size()-1];
  gStyle->SetOptStat("");
  c0->Divide(1,2);
  c0->cd(1);
  //gPad->SetGrid();
  xz->Draw("colz");

  c0->cd(2);
  yz->Draw("colz");
}

void ParentPDGCode(int pdgCode, double z0 = 0,double z1 = 600,double eLow=0, long nEntries = 10000000, long firstE = 0)
{
    TString name = Form("parentID of SDPID = %d,in Z range %f-%f_eLowCut%f",pdgCode,z0,z1,eLow);
    Conf::can.push_back(new TCanvas("C"+name,name));
    TCanvas *c0 = Conf::can[Conf::can.size()-1];
  TH1F *h =new TH1F(name,";PDG code;count",500,-250,250);

  for(int ie = firstE;ie<nEntries;++ie){
    Conf::LinkTree()->GetEntry(ie);
    int nT = Conf::evt_mcVertex->fTrackID.size();
    for(int i=0;i<nT;++i){
      std::vector<int> back = Conf::evt_mcVertex->GetParentPDGCode(pdgCode,z0,z1,eLow);
      for(size_t j=0;j<back.size();++j){
        h->Fill(back.at(j));
      }
    }
  }
  c0->cd();
  h->Draw();
}

void EofEMaxLepton(TString selection="",double z0=0,double z1=600,long nEntries = 100000000,long firstE = 0)
{
    TString name = Form("EMax lepton,in Z range %f-%f",z0,z1);
    Conf::can.push_back(new TCanvas("C"+name,name));
    TCanvas *c0 = Conf::can[Conf::can.size()-1];
  TH1F *h22 =new TH1F(name+"gamma",";E / MeV;count",1000,0,300000);
  h22->SetLineColor(kRed);
  TH1F *h11 =new TH1F(name+"electron",";E / MeV;count",1000,0,300000);
  h11->SetLineColor(kBlue);
  TH1F *h_11 =new TH1F(name+"positron",";E / MeV;count",1000,0,300000);
  h_11->SetLineColor(kGreen);


    TTree *t = Conf::LinkTree()->CopyTree(selection,"",nEntries,firstE);
    t->SetBranchAddress("TrackVertex",&Conf::evt_mcVertex);

    long xxEnts = t->GetEntries();
  for(int ie = 0;ie<xxEnts;++ie){
    t->GetEntry(ie);
    double maxE=0;
    int code = -1;
    int nT = Conf::evt_mcVertex->fTrackID.size();
    for(int i=0;i<nT;++i){
      int pdgcode = Conf::evt_mcVertex->fPDGCode.at(i);
      double z = Conf::evt_mcVertex->fPosition.at(i).z();
      if((pdgcode == 22 || pdgcode == 11 || pdgcode == -11) && z > z0 && z < z1){
        double  e= Conf::evt_mcVertex->fEnergy.at(i);
        if( e > maxE){
          maxE = e;
          code = pdgcode;
        }
      }
    }
    if(code == 22){
      h22->Fill(maxE);
    }else if(code == 11){
      h11->Fill(maxE);
    }else if(code == -11){
      h_11->Fill(maxE);
    }
  }

  c0->cd();
  h22->Draw();
  h11->Draw("same");
  h_11->Draw("same");
  //gPad->SetGrid();
}

void TotalEOfLeptonFromHadron(TString selection="",double eLow = 0,double z0=0,double z1=600,long nEntries = 100000000,long firstE =0)
{
    TString name = Form("totalEOflepton_parentHadron,in Z range %f-%f",z0,z1);
    Conf::can.push_back(new TCanvas("C"+name,name));
    TCanvas *c0 = Conf::can[Conf::can.size()-1];
  TH1F *hTotE =new TH1F(name+"totalEof22_11_-11",";E / MeV;count",5000,0,300000);


    TTree *t = Conf::LinkTree()->CopyTree(selection,"",nEntries,firstE);
    t->SetBranchAddress("TrackVertex",&Conf::evt_mcVertex);

    DmpEvtMCTrack *tmp = new DmpEvtMCTrack();
    long xxEnts = t->GetEntries();
  for(int ie = 0;ie<xxEnts;++ie){
if(ie%500 == 0)std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
    t->GetEntry(ie);
    double toE=0;
    int nT = Conf::evt_mcVertex->fTrackID.size();
    for(int i=0;i<nT;++i){
      if(Conf::evt_mcVertex->fEnergy.at(i) < eLow) continue;
      int pdgcode = Conf::evt_mcVertex->fPDGCode.at(i);
      double z = Conf::evt_mcVertex->fPosition.at(i).z();
      if((pdgcode == 22 || pdgcode == 11 || pdgcode == -11) && (z > z0 && z<z1)){
        int prtID = Conf::evt_mcVertex->fParentID.at(i);
        Conf::evt_mcVertex->Tracking(tmp,prtID,true);
        pdgcode = tmp->fPDGCode.at(0);
        if(pdgcode == 22 || pdgcode == 11 || pdgcode == -11) continue;
        toE += Conf::evt_mcVertex->fEnergy.at(i);
      }
    }
    hTotE->Fill(toE);
  }

  c0->cd();
  hTotE->Draw();
  //gPad->SetGrid();
}

};


namespace Compare
{

TLegend* Style(TH1* ele, TH1 *pro,TString tagE,TString tagP)
{
  Style::SetMarker(ele,28,kBlue,1.5);
  Style::SetMarker(pro,29,kRed,1.5);

  TLegend *l = Plot::Steer::GetNewLegend();
  l->AddEntry(ele,tagE,"lp");
  l->AddEntry(pro,tagP,"lp");

  return l;
}

void CreateCanvas(TString tag,TString eF,TString pF,TString eCut,TString pCut)
{
  TString cName = "c";
        cName +=Conf::can.size();
        cName +="-"+tag+"_";
        cName += eCut;
        TString tmp = eF;
        tmp.Remove(0,tmp.Last('/')+1);
        tmp.Remove(tmp.Last('.'),tmp.Length());
        cName +="--"+tmp+"--";

        cName += pCut;
        tmp = pF;
        tmp.Remove(0,tmp.Last('/')+1);
        tmp.Remove(tmp.Last('.'),tmp.Length());
        cName +="--"+tmp;

  Conf::can.push_back(new TCanvas(cName,cName));
  gPad->SetGrid();
}

void ThisValue(TString expre,TString electronFile, TString hadronFile,TString electronCut,TString protonCut,TString tagE="150GeV electron",TString tagP = "400GeV proton")
{
  Plot::ResetInputFile(electronFile);
  MyDraw(expre,electronCut);
  Plot::ResetInputFile(hadronFile);
  MyDraw(expre,protonCut,"same");
//void MyDraw(TString exp, TString cuts= Cut["T0"], TString opt="")
}

TLegend* ERatioProfile(TString electronFile, TString hadronFile,TString electronCut,TString protonCut,TString tagE="150GeV electron",TString tagP = "400GeV proton")
{
  Plot::ResetInputFile(electronFile);
  TProfile *e=Plot::ERatioProfile(electronCut);
  Plot::ResetInputFile(hadronFile);
  TProfile *p=Plot::ERatioProfile(protonCut);

  CreateCanvas("E Profile",electronFile,hadronFile,electronCut,protonCut);
  TLegend *l = Style(e,p,tagE,tagP);

  e->Draw("e");
  p->Draw("esame");
  l->Draw();
  return l;
}

void RMSProfile(TString electronFile, TString hadronFile,TString electronCut,TString protonCut,TString tagE="150GeV electron",TString tagP = "400GeV proton")
{
  Plot::ResetInputFile(electronFile);
  TProfile *e=Plot::RMSProfile(electronCut);
  Plot::ResetInputFile(hadronFile);
  TProfile *p=Plot::RMSProfile(protonCut);

  CreateCanvas("RMS Profile",electronFile,hadronFile,electronCut,protonCut);
  TLegend *l =Style(e,p,tagE,tagP);

  e->Draw();
  p->Draw("same");
  l->Draw();
}

void GProfile(TString electronFile, TString hadronFile,TString electronCut,TString protonCut,TString tagE="150GeV electron",TString tagP = "400GeV proton")
{
  Plot::ResetInputFile(electronFile);
  TProfile *e=Plot::GProfile(electronCut);
  Plot::ResetInputFile(hadronFile);
  TProfile *p=Plot::GProfile(protonCut);

  CreateCanvas("G Profile",electronFile,hadronFile,electronCut,protonCut);
  TLegend *l =Style(e,p,tagE,tagP);

  e->Draw();
  p->Draw("same");
  l->Draw();
}

void FProfile(TString electronFile, TString hadronFile,TString electronCut,TString protonCut,TString tagE="150GeV electron",TString tagP = "400GeV proton")
{
  Plot::ResetInputFile(electronFile);
  TProfile *e=Plot::FProfile(electronCut);
  Plot::ResetInputFile(hadronFile);
  TProfile *p=Plot::FProfile(protonCut);

  CreateCanvas("F Profile",electronFile,hadronFile,electronCut,protonCut);
  TLegend *l =Style(e,p,tagE,tagP);

  p->Draw();
  e->Draw("same");
  l->Draw();
}

void FractalProfile(TString electronFile, TString hadronFile,TString electronCut,TString protonCut,TString tagE="150GeV electron",TString tagP = "400GeV proton",int nBar1=1,int nBar2=4)
{
  Plot::ResetInputFile(electronFile);
  TProfile *e=Plot::FractalProfile(electronCut,nBar1,nBar2);
  Plot::ResetInputFile(hadronFile);
  TProfile *p=Plot::FractalProfile(protonCut,nBar1,nBar2);

  CreateCanvas(Form("F%d%d Profile",nBar1,nBar2),electronFile,hadronFile,electronCut,protonCut);
  TLegend *l =Style(e,p,tagE,tagP);

  e->Draw();
  p->Draw("same");
  l->Draw();
}

};

namespace Isolate
{
TH1D* ESpectrumOfIsolatedBar(TString selection="",long nEntries=100000000,long firstE=0)
{
  TString cName = "c";
          cName +=Conf::can.size();
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
          cName +="--IsolatedBarsSpectrum";
          cName += selection;

  std::vector<TH1D*> h;
  for(int l=0;l<14;++l){
    h.push_back(new TH1D(Form("layer_%d",l),";Energy / MeV;counts",200,0,200));
    h[l]->SetLineColor(l+2);
    h[l]->SetLineWidth(2);
    h[l]->SetLabelSize(0.05,"X");
    h[l]->SetLabelSize(0.05,"Y");
  }

  TTree *t = Conf::LinkTree()->CopyTree(selection,"",nEntries,firstE);
  t->SetBranchAddress("Bgo",&Conf::evt_bgo);
  long entries = t->GetEntries();
  cout<<"\t\tchoosed events:\t"<<entries<<endl;

  std::vector<int> layerID;
  layerID.push_back(-1);
  for(long ie=0;ie<entries;++ie){
    t->GetEntry(ie);
    for(int l=0;l<14;++l){
      layerID[0] = l;
      std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBar(layerID);
      for(unsigned int i=0;i<isoBars.size();++i){
        h[l]->Fill(isoBars[i]->fE);
      }
    }
  }

  Conf::can.push_back(new TCanvas(cName+"-a",cName+"a"));
   TH1D  *hAll = new TH1D(cName+"AllLayer",";Energy / MeV;counts",200,0,200);
    hAll->SetLineWidth(2);
    hAll->SetLabelSize(0.05,"X");
    hAll->SetLabelSize(0.05,"Y");
  //h[0]->Draw();
  for(int l=0;l<14;++l){
    h[l]->Draw(l==0 ? "" : "same");
    hAll->Add(h[l]);
  }

  return hAll;
}

TH1D* TotalEOfIsolatedBar(TString selection="",double eLow = 23,long nEntries=100000000,long firstE=0)
{
  TString cName = "c";
          cName +=Conf::can.size();
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
          cName +="--IsolatedBarsNumber";
          cName += selection;
          cName += eLow;

  std::vector<TH1D*> h;
  for(int l=0;l<14;++l){
    h.push_back(new TH1D(Form("layer_%d",l),";isolated bar number;counts",500,0,1000));
    h[l]->SetLineColor(l+2);
    h[l]->SetLineWidth(2);
    h[l]->SetLabelSize(0.05,"X");
    h[l]->SetLabelSize(0.05,"Y");
  }

  TTree *t = Conf::LinkTree()->CopyTree(selection,"",nEntries,firstE);
  t->SetBranchAddress("Bgo",&Conf::evt_bgo);
  long entries = t->GetEntries();
  cout<<"\t\tchoosed events:\t"<<entries<<endl;

  std::vector<int> layerID;
  layerID.push_back(-1);
  for(long ie=0;ie<entries;++ie){
    t->GetEntry(ie);
    for(int l=0;l<14;++l){
      layerID[0] = l;
      std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBar(layerID,eLow,200);
      double TotE = 0;
      for(size_t ib=0;ib<isoBars.size();++ib){
        TotE += isoBars.at(ib)->fE;
      }
      h[l]->Fill(TotE);
    }
  }

  Conf::can.push_back(new TCanvas(cName+"-a",cName+"a"));
   TH1D  *hAll = new TH1D(cName+"AllLayer",";isolated bar number;counts",500,0,1000);
    hAll->SetLineWidth(2);
    hAll->SetLabelSize(0.05,"X");
    hAll->SetLabelSize(0.05,"Y");
  //h[0]->Draw();
  for(int l=0;l<14;++l){
    h[l]->Draw(l==0 ? "" : "same");
    hAll->Add(h[l]);
  }

  return hAll;
}

void Distance2CoG(TString selection="", double E_low = 20,double E_high = 40,double Noise_E0 = 2,long nEntries = 10000000, double firstE = 0 )    // Bar E < E0 is noise, low
{
  TString cName = Form("c%d-Distance2CoGDevelopment-",(int)Conf::can.size());
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
          cName +="_"+selection;
  TString bar_ERange = Form("E Range of Bar%f to %f",E_low,E_high);

  TString  title = "BarID-CoG VS layer ID-";

  TH2F *h2D_CoGDis_LID = new TH2F(title+bar_ERange+tmp,title+bar_ERange+";Layer ID;Distance to CoG",28,0,14,80,-20,20);
  h2D_CoGDis_LID->SetLabelSize(0.05,"X");
  h2D_CoGDis_LID->SetLabelSize(0.05,"Y");
  h2D_CoGDis_LID->SetTitleSize(0.045,"X");
  h2D_CoGDis_LID->SetTitleSize(0.05,"Y");
  h2D_CoGDis_LID->SetTitleOffset(0.90,"X");
  h2D_CoGDis_LID->SetTitleOffset(0.75,"Y");

  TTree *t = Conf::LinkTree()->CopyTree(selection,"",nEntries,firstE);
  t->SetBranchAddress("Bgo",&Conf::evt_bgo);
  long entries = t->GetEntries();
  cout<<"\t\tchoosed events:\t"<<entries<<endl;

  for(long ievt=0;ievt<Conf::nEvts;++ievt){
    t->GetEntry(ievt);
    for(int l=0;l<BGO_LayerNO;++l){
      std::vector<int>  lxx; lxx.push_back(l);
      std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBar(lxx,E_low,E_high,Noise_E0);
      int nB = isoBars.size();
      double  CoG = Conf::evt_bgo->GetCoGBarIDInLayer(l);
      for(int ib = 0;ib<nB;++ib){
        h2D_CoGDis_LID->Fill(l,isoBars[ib]->fBar - CoG,isoBars[ib]->fE);
      }
    }
  }

  cName += "-"+bar_ERange;
  TCanvas *c1 = new TCanvas(cName+"a",cName+"a");
  Conf::can.push_back(c1);
  gPad->SetGrid();
  gPad->SetLogz();
  gStyle->SetOptStat(0);
  h2D_CoGDis_LID->Draw("colz");

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
 
  std::vector<int>  lIDs;
  for(int l=fromLayerID;l<BGO_LayerNO;++l ){
    lIDs.push_back(l);
  }

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
    std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBar(lIDs,Noise_E0);
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

  std::vector<int>  lIDs;
  for(int l=layerCut;l<BGO_LayerNO;++l ){
    lIDs.push_back(l);
  }

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
    //if(Cuts::IsVerticalMips()){
    //  continue;
    //}

      std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBar(lIDs,eCut);
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

void LongitudinalDevelopment(TString selection="", double E_low = 20,double E_high = 40,double Noise_E0 = 2,long nEntries = 10000000, double firstE = 0 )    // Bar E < E0 is noise, low
{
  TString cName = Form("c%d-IsolateLayerDevelopment-",(int)Conf::can.size());
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
          cName +="_"+selection;
  TString bar_ERange = Form("E Range of Bar%f to %f",E_low,E_high);

  TString  title = "BarID-CoG VS layer ID-";

  TH2F *h2D_CoGDis_LID = new TH2F(title+bar_ERange+tmp,title+bar_ERange+";Layer ID;Bar ID - CoG",28,0,14,80,-20,20);
  h2D_CoGDis_LID->SetLabelSize(0.05,"X");
  h2D_CoGDis_LID->SetLabelSize(0.05,"Y");

  title = "Mean E VS layer ID-";
  TH2F *h2D_MeanE_LID = new TH2F(title+bar_ERange+tmp,title+bar_ERange+";Layer ID;Mean E / MeV",28,0,14,E_high,0,E_high);
  h2D_MeanE_LID->SetLabelSize(0.05,"X");
  h2D_MeanE_LID->SetLabelSize(0.05,"Y");

  title = "E RMS VS layer ID-";
  TH2F *h2D_ERMS_LID = new TH2F(title+bar_ERange+tmp,title+bar_ERange+";Layer ID;RMS of E / MeV",28,0,14,60,0,30);
  h2D_ERMS_LID->SetLabelSize(0.05,"X");
  h2D_ERMS_LID->SetLabelSize(0.05,"Y");

  title = "NBars VS layer ID-";
  TH2F *h2D_NBars_LID = new TH2F(title+bar_ERange+tmp,title+bar_ERange+";Layer ID;Bar number",28,0,14,50,0,25);
  h2D_NBars_LID->SetLabelSize(0.05,"X");
  h2D_NBars_LID->SetLabelSize(0.05,"Y");

  TTree *t = Conf::LinkTree()->CopyTree(selection,"",nEntries,firstE);
  t->SetBranchAddress("Bgo",&Conf::evt_bgo);
  long entries = t->GetEntries();
  cout<<"\t\tchoosed events:\t"<<entries<<endl;

  for(long ievt=0;ievt<Conf::nEvts;++ievt){
    t->GetEntry(ievt);
    for(int l=0;l<BGO_LayerNO;++l){
      std::vector<int>  lxx; lxx.push_back(l);
      std::vector<DmpBgoFiredBar*>  isoBars = Conf::evt_bgo->GetIsolatedBar(lxx,E_low,E_high,Noise_E0);
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

  cName += "-"+bar_ERange;
  TCanvas *c1 = new TCanvas(cName+"a",cName+"a");
  Conf::can.push_back(c1);
  c1->Divide(2,2);
  c1->cd(1); gPad->SetGrid();   h2D_NBars_LID->Draw("colz");
  c1->cd(2); gPad->SetGrid();   h2D_MeanE_LID->Draw("colz");//   gPad->SetLogy();
  c1->cd(3); gPad->SetGrid();   h2D_CoGDis_LID->Draw("colz");
  c1->cd(4); gPad->SetGrid();   h2D_ERMS_LID->Draw("colz");// gPad->SetLogy();

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
    std::vector<int> lxx;
    for(int l=0;l<BGO_LayerNO;++l){
      lxx.push_back(l);
    }
    std::vector<DmpBgoFiredBar*>  allEvt = Conf::evt_bgo->GetIsolatedBar(lxx,Noise_E0);
    double allIsoBars = allEvt.size();
    h1D_TotNO->Fill(allIsoBars);
    for(int l=0;l<BGO_LayerNO;++l){
      std::vector<int>  l2l;l2l.push_back(l);
      std::vector<DmpBgoFiredBar*>  allIsoL = Conf::evt_bgo->GetIsolatedBar(l2l,Noise_E0);
      int nal = allIsoL.size();
      h2D_TotNo_LID->Fill(l,nal / allIsoBars);
      std::vector<DmpBgoFiredBar*>  allIsoLL20MeV = Conf::evt_bgo->GetIsolatedBar(l2l,Noise_E0,20,Noise_E0);
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

namespace Cluster
{

TH1D* ESpectrum(TString selection="",long nEntries=100000000,long firstE=0)//,double eMax=80000)
{
  TString cName = "c";
          cName +=Conf::can.size();
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
          cName +="--ClusterESpectrum";
          cName += selection;

  TTree *t = Conf::LinkTree()->CopyTree(selection,"",nEntries,firstE);
  t->SetBranchAddress("Bgo",&Conf::evt_bgo);
  long entries = t->GetEntries();
  cout<<"\t\tchoosed events:\t"<<entries<<endl;

  std::vector<TH1D*> h;
  t->GetEntry(20);  // 20 is any number
  double eMax = Conf::evt_bgo->GetEMaxCluster()->fTotE * 1.5;
  for(int l=0;l<14;++l){
    h.push_back(new TH1D(cName+Form("layer_%d",l),";Energy / MeV;counts",1000,0,eMax));
    h[l]->SetLineColor(l+2);
    h[l]->SetLineWidth(2);
    h[l]->SetLabelSize(0.05,"X");
    h[l]->SetLabelSize(0.05,"Y");
  }

  for(long ie=0;ie<entries;++ie){
    t->GetEntry(ie);
    for(int l=0;l<14;++l){
      std::vector<DmpEvtBgoCluster*>  clus = Conf::evt_bgo->GetAllClusterInLayer(l);
      for(unsigned int i=0;i<clus.size();++i){
        h[l]->Fill(clus.at(i)->fTotE);
      }
    }
  }

  Conf::can.push_back(new TCanvas(cName+"-a",cName+"a"));
   TH1D  *hAll = new TH1D(cName+"AllLayer",";Energy / MeV;counts",1000,0,eMax);
    hAll->SetLineWidth(2);
    hAll->SetLabelSize(0.05,"X");
    hAll->SetLabelSize(0.05,"Y");
  //h[0]->Draw();
  for(int l=0;l<14;++l){
    h[l]->Draw(l==0 ? "" : "same");
    hAll->Add(h[l]);
  }

  return hAll;
}

TH1D* BarNumber(TString selection="",double eLowCluster = 0, double eLowCut_Bar = 0,long nEntries=100000000,long firstE=0)//,double eMax=80000)
{
  TString cName = "c";
          cName +=Conf::can.size();
          TString tmp = Conf::inputFileName[Conf::inputFileName.size()-1];
          tmp.Remove(0,tmp.Last('/')+1);
          tmp.Remove(tmp.Last('.'),tmp.Length());
          cName +="--"+tmp;
          cName +="--ClusterSize";
          cName += selection;

  TTree *t = Conf::LinkTree()->CopyTree(selection,"",nEntries,firstE);
  t->SetBranchAddress("Bgo",&Conf::evt_bgo);
  long entries = t->GetEntries();
  cout<<"\t\tchoosed events:\t"<<entries<<endl;

  std::vector<TH1D*> h;
  for(int l=0;l<14;++l){
    h.push_back(new TH1D(cName+Form("layer_%d",l),";Bar Number;counts",20,0,20));
    h[l]->SetLineColor(l+2);
    h[l]->SetLineWidth(2);
    h[l]->SetLabelSize(0.05,"X");
    h[l]->SetLabelSize(0.05,"Y");
  }

  for(long ie=0;ie<entries;++ie){
    t->GetEntry(ie);
    for(int l=0;l<14;++l){
      std::vector<DmpEvtBgoCluster*>  clus = Conf::evt_bgo->GetAllClusterInLayer(l,eLowCluster);
      for(unsigned int i=0;i<clus.size();++i){
        h[l]->Fill(clus.at(i)->GetBarNumber(eLowCut_Bar));
      }
    }
  }

  Conf::can.push_back(new TCanvas(cName+"-a",cName+"a"));
   TH1D  *hAll = new TH1D(cName+"AllLayer",";Bar Number",20,0,20);
    hAll->SetLineWidth(2);
    hAll->SetLabelSize(0.05,"X");
    hAll->SetLabelSize(0.05,"Y");
  //h[0]->Draw();
  for(int l=0;l<14;++l){
    h[l]->Draw(l==0 ? "" : "same");
    hAll->Add(h[l]);
  }

  return hAll;
}

};

};

#endif

