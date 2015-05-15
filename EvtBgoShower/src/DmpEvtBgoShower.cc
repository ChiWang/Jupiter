/*   $Id: DmpEvtBgoShower.cc, 2015-01-15 16:40:51 DAMPE $
 *--------------------------------------------------------
 *  Author(s):
 *
 *--------------------------------------------------------
*/

//#include <time.h>

#include <math.h>
#include <iostream>
#include <algorithm>
#include "TH2D.h"
#include "TF1.h"
#include "TMath.h"
#include "DmpEvtBgoShower.h"

ClassImp(DmpBgoFiredBar)
ClassImp(DmpEvtBgoCluster)
ClassImp(DmpEvtBgoShower)

//-------------------------------------------------------------------
DmpBgoFiredBar::DmpBgoFiredBar()
{
  Reset();
}

//-------------------------------------------------------------------
DmpBgoFiredBar::DmpBgoFiredBar(int l,int b,double e,double e0,double e1):fLayer(l),fBar(b),fE(e),fE0(e0),fE1(e1)
{
  if(e0 == 0 || e1 ==0){
    std::cout<<"+++++--> one side readout:\t";
    this->MyPrint();
    //string ask = "";
    //std::cout<<"Continue?   (y/No)";
    //cin>>ask;
    //if(ask == "No"){
    //  throw;
    //}
  }
  fPosition.SetXYZ(0,0,0);
}

//-------------------------------------------------------------------
DmpBgoFiredBar::DmpBgoFiredBar(int l,int b,double e,double e0,double e1,TVector3 v):fLayer(l),fBar(b),fE(e),fE0(e0),fE1(e1),fPosition(v)
{
}

//-------------------------------------------------------------------
DmpBgoFiredBar::~DmpBgoFiredBar()
{
}

//-------------------------------------------------------------------
void DmpBgoFiredBar::Reset()
{
  fLayer = 0;
  fBar = 0;
  fE = 0;
  fE0 = 0;
  fE1 = 0;
  fPosition.SetXYZ(0,0,0);
}

//-------------------------------------------------------------------
void DmpBgoFiredBar::LoadFrom(const DmpBgoFiredBar *&r)
{
  fLayer = r->fLayer;
  fBar = r->fBar;
  fE = r->fE;
  fE0 = r->fE0;
  fE1 = r->fE1;
  fPosition = r->fPosition;
}

