
#include <vector>

/*
 *  root -l RMSProfileCom.cxx
 *
 *
 *  Plot::RestInputFile(xx)
 *
 *  Plott::AddInpputFile(xx)
 *
 *  CompareRMS()
 *
 */

void RMSProfileCom(){
  gSystem->Load("$DMPSWWORK/lib/libMyplot.so");
}

Void CompareRMS(TCut cuts){ // cut energy range
  vector<TProfile*>     profi;
  int nFile = Plot::Conf::inputFileName.size();
  TLegend *leg = new TLegend(0.6,0.65,0.88,0.85);
  leg->SetTextFont(72);
  leg->SetTextSize(0.04);
  for(int i = 0;i<nFile;++i){
    TString name = Plot::Conf::inputFileName[i];
    Plot::ResetInputFile(name);
    TProfile *f = 0;
    if(name.Contains("pion")){
      f = Plot::RMSFValueProfile(true,cuts);
    }else{
      f = Plot::RMSFValueProfile();
    }
    f->SetMarkerSize();
    f->SetMarkerStyle(20+i);
    f->SetMarkerColor(1+i);
    f->SetLineColor(1+i);
    profi.push_back(f);
    name.Remove(0,name.First('_')+1);
    name.Remove(name.First('_'),name.Length());
    leg->AddEntry(f,name);
  }
  TCanvas *c0 = new TCanvas();
  profi[0]->DrawNormalized();
  for(int i=1;i<nFile;++i){
    profi[i]->DrawNormalized("same");
  }
}

