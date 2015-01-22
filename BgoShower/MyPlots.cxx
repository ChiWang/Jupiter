#include <vector>
//#include "TH2F.h"
#define  __treeName "/Event/Rec0"

void MyPlots()
{
  //gSystem->Load("$DMPSWSYS/lib/libDmpBase.so");
  //gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
  gSystem->Load("$DMPSWWORK/lib/libDmpEvtBgoShower.so");
  //gInterpreter->AddIncludePath("$DMPSWSYS/include");
  //gInterpreter->AddIncludePath("$DMPSWWORK/include");
}

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
};

void PrintInputFile()
{
  for(int i =0;i<Conf::inputFileName.size();++i){
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
}

TChain *LinkTree()
{
  if(Conf::chain == 0 && Conf::inputFileName.size()){
    Conf::chain  = new TChain(__treeName);
    for(int i = 0;i<Conf::inputFileName.size();++i){
      Conf::chain->AddFile(Conf::inputFileName[i]);
    }
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

std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
  LinkTree()->Draw(exp,cuts,opt);
std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
}

namespace Plot
{
void EnergySpectrum(int layer=-1, int barID=-1, TCut cuts=Cuts::GlobalCut)
{
        // (-1,-1) total. (layer, -1) each bar in this layer. (-1,bar) this barID in each layer
  if(layer < 0 && barID < 0){
    MyDraw("Bgo.fTotE",cuts);
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

    DmpEvtBgoShower *Bgo = 0;
    LinkTree()->SetBranchAddress("Bgo",&Bgo);
    LinkTree()->GetEntry(evtID);

    for(int ic = 0;ic < Bgo->fClusters->GetEntriesFast();++ic){
      DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(Bgo->fClusters->At(ic));
      int lid = aC->fLayer;
      for(int b = 0;b<aC->fFiredBar->GetEntriesFast();++b){
        DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(aC->fFiredBar->At(b));
        if(lid % 2 == 0){
          yz->Fill(lid,aB->fBar,aB->fE/Bgo->fTotE);
        }else{
          xz->Fill(lid,aB->fBar,aB->fE/Bgo->fTotE);
        }
      }
    }

    Conf::can.push_back(new TCanvas("Display_"+name,"Display_"+name));
    TCanvas *c0 = Conf::can[Conf::can.size()-1];
    c0->Divide(2,1);
    c0->cd(1); gPad->SetGrid(); xz->Draw("colz");
    c0->cd(2); gPad->SetGrid(); yz->Draw("colz");
  }

};