void DmpBgoFiredBar::MyPrint()const
{
  std::cout<<"\tl="<<fLayer<<"\tb="<<fBar<<"\tE=("<<fE<<", "<<fE0<<", "<<fE1<<")\tPosi=("<<fPosition.x()<<", "<<fPosition.y()<<", "<<fPosition.z()<<")"<<std::endl;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
DmpEvtBgoCluster::DmpEvtBgoCluster()
{
  fFiredBar = new TClonesArray("DmpBgoFiredBar",5);
  Reset();
}

//-------------------------------------------------------------------
DmpEvtBgoCluster::DmpEvtBgoCluster(int l,int s,double e):fLayer(l),fSeedBarID(s),fTotE(e)
{
  fFiredBar = new TClonesArray("DmpBgoFiredBar",5);
}

//-------------------------------------------------------------------
DmpEvtBgoCluster::~DmpEvtBgoCluster()
{
  Reset();
  delete fFiredBar;
}

//-------------------------------------------------------------------
void DmpEvtBgoCluster::Reset()
{
  fLayer = 0;
  fSeedBarID = 0;
  fTotE = 0;
  fCenter.SetXYZ(0.,0.,0.);
  if(fFiredBar->GetEntriesFast()){
    fFiredBar->Delete();
  }
}

//-------------------------------------------------------------------
void DmpEvtBgoCluster::LoadFrom(const DmpEvtBgoCluster *&r)
{
  Reset();
  fLayer = r->fLayer;
  fSeedBarID = r->fSeedBarID;
  fTotE = r->fTotE;
  fCenter = r->fCenter;
  for(int i=0;i<r->fFiredBar->GetEntriesFast();++i){
    DmpBgoFiredBar *lef = dynamic_cast<DmpBgoFiredBar*>(fFiredBar->New(i));
    const DmpBgoFiredBar *rig = dynamic_cast<DmpBgoFiredBar*>(r->fFiredBar->At(i));
    lef->LoadFrom(rig);
  }
}

//-------------------------------------------------------------------
int DmpEvtBgoCluster::GetClusterSize()const
{
  return fFiredBar->GetEntriesFast();
}

//-------------------------------------------------------------------
DmpBgoFiredBar *DmpEvtBgoCluster::GetSeedBar()const
{
  DmpBgoFiredBar *seedBar = 0;
  double max_e = 0;
  for(int i=0;i<fFiredBar->GetEntriesFast();++i){
    DmpBgoFiredBar *it = dynamic_cast<DmpBgoFiredBar*>(fFiredBar->At(i));
    if(it->fE > max_e){
      seedBar = it;
      max_e = seedBar->fE;
    }
  }
  if(seedBar){
    seedBar = (seedBar->fBar == fSeedBarID) ? seedBar : 0;
  }
  return seedBar;
}

//-------------------------------------------------------------------
double DmpEvtBgoCluster::GetWindowEnergy(int len)const
{
  double e = 0;
  int n = fFiredBar->GetEntriesFast();
  for(int i=0;i<n;++i){
    DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(fFiredBar->At(i));
    if(TMath::Abs(aB->fBar - fSeedBarID) <= len){
      e += aB->fE;
    }
  }
  return e;
}

double DmpEvtBgoCluster::GetERatioOfSeedBar(int len)const
{
  return this->GetSeedBar()->fE / this->GetWindowEnergy(len);
}

//-------------------------------------------------------------------
double DmpEvtBgoCluster::GetCoGBarID()const
{
  double v=0;
  for(int i=0;i<fFiredBar->GetEntriesFast();++i){
    DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(fFiredBar->At(i));
    v += aB->fE * (aB->fBar+1);
  }
  v = v / fTotE;
  return (v-1.);
}

double DmpEvtBgoCluster::GetTotalEnergy(int whichSide)const
{
  if(whichSide == -1){
    return fTotE;
  }
  double e = 0;
  for(int i=0;i<fFiredBar->GetEntriesFast();++i){
    DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(fFiredBar->At(i));
    if(whichSide == 0){
      e += aB->fE0;
    }else if(whichSide == 1){
      e += aB->fE1;
    }
  }
  return e;
}

int DmpEvtBgoCluster::GetFiredBarNumber(double e_low)const
{
  int n=0;
  for(int i=0;i<fFiredBar->GetEntriesFast();++i){
    if((dynamic_cast<DmpBgoFiredBar*>(fFiredBar->At(i)))->fE > e_low) ++n;
  }
  return n;
}

void DmpEvtBgoCluster::AddNewFiredBar(DmpBgoFiredBar *ab)
{
  fCenter = fCenter * fTotE + ab->fPosition * ab->fE;
  fTotE += ab->fE;
  fCenter *= (1/fTotE);
  DmpBgoFiredBar *newb = dynamic_cast<DmpBgoFiredBar*>(fFiredBar->New(fFiredBar->GetEntriesFast()));
  const DmpBgoFiredBar *rit = ab;
  newb->LoadFrom(rit);
  //std::cout<<"\tTotE = "<<fTotE<<"\t\tCenter: "<<fCenter.x()<<"\t"<<fCenter.y()<<"\t"<<fCenter.z()<<std::endl;
}

//-------------------------------------------------------------------
void DmpEvtBgoCluster::MyPrint()const
{
  std::cout<<"l="<<fLayer<<"\tseed Bar="<<fSeedBarID<<"\tE="<<fTotE<<"\tPosi=("<<fCenter.x()<<", "<<fCenter.y()<<", "<<fCenter.z()<<std::endl;
  for(int i=0;i<fFiredBar->GetEntriesFast();++i){
    (dynamic_cast<DmpBgoFiredBar*>(fFiredBar->At(i)))->MyPrint();
  }
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
DmpEvtBgoShower::DmpEvtBgoShower()
{
  fClusters = new TClonesArray("DmpEvtBgoCluster",30);
  Reset();
}

//-------------------------------------------------------------------
DmpEvtBgoShower::~DmpEvtBgoShower()
{
  Reset();
  delete fClusters;
}

//-------------------------------------------------------------------
void DmpEvtBgoShower::Reset()
{
  fTotE = 0.;
  fLRMS = 0.;
  for(int i=0;i<BGO_LayerNO;++i){
    fRMS[i] = -1;
    fFValue[i] = -1.;
  }
  for(int i=0;i<5;++i){
    fXZFitPar[i] = 0;
    fYZFitPar[i] = 0;
  }
  if(fClusters->GetEntriesFast()){
    fClusters->Delete();
  }
}

//-------------------------------------------------------------------
void DmpEvtBgoShower::LoadFrom(const DmpEvtBgoShower *&r)
{
  Reset();
  fTotE = r->fTotE;
  fLRMS = r->fLRMS;
  for(int i=0;i<BGO_LayerNO;++i){
    fRMS[i] = r->fRMS[i];
    fFValue[i] = r->fFValue[i];
  }
  for(int i=0;i<5;++i){
    fXZFitPar[i] = r->fXZFitPar[i];
    fYZFitPar[i] = r->fYZFitPar[i];
  }
  for(int i=0;i<r->fClusters->GetEntriesFast();++i){
    DmpEvtBgoCluster *lef = dynamic_cast<DmpEvtBgoCluster*>(fClusters->New(i));
    const DmpEvtBgoCluster *rig = dynamic_cast<DmpEvtBgoCluster*>(r->fClusters->At(i));
    lef->LoadFrom(rig);
  }
}

//-------------------------------------------------------------------
DmpEvtBgoCluster* DmpEvtBgoShower::GetSeedCluster()const
{
  DmpEvtBgoCluster *seedClu =0;
  double max_e = 0;
  for(int i=0;i<fClusters->GetEntriesFast();++i){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(i));
    if(aC->fTotE > max_e){
      seedClu = aC;
      max_e = aC->fTotE;
    }
  }
  return seedClu;
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetTotalEnergy(int layerid, int whichSide)const
{
  double e = 0.;
  for(int i=0;i<fClusters->GetEntriesFast();++i){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(i));
    if(layerid == aC->fLayer || layerid == -1){
      e += aC->GetTotalEnergy(whichSide);
    }
  }
  return e;
}

double DmpEvtBgoShower::GetEnergyOfBar(int l,int barID)const
{
  std::vector<DmpEvtBgoCluster*>  cluInL = this->GetAllClusterInLayer(l);
  for(int i=0;i<cluInL.size();++i){
    for(int b=0;b<cluInL[i]->fFiredBar->GetEntriesFast();++b){
      DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(cluInL[i]->fFiredBar->At(b));
      if(aB->fBar == barID){
        return aB->fE;
      }
    }
  }
  return -99;
}

//-------------------------------------------------------------------
int DmpEvtBgoShower::GetLayerIDOfMaxE()const
{
  double max_e = 0.;
  int id = -1;
  for(int i=0;i<BGO_LayerNO;++i){
    double ei = this->GetTotalEnergy(i);
    if(ei > max_e){
      max_e = ei;
      id = i;
    }
  }
  return id;
}

int DmpEvtBgoShower::GetLayerIDOfMinE()const
{
  double min_e = 99999999;
  int id = -1;
  for(int i=0;i<BGO_LayerNO;++i){
    double ei = this->GetTotalEnergy(i);
    if(ei < min_e && ei > 23){
      min_e = ei;
      id = i;
    }
  }
  return id;
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetEnergyOfEMaxLayer()const
{
  return this->GetTotalEnergy(this->GetLayerIDOfMaxE());
}

double DmpEvtBgoShower::GetEnergyOfEMinLayer()const
{
  return this->GetTotalEnergy(this->GetLayerIDOfMinE());
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetEnergyRatioOfEMaxLayer()const
{
  return this->GetEnergyOfEMaxLayer() / fTotE;
}

double DmpEvtBgoShower::GetEnergyRatioOfEMinLayer()const
{
  return this->GetEnergyOfEMinLayer() / fTotE;
}

double DmpEvtBgoShower::GetFractalOfMaxMinLayer()const//LayerIDOfMaxMinLayer()const
{
  return TMath::Log10(this->GetEnergyRatioOfEMaxLayer()) / TMath::Log10(this->GetEnergyRatioOfEMinLayer());
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetEnergyRatioOfLayer(int l)const
{
  return this->GetTotalEnergy(l) / fTotE;
}


//-------------------------------------------------------------------
DmpEvtBgoCluster* DmpEvtBgoShower::GetMaxClusterInLayer(int l)const
{
  DmpEvtBgoCluster *cc = 0;
  double max_e = 0;
  for(int i=0;i<fClusters->GetEntriesFast();++i){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(i));
    if(aC->fLayer == l){
      if(aC->fTotE > max_e){
        max_e = aC->fTotE;
        cc = aC;
      }
    }
  }
  return cc;
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetWindowEnergy(int nBars,int nHalf)const
{
  double TE = 0;
  int lid = this->GetLayerIDOfMaxE();
  int min = (lid - nHalf)<0 ? 0 : lid - nHalf;
  int max = (lid + nHalf)>BGO_LayerNO -1? BGO_LayerNO -1: lid + nHalf;
  for(int i = min;i<= max;++i){
    DmpEvtBgoCluster *aClu = this->GetMaxClusterInLayer(i);
    if(aClu != 0){
      TE += aClu->GetWindowEnergy(nBars);
    }
  }
  return TE;
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetWindowEnergyRatio(int nBars,int nHalf)const
{
  return this->GetWindowEnergy(nBars,nHalf) / fTotE;
}

int DmpEvtBgoShower::GetFiredBarNumber(int layerID,double e_low)const
{
  int b = 0;
  int nClus = fClusters->GetEntriesFast();
  for(int l=0;l<nClus;++l){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(l));
    if(layerID == aC->fLayer || layerID == -1){
      b += aC->GetFiredBarNumber(e_low);
    }
  }
  return b;
}

int DmpEvtBgoShower::GetLayerIDOfMaxFiredBarNo()const
{
  int id=-1;
  int mn=0;
  for(int i=0;i<BGO_LayerNO;++i){
    int n = this->GetFiredBarNumber(i);
    if(n > mn){
      id = i;
      mn = n;
    }
  }
  return id;
}

//-------------------------------------------------------------------
DmpEvtBgoCluster* DmpEvtBgoShower::AddNewCluster(DmpBgoFiredBar *seedBar)
{
  DmpEvtBgoCluster *newC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->New(fClusters->GetEntriesFast()));
  newC->fLayer = seedBar->fLayer;
  newC->fSeedBarID = seedBar->fBar;
  //std::cout<<"===> Add new cluser(layer, seed bar):\t"<<newC->fLayer<<"\t"<<newC->fSeedBarID<<std::endl;
  newC->AddNewFiredBar(seedBar);
  return newC;
}

//-------------------------------------------------------------------
std::vector<DmpEvtBgoCluster*> DmpEvtBgoShower::GetAllClusterInLayer(int l)const
{
  std::vector<DmpEvtBgoCluster*>    cInLayer;
  for(int i=0;i<fClusters->GetEntriesFast();++i){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(i));
    if(aC->fLayer == l){
      cInLayer.push_back(aC);
    }
  }
  return cInLayer;
}

//-------------------------------------------------------------------
std::vector<DmpBgoFiredBar*> DmpEvtBgoShower::GetIsolatedBar(int l,double noise)const
{
  std::vector<DmpBgoFiredBar*>  backme;
  std::vector<DmpBgoFiredBar*>  allB;
  std::vector<DmpEvtBgoCluster*> allC = this->GetAllClusterInLayer(l);
  for(int ic=0;ic<allC.size();++ic){
    int nB = allC[ic]->fFiredBar->GetEntriesFast();
    for(int i=0;i<nB;++i){
      DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(allC[ic]->fFiredBar->At(i));
      if(aB->fE > noise){
        allB.push_back(aB);
      }
    }
  }
  for(int ib=0;ib<allB.size();++ib){
    int bid = allB[ib]->fBar;
    bool isIso = true;
    for(int i = 0;i<allB.size();++i){
      if(TMath::Abs(allB[i]->fBar - bid) == 1){
        isIso = false;
        break;
      }
    }
    if(isIso){
        backme.push_back(allB[ib]);
    }
  }
  return backme;
}

//-------------------------------------------------------------------
std::vector<DmpBgoFiredBar*> DmpEvtBgoShower::GetIsolatedBar(int l,double eLow, double eHigh,double noise)const
{
  std::vector<DmpBgoFiredBar*>  backme;
  std::vector<DmpBgoFiredBar*>  allB;
  std::vector<DmpEvtBgoCluster*> allC = this->GetAllClusterInLayer(l);
  for(int ic=0;ic<allC.size();++ic){
    int nB = allC[ic]->fFiredBar->GetEntriesFast();
    for(int i=0;i<nB;++i){
      DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(allC[ic]->fFiredBar->At(i));
      if(aB->fE > noise){
        allB.push_back(aB);
      }
    }
  }
  for(int ib=0;ib<allB.size();++ib){
    int bid = allB[ib]->fBar;
    bool isIso = true;
    for(int i = 0;i<allB.size();++i){
      if(TMath::Abs(allB[i]->fBar - bid) == 1){
        isIso = false;
        break;
      }
    }
    if(isIso){
        if(allB[ib]->fE > eLow && allB[ib]->fE < eHigh) backme.push_back(allB[ib]);
    }
  }
  return backme;
}

//-------------------------------------------------------------------
std::vector<DmpBgoFiredBar*> DmpEvtBgoShower::GetIsolatedBarFromLayer(int sl,double noise)const
{
  std::vector<DmpBgoFiredBar*>  backme;
  for(int l=sl;l<BGO_LayerNO;++l){
    std::vector<DmpBgoFiredBar*> tmp = this->GetIsolatedBar(l,noise);
    for(int i=0;i<tmp.size();++i){
      backme.push_back(tmp.at(i));
    }
  }
  return backme;
}

std::vector<DmpBgoFiredBar*> DmpEvtBgoShower::GetIsolatedBarFromLayer(int sl,double el,double eh,double noise)const
{
  std::vector<DmpBgoFiredBar*>  backme;
  for(int l=sl;l<BGO_LayerNO;++l){
    std::vector<DmpBgoFiredBar*> tmp = this->GetIsolatedBar(l,noise);
    for(int i=0;i<tmp.size();++i){
      if(tmp.at(i)->fE > el && tmp.at(i)->fE < eh){
        backme.push_back(tmp.at(i));
      }
    }
  }
  return backme;
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetCoGBarIDInLayer(int l)const
{
  double totalEInL = this->GetTotalEnergy(l);
  if(totalEInL == 0){return -1;}

  double cog = 0;
  std::vector<DmpEvtBgoCluster*> cInL = this->GetAllClusterInLayer(l);
  for(int i=0;i<cInL.size();++i){
    cog += (cInL[i]->GetCoGBarID()+1) * cInL[i]->fTotE;
  }
  return (cog/totalEInL - 1);
}

//-------------------------------------------------------------------
Position DmpEvtBgoShower::GetCoGPositionInLayer(int l)const
{
  Position po(0,0,0);
  double totalEInL = this->GetTotalEnergy(l);
  if(totalEInL == 0){return po;}    // po.z() must == 0

  std::vector<DmpEvtBgoCluster*> cInL = this->GetAllClusterInLayer(l);
  for(int i=0;i<cInL.size();++i){
    po += cInL[i]->fCenter * cInL[i]->fTotE;
  }         // p0.z() must != 0, but x(),y() may = 0
  return po;
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetPileupRatio()const
{
  return (fClusters->GetEntriesFast() / (double)BGO_LayerNO - 1);
}

double DmpEvtBgoShower::GetGValue(int layerID)const
{
  double v = fRMS[layerID];
  if(v <= 0) return v;
  return fRMS[layerID]*fRMS[layerID] * fTotE / this->GetTotalEnergy(layerID);
}

double DmpEvtBgoShower::GetTotalRMS()const
{
  double v = 0.;
  for(int i=0;i<BGO_LayerNO;++i){
    v += fRMS[i];
  }
  return v;
}

double DmpEvtBgoShower::GetNormalizedRMS(int layerID)const
{
  return fRMS[layerID]/this->GetTotalRMS();
}

#define Z0 58.5         // first layer z
//-------------------------------------------------------------------
Position DmpEvtBgoShower::GetEntryPoint()const
{
  Position p(0,0,Z0);
  p.SetX(fXZFitPar[0]+fXZFitPar[1]*Z0);
  p.SetY(fYZFitPar[0]+fYZFitPar[1]*Z0);
  return p;
}

//-------------------------------------------------------------------
Direction DmpEvtBgoShower::GetTrackDirection()const
{
  Direction d(fXZFitPar[1],fYZFitPar[1],1);
  return d;
}

//-------------------------------------------------------------------
void DmpEvtBgoShower::Calculation()
{
  fTotE = this->GetTotalEnergy();

  fLRMS = 0.;
  std::vector<double>  eInLayer(BGO_LayerNO,0);
{
  for(int i=0;i<BGO_LayerNO;++i){
    eInLayer[i] = (this->GetTotalEnergy(i));
  }
  double cenL = 0;
  for(int i=0;i<BGO_LayerNO;++i){
    cenL += (i+1)*eInLayer[i];
  }
  cenL = cenL/fTotE - 1;
  for(int i=0;i<BGO_LayerNO;++i){
    fLRMS += eInLayer[i] * pow((double)i-cenL,2);
  }
  fLRMS = sqrt(fLRMS / fTotE);
}

  // fRMS, fFValue
  for(int lid=0;lid<BGO_LayerNO;++lid){
    if(eInLayer[lid] == 0) continue;

    double cen = this->GetCoGBarIDInLayer(lid);
    std::vector<DmpBgoFiredBar*>  bInL;   // key
    std::vector<DmpEvtBgoCluster*> cInL = this->GetAllClusterInLayer(lid);
    for(int i=0;i<cInL.size();++i){
      for(int b=0;b<cInL[i]->fFiredBar->GetEntriesFast();++b){
        bInL.push_back(dynamic_cast<DmpBgoFiredBar*>(cInL[i]->fFiredBar->At(b)));
      }
    }
    double rms2_Xt =0;
    for(int b=0;b<bInL.size();++b){
      rms2_Xt += bInL[b]->fE * pow(cen - bInL[b]->fBar,2);
    }
    fFValue[lid] = rms2_Xt / fTotE;
    fRMS[lid] = sqrt(rms2_Xt / eInLayer[lid]);
  }

    //clock_t t3=clock();
    //
  TH2D xz_posi("xz","xz",550,50,600,800,-400,400);
  TH2D yz_posi("yz","yz",550,50,600,800,-400,400);
  static TF1  p1("myf","pol1");
  for(int i=0;i<BGO_LayerNO;++i){
    if(this->GetEnergyRatioOfLayer(i) < fTotE * 0.20) continue;
    DmpEvtBgoCluster *aC = this->GetMaxClusterInLayer(i);
    if(aC){
      if(aC->GetERatioOfSeedBar(1) < 0.4) continue;
      if(i%2 == 0){
        yz_posi.Fill(aC->fCenter.z(),aC->fCenter.y(),aC->fTotE);
      }else{
        xz_posi.Fill(aC->fCenter.z(),aC->fCenter.x(),aC->fTotE);
      }
    }
  }
  if(xz_posi.GetEntries()>1 && yz_posi.GetEntries()>1){
    xz_posi.Fit("myf","QCN");
    fXZFitPar[0] = p1.GetParameter(0);
    fXZFitPar[1] = p1.GetParameter(1);
    fXZFitPar[2] = p1.GetParError(0);
    fXZFitPar[3] = p1.GetParError(1);
    fXZFitPar[4] = p1.GetChisquare();
  //std::cout<<"XZ fit parameters (Pa0,P1,P0_E,P1_E,Chi2):";
  //for(int i=0;i<5;++i){
  //  std::cout<<"\t"<<fXZFitPar[i];
  //}
  //std::cout<<std::endl;
    yz_posi.Fit("myf","QN");
    fYZFitPar[0] = p1.GetParameter(0);
    fYZFitPar[1] = p1.GetParameter(1);
    fYZFitPar[2] = p1.GetParError(0);
    fYZFitPar[3] = p1.GetParError(1);
    fYZFitPar[4] = p1.GetChisquare();
  }
}

//-------------------------------------------------------------------
void DmpEvtBgoShower::MyPrint(bool allInfor)const
{
  std::cout<<"===> Energy, nClusters, Max_E_L, fLRMS:\t"<<fTotE<<"\t"<<fClusters->GetEntriesFast()<<"\t"<<GetLayerIDOfMaxE()<<"\t"<<fLRMS<<std::endl;
  std::cout<<"===> Linear fit parameters (Pa0,P1,P0_E,P1_E,Chi2):\n\tXZ:\t";
  for(int i=0;i<5;++i){
    std::cout<<"\t"<<fXZFitPar[i];
  }
  std::cout<<"\n\tYZ:\t";
  for(int i=0;i<5;++i){
    std::cout<<"\t"<<fYZFitPar[i];
  }
  std::cout<<std::endl;
  std::cout<<"===> RMS and FValue:  X\t\t\tY"<<std::endl;
  for(int i=0;i<BGO_LayerNO/2;++i){
    std::cout<<"\t"<<fRMS[i*2+1]<<", "<<fFValue[i*2+1]<<"\t"<<fRMS[i*2]<<", "<<fFValue[i*2]<<std::endl;
  }
  if(allInfor){
    for(short i=0;i<fClusters->GetEntriesFast();++i){
      (dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(i)))->MyPrint();
    }
  }
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetGValueOfEMaxLayer()const
{
  return this->GetGValue(this->GetLayerIDOfMaxE());
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetRMSOfEMaxLayer()const
{
  return this->fRMS[this->GetLayerIDOfMaxE()];
}

//-------------------------------------------------------------------
int DmpEvtBgoShower::GetLayerIDOfMaxGValue()const
{
  double v = 0,tmp = 0;
  int id =0;
  for(int i=0;i<BGO_LayerNO;++i){
    tmp = this->GetGValue(i);
    if(tmp > v){
      v = tmp;
      id = i;
    }
  }
  return id;
}

int DmpEvtBgoShower::GetLayerIDOfMaxFValue()const
{
  double f=fFValue[0];
  int l=0;
  for(int i=1;i<BGO_LayerNO;++i){
    if(f < fFValue[i]){
      f=fFValue[i];
      l = i;
    }
  }
  return l;
}

int DmpEvtBgoShower::GetLayerIDOfMinFValue()const
{
  double f=fFValue[0];
  int l=0;
  for(int i=1;i<BGO_LayerNO;++i){
    if(f > fFValue[i]){
      f=fFValue[i];
      l = i;
    }
  }
  return l;
}

//-------------------------------------------------------------------
int DmpEvtBgoShower::GetLayerIDOfMinGValue()const
{
  double v = 1000,tmp = 0;
  int id =0;
  for(int i=0;i<BGO_LayerNO;++i){
    tmp = this->GetGValue(i);
    if(tmp < v){
      v = tmp;
      id = i;
    }
  }
  return id;
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetMaxGValue()const
{
  int lid = this->GetLayerIDOfMaxGValue();
  return this->GetGValue(lid);
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetMyValue(int nHalfLayer)const
{
  int id = this->GetLayerIDOfMaxGValue();
  double v = 0;
  int min = (id - nHalfLayer)<0 ? 0: (id-nHalfLayer);
  int max = (id + nHalfLayer)>13 ? 13: (id+nHalfLayer);
  for(int i = min;i<=max;++i){
    v += this->GetGValue(i);
  }
  return v;
}

//-------------------------------------------------------------------
int DmpEvtBgoShower::GetLayerIDOfMaxRMS()const
{
  int id = 0;
  double v = 0;
  for(int i = 0;i<BGO_LayerNO;++i){
    if(fRMS[i]>v){
      v = fRMS[i];
      id = i;
    }
  }
  return id;
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetMaxRMS()const
{
   return *std::max_element(fRMS,fRMS+BGO_LayerNO);
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetMaxFValue()const
{
   return *std::max_element(fFValue,fFValue+BGO_LayerNO);
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetMinRMS()const
{
  double v = 999.9;
  for(int i = 0;i<BGO_LayerNO;++i){
    if(fRMS[i]<v && !(fRMS[i] <0)){
      v = fRMS[i];
    }
  }
  return v;
}

//-------------------------------------------------------------------
int DmpEvtBgoShower::GetLayerIDOfMinRMS()const
{
  int id = 0;
  double v = 999.9;
  for(int i = 0;i<BGO_LayerNO;++i){
    if(fRMS[i]<v && !(fRMS[i] <0)){
      v = fRMS[i];
      id = i;
    }
  }
  return id;
}

DmpBgoFiredBar* DmpEvtBgoShower::GetEMaxBar()const
{
  DmpBgoFiredBar *it = 0;
  double max_e = 0;
  int nC = fClusters->GetEntriesFast();
  for(int i=0;i<nC;++i){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(i));
    if(aC->GetSeedBar()->fE > max_e){
      it = aC->GetSeedBar();
      max_e = it->fE;
    }
  }
  return it;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
#define EOfMips    23.0
bool DmpEvtBgoShower::_triggerFromLayer(int layerID,double threshold)const
{
  std::vector<DmpEvtBgoCluster*> clus = GetAllClusterInLayer(layerID);
  for(int ic=0;ic<clus.size();++ic){
    for(int ib=0;ib<clus[ic]->fFiredBar->GetEntriesFast();++ib){
      if((dynamic_cast<DmpBgoFiredBar*>(clus[ic]->fFiredBar->At(ib)))->fE > threshold*EOfMips){
        return true;
      }
    }
  }
  return false;
}

bool DmpEvtBgoShower::T0(double threshold)const
{
  return _triggerFromLayer(0,threshold);
}

bool DmpEvtBgoShower::Group0_01(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(0,threshold)&&_triggerFromLayer(1,threshold)&&_triggerFromLayer(2,threshold)&&_triggerFromLayer(3,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group0_10(double threshold)const
{
  if(this->T0()){
    return Group0_01(threshold);
  }
  return false;
}

bool DmpEvtBgoShower::Group0_11(double threshold)const
{
  if(this->T0()){
    return Group0_01(threshold);
  }
  return false;
}

bool DmpEvtBgoShower::Group1_00(double threshold)const
{
  if(this->T0()){
    bool plan0 = _triggerFromLayer(0,threshold)||_triggerFromLayer(1,threshold);
    bool plan1 = _triggerFromLayer(2,threshold)||_triggerFromLayer(3,threshold);
    bool plan5 = _triggerFromLayer(10,threshold)||_triggerFromLayer(11,threshold);
    bool plan6 = _triggerFromLayer(12,threshold)||_triggerFromLayer(13,threshold);
    return (plan0 && plan1 && plan5 && plan6);
  }
  return false;
}

bool DmpEvtBgoShower::Group1_01(double threshold)const
{
  if(this->T0()){
    bool plan0 = _triggerFromLayer(0,threshold)||_triggerFromLayer(1,threshold);
    bool plan6 = _triggerFromLayer(12,threshold)||_triggerFromLayer(13,threshold);
    return (plan0 && plan6);
  }
  return false;
}

bool DmpEvtBgoShower::Group1_10(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(2,threshold) && _triggerFromLayer(10,threshold) && _triggerFromLayer(12,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group1_11(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(0,threshold) && _triggerFromLayer(12,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group2_00(double threshold)const
{
  if(this->T0()){
    bool plan0 = _triggerFromLayer(0,threshold)||_triggerFromLayer(1,threshold);
    bool plan1 = _triggerFromLayer(2,threshold)&&_triggerFromLayer(3,threshold);
    bool plan5 = _triggerFromLayer(10,threshold)&&_triggerFromLayer(11,threshold);
    bool plan6 = _triggerFromLayer(12,threshold)&&_triggerFromLayer(13,threshold);
    return (plan0 && plan1 && plan5 && plan6);
  }
  return false;
}

bool DmpEvtBgoShower::Group2_01(double threshold)const
{
  if(this->T0()){
    bool plan0 = _triggerFromLayer(0,threshold)&&_triggerFromLayer(1,threshold);
    bool plan6 = _triggerFromLayer(12,threshold)&&_triggerFromLayer(13,threshold);
    return (plan0 && plan6);
  }
  return false;
}

bool DmpEvtBgoShower::Group2_10(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(3,threshold) && _triggerFromLayer(11,threshold) && _triggerFromLayer(13,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group2_11(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(1,threshold) && _triggerFromLayer(13,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group3_0000(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(0,threshold) && _triggerFromLayer(1,threshold) && _triggerFromLayer(2,threshold) && _triggerFromLayer(3,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group3_0001(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(0,threshold) && _triggerFromLayer(1,threshold) && _triggerFromLayer(2,threshold) && _triggerFromLayer(3,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group3_0010(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(0,threshold) && _triggerFromLayer(1,threshold) && _triggerFromLayer(2,threshold) && _triggerFromLayer(3,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group3_0011(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(0,threshold) && _triggerFromLayer(1,threshold) && _triggerFromLayer(2,threshold) && _triggerFromLayer(3,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group3_0100(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(0,threshold) && _triggerFromLayer(1,threshold) && _triggerFromLayer(2,threshold) && _triggerFromLayer(3,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group3_0101(double threshold)const
{
  if(this->T0()){
    return (_triggerFromLayer(0,threshold) || _triggerFromLayer(1,threshold) && _triggerFromLayer(2,threshold) && _triggerFromLayer(3,threshold));
  }
  return false;
}

bool DmpEvtBgoShower::Group3_0110(double threshold)const
{
  if(this->T0()){
  }
  return false;
}

bool DmpEvtBgoShower::Group3_0111(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group3_1000(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group3_1001(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group3_1010(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group3_else(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group4_000(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group4_001(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group4_010(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group4_011(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group4_100(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group4_101(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group4_110(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

bool DmpEvtBgoShower::Group4_111(double threshold)const
{
  if(this->T0()){
    return true;
  }
  return false;
}

DmpBgoFiredBar* DmpEvtBgoShower::GetEMaxBarInLayer(int l)const
{
  std::vector<DmpEvtBgoCluster*>  cluInL = this->GetAllClusterInLayer(l);
  DmpBgoFiredBar *add=cluInL[0]->GetSeedBar();
  for(int i=1;i<cluInL.size();++i){
    DmpBgoFiredBar *tmp=cluInL[i]->GetSeedBar();
    if(add->fE < tmp->fE){
      add = tmp;
    }
  }
  return add;
}

int DmpEvtBgoShower::GetClusterNoInLayer(int l,double eCut,double eh)const
{
  std::vector<DmpEvtBgoCluster*>  cluInL = this->GetAllClusterInLayer(l);
  int n=0;
  for(int i=0;i<cluInL.size();++i){
    DmpBgoFiredBar *tmp = cluInL[i]->GetSeedBar();
    if(tmp->fE > eCut && tmp->fE < eh) ++n;
  }
  return n;
}

double DmpEvtBgoShower::GetFractal(int l,int b1,int b2)const
{
  double f=0;
  DmpEvtBgoCluster *ac = GetMaxClusterInLayer(l);
  if(ac){
    f = TMath::Log10(ac->GetERatioOfSeedBar(b2)) / TMath::Log10(ac->GetERatioOfSeedBar(b1));
  }
  return f;
}

double DmpEvtBgoShower::GetFractalOfEMaxLayer(int b1,int b2)const
{
  int l=this->GetLayerIDOfMaxE();
  return this->GetFractal(l,b1,b2);
}

double DmpEvtBgoShower::GetFractalOfGMaxLayer(int b1,int b2)const
{
  int l=this->GetLayerIDOfMaxGValue();
  return this->GetFractal(l,b1,b2);
}

double DmpEvtBgoShower::GetFractalOfFMaxLayer(int b1,int b2)const
{
  int l=this->GetLayerIDOfMaxFValue();
  return this->GetFractal(l,b1,b2);
}

double DmpEvtBgoShower::GetFractalOfRMSMaxLayer(int b1,int b2)const
{
  int l = this->GetLayerIDOfMaxRMS();
  return this->GetFractal(l,b1,b2);
}

/*
DmpBgoIsolatedBars::DmpBgoIsolatedBars()
:fEvtBgoShower(0),
fFromLayerID(0),
fECutNoise(2),        
fECutLow(fECutNoise),
fECutHigh(500)// 500 MeV, a very big value
{}

DmpBgoIsolatedBars::DmpBgoIsolatedBars(DmpEvtBgoShower *r)
:fEvtBgoShower(r),
fFromLayerID(0),
fECutNoise(2),        
fECutLow(fECutNoise),
fECutHigh(500)// 500 MeV, a very big value
{}

DmpBgoIsolatedBars::DmpBgoIsolatedBars(DmpEvtBgoShower *r,double eBarLow,double eBarH,double noiseCut)
:fEvtBgoShower(r),
fECutLow(eBarLow),
fECutHigh(eBarH),// 500 MeV, a very big value
fFromLayerID(0),
fECutNoise(noiseCut)
{}

DmpBgoIsolatedBars::DmpBgoIsolatedBars(DmpEvtBgoShower *r,double eBarLow,double eBarH,int fromLayerID,double noiseCut)
:fEvtBgoShower(r),
fECutLow(eBarLow),
fECutHigh(eBarH),// 500 MeV, a very big value
fFromLayerID(fromLayerID),
fECutNoise(noiseCut)
{}

DmpBgoIsolatedBars::~DmpBgoIsolatedBars()
{
}

bool DmpBgoIsolatedBars::UpdateEvent()
{
  if(fEvtBgoShower == 0){return false;}
  this->Reset();
  fIsolatedBars = fEvtBgoShower->GetIsolatedBarFromLayer(fFromLayerID,fECutLow, fECutHigh,fECutNoise);
  fNIsoBars = fIsolatedBars.size();
  if(fNIsoBars == 0) {return true;}

  int lidList[fNIsoBars];
  int bidList[fNIsoBars];
  int bidList_0[fNIsoBars];
  int nBarInL_0=0;
  int bidList_1[fNIsoBars];
  int nBarInL_1=0;
  double eList[fNIsoBars];

  for(int i=0;i<fNIsoBars;++i){
    int l = fIsolatedBars[i]->fLayer;
    int b = fIsolatedBars[i]->fBar;
    lidList[i] = l;
    bidList[i] = b;
    if(l%2 == 0){
      bidList_0[nBarInL_0] = b;
      ++nBarInL_0;
    }else{
      bidList_1[nBarInL_1] = b;
      ++nBarInL_1;
    }
    eList[i] = fIsolatedBars[i]->fE;
    fETotal += eList[i];
  }

  fEMean = fETotal / fNIsoBars;
  fRMSLayer = TMath::RMS(fNIsoBars,lidList);
  fRMSBar = TMath::RMS(fNIsoBars,bidList);
  fRMSBar_L0 = TMath::RMS(nBarInL_0,bidList_0);
  fRMSBar_L1 = TMath::RMS(nBarInL_1,bidList_1);
  fBarIDMean_L0 = TMath::Mean(nBarInL_0,bidList_0);
  fBarIDMean_L1 = TMath::Mean(nBarInL_1,bidList_1);

  for(int i=0;i<fNIsoBars;++i){
    if(lidList[i] < fFirstLID){ fFirstLID = l;}
    bool noHas = true;
    for(int xx = 0;xx<i;++xx){
      if(lidList[xx] == lidList[i]){
        noHas = false;
        break;
      }
    }
    if(noHas) ++fNLayerID;
  }

  for(int i=0;i<fNIsoBars;++i){
    bool noHas = true;
    for(int xx = 0;xx<i;++xx){
      if(bidList[xx] == bidList[i]){
        noHas = false;
        break;
      }
    }
    if(noHas) ++fNBarID;
  }

}

void DmpBgoIsolatedBars::Reset()
{
  fNIsoBars = 0;
  fETotal = 0;  // must be 0, use it somewhere else
  fEMean = 0;
  fRMSLayer = -99;
  fRMSBar = -99;
  fRMSBar_L0 = -99;
  fRMSBar_L1 = -99;
  fBarIDMean_L0 = -99;
  fBarIDMean_L1 = -99;
  fNBarID = 0;  // must be 0
  fNLayerID = 0;    // must set to 0
  fFirstLID = BGO_LayerNO;   // must be this value
}

*/
