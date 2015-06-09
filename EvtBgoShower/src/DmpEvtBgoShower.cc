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

double DmpEvtBgoCluster::GetERatioOfSeedBar()const
{
  return this->GetSeedBar()->fE / fTotE;
}

double DmpEvtBgoCluster::GetS1ToSn(int n)const
{
  return this->GetSeedBar()->fE / this->GetWindowEnergy(n);
}

double DmpEvtBgoCluster::GetFractal(int nb1,int nb2)const
{
  return TMath::Log10(this->GetS1ToSn(nb1)) / TMath::Log10(this->GetS1ToSn(nb2));
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

int DmpEvtBgoCluster::GetBarNumber(double e_low)const
{
  int n=0;
  for(int i=0;i<fFiredBar->GetEntriesFast();++i){
    if((dynamic_cast<DmpBgoFiredBar*>(fFiredBar->At(i)))->fE > e_low) ++n;
  }
  return n;
}

double DmpEvtBgoCluster::GetRMS()const
{
  double rms2_Xt =0;
  DmpBgoFiredBar *ab=0;
  double cen=this->GetCoGBarID();
  for(int b=0;b<fFiredBar->GetEntriesFast();++b){
    ab = dynamic_cast<DmpBgoFiredBar*>(fFiredBar->At(b));
    rms2_Xt += ab->fE * pow(cen - ab->fBar,2);
  }
  return sqrt(rms2_Xt / fTotE);
}

double DmpEvtBgoCluster::GetSeedRatio2Size()const
{
  return this->GetERatioOfSeedBar() / this->GetBarNumber();
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
DmpEvtBgoCluster* DmpEvtBgoShower::GetEMaxCluster()const
{
  DmpEvtBgoCluster *ac =0;
  double e = 0;
  for(int i=0;i<fClusters->GetEntriesFast();++i){
    DmpEvtBgoCluster *tmp = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(i));
    if(e < tmp->fTotE){
      ac = tmp;
      e = tmp->fTotE;
    }
  }
  return ac;
}

DmpEvtBgoCluster* DmpEvtBgoShower::GetEMaxClusterInEMaxLayer()const
{
  int lid=this->GetLayerIDOfMaxE();
  return this->GetEMaxClusterInLayer(lid);
}

DmpEvtBgoCluster* DmpEvtBgoShower::GetEMaxClusterInCoGLayer()const
{
  int lid=this->GetLayerIDOfCoG();
  return this->GetEMaxClusterInLayer(lid);
}

DmpEvtBgoCluster* DmpEvtBgoShower::GetEMaxClusterInGMinLayer()const
{
  int lid=this->GetLayerIDOfMinGValue();
  return this->GetEMaxClusterInLayer(lid);
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetTotalEnergy(int layerid, int whichSide)const
{
  if(layerid < 0 && fTotE > 0) return fTotE;
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
  if(l<0 || l>13 || barID <0 || barID > 21) return 0;
  std::vector<DmpEvtBgoCluster*>  cluInL = this->GetAllClusterInLayer(l);
  for(int i=0;i<cluInL.size();++i){
    for(int b=0;b<cluInL[i]->fFiredBar->GetEntriesFast();++b){
      DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(cluInL[i]->fFiredBar->At(b));
      if(aB->fBar == barID){
        return aB->fE;
      }
    }
  }
  return 0;
}

//-------------------------------------------------------------------
int DmpEvtBgoShower::GetLayerIDOfMaxE(int from,bool checkMe)const
{
  double max_e = 0.;
  int id = -1;
  for(int i=from;i<BGO_LayerNO;++i){
    double ei = this->GetTotalEnergy(i);
    if(ei > max_e){
      max_e = ei;
      id = i;
    }
  }
  if(id <= 11 && checkMe){
    double nextE = this->GetTotalEnergy(id+1);
    double next2E = this->GetTotalEnergy(id+2);
    if(nextE < next2E){
      id = this->GetLayerIDOfMaxE(id+1);
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
double DmpEvtBgoShower::GetERatioOfEMaxLayer()const
{
  return this->GetEnergyOfEMaxLayer() / fTotE;
}

double DmpEvtBgoShower::GetERatioOfCoGZ()const
{
  int l = this->GetLayerIDOfCoG();
  return this->GetERatioOfLayer(l);
}

double DmpEvtBgoShower::GetERatioOfEMinLayer()const
{
  return this->GetEnergyOfEMinLayer() / fTotE;
}

double DmpEvtBgoShower::GetERatioOfEMaxClusterInLayer(int l)const
{
  std::vector<double>  v = this->GetERatioOfClustersInLayer(l);
  return v.at(0);
}

double DmpEvtBgoShower::GetERatioOfEMaxClusterInCoG()const
{
  int l = this->GetLayerIDOfCoG();
  std::vector<double>  v = this->GetERatioOfClustersInLayer(l);
  if(v.size() ==0) return 0;
  return v.at(0);
}

double DmpEvtBgoShower::GetERatioOfEMinClusterInLayer(int l)const
{
  DmpEvtBgoCluster *ac = this->GetEMinClusterInLayer(l);
  double v=0;
  if(ac){
    v = ac->fTotE / this->GetTotalEnergy(l);
  }
  return v;
}

double DmpEvtBgoShower::GetERatioOfEMinClusterInEMinLayer()const
{
  int l = this->GetLayerIDOfMinE();
  return this->GetERatioOfEMinClusterInLayer(l);
}

double DmpEvtBgoShower::GetERatioOfEMaxClusterInMaxRMSLayer()const
{
  int l=this->GetLayerIDOfMaxRMS();
  DmpEvtBgoCluster *ac = this->GetEMaxClusterInLayer(l);
  double v=0;
  if(ac){
    v = ac->fTotE / this->GetTotalEnergy(l);
  }
  return v;
}

double DmpEvtBgoShower::GetEMax_ETail()const//LayerIDOfMaxMinLayer()const
{
  return this->GetERatioOfEMaxLayer() / this->GetERatioAtTail();
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetERatioOfLayer(int l)const
{
  return this->GetTotalEnergy(l) / fTotE;
}


std::vector<double> DmpEvtBgoShower::GetERatioOfLayers()const
{
  std::vector<double>  reme(BGO_LayerNO,0);
  for(int i=0;i<fClusters->GetEntriesFast();++i){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(i));
    reme[aC->fLayer] += aC->GetTotalEnergy();
  }
  for(int l=0;l<BGO_LayerNO;++l){
    reme[l] = reme[l] / fTotE;
  }
  return reme;
}

//-------------------------------------------------------------------
DmpEvtBgoCluster* DmpEvtBgoShower::GetEMaxClusterInLayer(int l)const
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

DmpEvtBgoCluster* DmpEvtBgoShower::GetEMinClusterInLayer(int l)const
{
  DmpEvtBgoCluster *cc = 0;
  double min_e = 9999999;
  for(int i=0;i<fClusters->GetEntriesFast();++i){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(i));
    if(aC->fLayer == l){
      if(aC->fTotE < min_e){
        min_e = aC->fTotE;
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
  DmpBgoFiredBar *seedB = this->GetEMaxBarInLayer(lid);
  while(seedB == 0){
    lid -= 1;
    seedB = this->GetEMaxBarInLayer(lid);
    //std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
  }
  int bid = seedB->fBar;
  int min = (lid - nHalf)<0 ? 0 : lid - nHalf;
  int max = (lid + nHalf)>13 ? 13: lid + nHalf;
  for(int i = min;i<= max;++i){
    std::vector<DmpBgoFiredBar*> bars = this->GetBars(i,0.5,1);
    for(int b=0;b<bars.size();++b){
      if(TMath::Abs(bars[b]->fBar - bid) <= nBars){
        TE += bars[b]->fE;
      }
    }
  }
  return TE;
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetWindowERatio(int nBars,int nHalf)const
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
      b += aC->GetBarNumber(e_low);
    }
  }
  return b;
}

int DmpEvtBgoShower::GetMaxFiredBarNumber()const
{
  return this->GetFiredBarNumber(this->GetLayerIDOfMaxFiredBarNo());
}

std::vector<int> DmpEvtBgoShower::GetFiredBarNumbers(double e_low)const
{
  std::vector<int> b(BGO_LayerNO,0);
  int nClus = fClusters->GetEntriesFast();
  for(int l=0;l<nClus;++l){
    DmpEvtBgoCluster *aC = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(l));
    b[aC->fLayer] += aC->GetBarNumber(e_low);
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

std::vector<double> DmpEvtBgoShower::GetERatioOfClustersInLayer(int l)const
{
  std::vector<DmpEvtBgoCluster*>    cInLayer = this->GetAllClusterInLayer(l);
  std::vector<double>  rv;
  double allE = this->GetTotalEnergy(l);
  for(int i=0;i<cInLayer.size();++i){
    rv.push_back(cInLayer[i]->fTotE / allE);
    //std::cout<<"\tDEBUG\t"<<rv.at(i);//<<std::endl;
  }
  //std::cout<<std::endl;
  std::sort(rv.rbegin(),rv.rend());
  //for(int i=0;i<rv.size();++i){
  //        std::cout<<"\t\t"<<rv.at(i);
  //}
  //std::cout<<std::endl;
  return rv;
}

std::vector<DmpBgoFiredBar*> DmpEvtBgoShower::GetBars(int l,double eBarCut,double eClusterCut)const
{
  std::vector<DmpBgoFiredBar*>  allB;
  std::vector<DmpEvtBgoCluster*> allC = this->GetAllClusterInLayer(l);
  for(int ic=0;ic<allC.size();++ic){
    if(allC[ic]->fTotE < eClusterCut) continue;
    int nB = allC[ic]->fFiredBar->GetEntriesFast();
    for(int i=0;i<nB;++i){
      DmpBgoFiredBar *aB = dynamic_cast<DmpBgoFiredBar*>(allC[ic]->fFiredBar->At(i));
      if(aB->fE > eBarCut){
        allB.push_back(aB);
      }
    }
  }
  return allB;
}

//-------------------------------------------------------------------
std::vector<DmpBgoFiredBar*> DmpEvtBgoShower::GetIsolatedBar(std::vector<int> allL,double noise)const
{
  std::vector<DmpBgoFiredBar*>  backme;
  for(std::vector<int>::iterator it = allL.begin();it!=allL.end();++it){
    std::vector<DmpBgoFiredBar*>  allB = this->GetBars(*it,noise,0);
    int nbb = allB.size();
    for(int ib=0;ib<nbb;++ib){
      int bid = allB[ib]->fBar;
      bool isIso = true;
      for(int i = 0;i<nbb;++i){
        if(TMath::Abs(allB[i]->fBar - bid) == 1){
          isIso = false;
          break;
        }
      }
      if(isIso){
        backme.push_back(allB[ib]);
      }
    }
  }
  return backme;
}

//-------------------------------------------------------------------
std::vector<DmpBgoFiredBar*> DmpEvtBgoShower::GetIsolatedBar(std::vector<int> l,double eLow, double eHigh,double noise)const
{
  std::vector<DmpBgoFiredBar*>  backme = this->GetIsolatedBar(l,noise);
  for(std::vector<DmpBgoFiredBar*>::iterator it = backme.begin(); it != backme.end(); ){
    if((*it)->fE < eLow || (*it)->fE > eHigh){
            // erase this one, and to next
      it = backme.erase(it);
    }else{
            // to next
      ++it;
    }
  }
  return backme;
}

DmpBgoFiredBar* DmpEvtBgoShower::GetEMaxIsolatedBar(std::vector<int> fromIDs,double noise)const
{
  std::vector<DmpBgoFiredBar*>  ab = this->GetIsolatedBar(fromIDs,noise);
  if(ab.size() == 0){
std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
    return 0;
  }
  DmpBgoFiredBar *mb =ab[0];
  for(int i=0;i<ab.size();++i){
    if(ab[i]->fE > mb->fE){
      mb = ab[i];
    }
  }
  return mb;
}

DmpBgoFiredBar* DmpEvtBgoShower::GetEMinIsolatedBar(std::vector<int> fromL,double noise)const
{
  std::vector<DmpBgoFiredBar*>  ab = this->GetIsolatedBar(fromL,noise);
  if(ab.size() == 0) return 0;
  DmpBgoFiredBar *mib =ab[0];
  for(int i=0;i<ab.size();++i){
    if(ab[i]->fE < mib->fE){
      mib = ab[i];
    }
  }
  return mib;
}

int DmpEvtBgoShower::GetLayerIDOfMostIsolatedBars(double eLow,double eHigh,double noise)const
{
  std::vector<int> layerIDs;
  for(int l=0;l<BGO_LayerNO;++l){
    layerIDs.push_back(l);
  }
  std::vector<DmpBgoFiredBar*>  allBars = this->GetIsolatedBar(layerIDs,eLow,eHigh,noise);
  std::vector<int> nbars(BGO_LayerNO,0);
  for(int i=0;i<allBars.size();++i){
    nbars[allBars[i]->fLayer] +=1;
  }
  int rme=-1;
  int nbs = 0;
  for(int l=0;l<BGO_LayerNO;++l){
    if(nbars[l] > nbs){
      rme = l;
      nbs = nbars[l];
    }
  }
  return rme;
}

int DmpEvtBgoShower::GetIsolatedBarNo(std::vector<int> l, double el, double eh,double noise)const
{
  std::vector<DmpBgoFiredBar*>  aa = this->GetIsolatedBar(l,el, eh,noise);
  return aa.size();
}

double DmpEvtBgoShower::GetBRMSOfIsolatedBar(std::vector<int> L,double el,double eh,double noise)const
{
  std::vector<DmpBgoFiredBar*>  ab = this->GetIsolatedBar(L,el,eh,noise);
  int n = ab.size();
  int bs[n];
  for(int i=0;i<n;++i){
    bs[i]  = ab[i]->fBar;
  }
  return TMath::RMS(n,bs);
}

double DmpEvtBgoShower::GetLRMSOfIsolatedBar(std::vector<int> l,double el,double eh,double noise)const
{
  std::vector<DmpBgoFiredBar*>  ab = this->GetIsolatedBar(l,el,eh,noise);
  int n = ab.size();
  int bs[n];
  for(int i=0;i<n;++i){
    bs[i]  = ab[i]->fLayer;
  }
  return TMath::RMS(n,bs);
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
int DmpEvtBgoShower::GetClusterNo(double eL,double eH)const
{
  int x=0;
  for(int l=0;l<BGO_LayerNO;++l){
    x += this->GetClusterNoInLayer(l,eL,eH);
  }
  return x;
}

double DmpEvtBgoShower::GetGValue(int layerID)const
{
  double e = this->GetTotalEnergy(layerID);
  if(e >0){
    return TMath::Log10(fRMS[layerID]*fRMS[layerID] * fTotE / e);
  }
  return -1;
}

double DmpEvtBgoShower::GetMaxRMSFromTail()const
{
  double m_rms = 0;
  for(int l = 1;l < BGO_LayerNO;++l){
    if(fRMS[l] > m_rms){
      m_rms = fRMS[l];
    }
  }
  return m_rms;
}

double DmpEvtBgoShower::GetTotalRMS(int id,bool allGap)const
{
  double v = 0.;
  for(int i=0;i<BGO_LayerNO;++i){
    if(i%2 == id || id == -1){
      if(fRMS[i] >= 0 || allGap){
        v += fRMS[i];
      }
    }
  }
  return v;
}

double DmpEvtBgoShower::GetRMS()const
{
  double vxm,vym;
  int xl=-1,yl = 99;  // NOTE:    must set -1 and 99
  int last_xl=0,last_yl=0;
  while(TMath::Abs(xl-yl) != 1){
    if(last_xl == xl && last_yl == yl){
      break;
    }else{
      last_xl = xl;
      last_yl = yl;
    }
    if(xl > yl) xl = yl;  // set the small value
    vxm = 0.;
    vym = 0.;
  //std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")\t"<<xl<<"\t"<<yl<<std::endl;
    for(int i=xl+1;i<BGO_LayerNO;++i){
      if(i%2 == 0){
        if(vxm < fRMS[i]){
          vxm = fRMS[i];
          xl = i;
        }
      }else{
        if(vym < fRMS[i]){
          vym = fRMS[i];
          yl = i;
        }
      }
    }
  }
  std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")\t"<<xl<<"\t"<<yl<<std::endl;
  return TMath::Sqrt(vxm*vxm + vym *vym);
}

double DmpEvtBgoShower::GetRMSOfEMaxBarID(int type)const
{
  double bkme = 0;
  if(type == -1){
    double x = this->GetRMSOfEMaxBarID(0);
    double y = this->GetRMSOfEMaxBarID(1);
    return TMath::Sqrt(x*x + y*y);
  }
  int  bID[7];
  int nBars = 0;
  for(int l=0;l<BGO_LayerNO;++l){
    if(l%2 == type){
      DmpBgoFiredBar *ab  = this->GetEMaxBarInLayer(l);
      if(ab != 0){
        bID[nBars] = ab->fBar;
        ++nBars;
      }
    }
  }
  return TMath::RMS(nBars,bID);
}

#define Z0 58.5         // first layer z
//-------------------------------------------------------------------
Position DmpEvtBgoShower::GetEntryPoint()const
{
  Position p(0,0,-1);
  p.SetX(fXZFitPar[0]+fXZFitPar[1]*Z0);
  p.SetY(fYZFitPar[0]+fYZFitPar[1]*Z0);
  for(int l=0;l<BGO_LayerNO;++l){
    DmpEvtBgoCluster *mc = this->GetEMaxClusterInLayer(l);
    if(mc == 0) continue;
    if(mc->GetERatioOfSeedBar() > 0.5 && this->GetClusterNoInLayer(l) < 3){
      p.SetZ(mc->fCenter.z());
      break;
    }
  }
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
  fTotE = this->GetTotalEnergy(-1);

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
  int nx=0,ny=0;
  for(int i=0;i<BGO_LayerNO;++i){
    if(this->GetERatioOfLayer(i) < 0.03 && i > 5) continue;
    std::vector<DmpBgoFiredBar*>  allBars = this->GetBars(i,10,18);
    double te = this->CalculateTotalE(allBars);
    TVector3 pos = this->CalculatePosition(allBars);
    if(i%2 == 0){
      yz_posi.Fill(pos.z(),pos.y(),te);
            ++ny;
    }else{
            ++nx;
      xz_posi.Fill(pos.z(),pos.x(),te);
    }
  }
if(nx < 2 || ny <2) std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")\tnx = "<<nx<<"\tny = "<<ny<<"\t\tTotE = "<<fTotE<<std::endl;
  if(xz_posi.GetEntries()>1 && yz_posi.GetEntries()>1){
    xz_posi.Fit("myf","QCN");
    fXZFitPar[0] = p1.GetParameter(0);
    fXZFitPar[1] = p1.GetParameter(1);
    fXZFitPar[2] = p1.GetParError(0);
    fXZFitPar[3] = p1.GetParError(1);
    fXZFitPar[4] = p1.GetChisquare()/p1.GetNDF();
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
    fYZFitPar[4] = p1.GetChisquare()/p1.GetNDF();
  }
}

double DmpEvtBgoShower::CalculateTotalE(std::vector<DmpBgoFiredBar*> rig)const
{
  double e=0;
  for(int i=0;i<rig.size();++i){
    e += rig[i]->fE;
  }
  return e;
}

TVector3 DmpEvtBgoShower::CalculatePosition(std::vector<DmpBgoFiredBar*> rig)const
{
  TVector3  re(0,0,0);
  double te=0;
  for(int i=0;i<rig.size();++i){
    re += rig[i]->fPosition * rig[i]->fE;
    te += rig[i]->fE;
  }
  re *= (1/te);
  return re;
}

//-------------------------------------------------------------------
void DmpEvtBgoShower::MyPrint(bool allInfor)const // TODO
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

double DmpEvtBgoShower::GetGValueOfCoG()const
{
  return this->GetGValue(this->GetLayerIDOfCoG());
}

//-------------------------------------------------------------------
double DmpEvtBgoShower::GetRMSOfEMaxLayer()const
{
  return this->fRMS[this->GetLayerIDOfMaxE()];
}

double DmpEvtBgoShower::GetRMSOfCoG()const
{
  int l = this->GetLayerIDOfCoG();
  return this->fRMS[l];
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

double DmpEvtBgoShower::GetMinGValue()const
{
  int lid = this->GetLayerIDOfMinGValue();
  return this->GetGValue(lid);
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

double DmpEvtBgoShower::GetERMax(int nLayer)const
{
  std::vector<double>  aE = this->GetERatioOfLayers();
  int lCoG = this->GetLayerIDOfMaxE();
  int ys = nLayer%2;
  nLayer = nLayer / 2;

  std::vector<double> e(4,0);
  for(int n=0;n<4;++n){
    for(int i=lCoG-nLayer-n;i<=lCoG + nLayer + ys-n;++i){
      e[n] += (i>=0 && i<=BGO_LayerNO-1) ? aE.at(i) : 0;
    }
  }

  double mx =0;
  for(int i=0;i<4;++i){
    if(e[i] > mx){
      mx = e[i];
    }
  }
  return mx;
}

double DmpEvtBgoShower::GetERMin(int nLayer)const
{
  int l= this->GetLayerIDOfTail();
  double t=0;
  for(int i=l-nLayer+1;i<=l;++i){
    t += this->GetERatioOfLayer(i);
  }
  return t;
}

double DmpEvtBgoShower::GetERT2()const
{
  return TMath::Log10(this->GetERMin(2));
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
  int l = this->GetLayerIDOfMinRMS();
  return fRMS[l];
}

//-------------------------------------------------------------------
int DmpEvtBgoShower::GetLayerIDOfMinRMS()const
{
  int id = 0;
  double v = 999.9;
  for(int i = 0;i<BGO_LayerNO;++i){
    if(fRMS[i]<v && fRMS[i] > 0){
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

DmpBgoFiredBar* DmpEvtBgoShower::GetBar(int l,int b)const
{
  std::vector<DmpBgoFiredBar*> bars = this->GetBars(l,0,0);
  for(int i=0;i<bars.size();++i){
    if(bars[i]->fBar == b){
      return bars[i];
    }
  }
  return 0;
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
  if(cluInL.size() == 0) return 0;
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
  DmpBgoFiredBar* seedBar = this->GetEMaxBarInLayer(l);
  if(seedBar == 0) return 0;
  int bid = seedBar->fBar;
  double seedE = seedBar->fE;
  std::vector<DmpBgoFiredBar*>  allB = this->GetBars(l,2.5,10);
  double tmpe1=0,tmpe2=0;
  for(int i=0;i<allB.size();++i){
    int b = allB[i]->fBar;
    double e = allB[i]->fE;
    if(TMath::Abs(bid - b) <= b1){
      tmpe1 += e;
    }
    if(TMath::Abs(bid - b) <= b2){
      tmpe2 += e;
    }
  }
  return TMath::Log10(seedE/tmpe1) / TMath::Log10(seedE/tmpe2);
}

double DmpEvtBgoShower::GetFractal(int b1,int b2)const
{
  DmpBgoFiredBar* seedBar = this->GetEMaxBar();
  if(seedBar == 0) return 0;
  int bid = seedBar->fBar;
  double seedE = 0;//seedBar->fE;
  double tmpe1=0,tmpe2=0;
  for(int l=0;l<fClusters->GetEntriesFast();++l){
    DmpEvtBgoCluster* ac = dynamic_cast<DmpEvtBgoCluster*>(fClusters->At(l));
    for(int i=0;i<ac->fFiredBar->GetEntriesFast();++i){
      DmpBgoFiredBar *ab = dynamic_cast<DmpBgoFiredBar*>(ac->fFiredBar->At(i));
      int b = ab->fBar;
      double e = ab->fE;
      if(TMath::Abs(bid - b) <= b1){
        tmpe1 += e;
      }
      if(TMath::Abs(bid - b) <= b2){
        tmpe2 += e;
      }
      if( b == bid){
        seedE += e;
      }
    }
  }
  return TMath::Log10(seedE/tmpe1) / TMath::Log10(seedE/tmpe2);
}

double DmpEvtBgoShower::GetERL3(int l,int type)const
{
  if(this->GetFiredBarNumber(l) < 3) return 0;
  double b = this->GetCoGBarIDInLayer(l);
  double et = this->GetEnergyOfBar(l,b);
  et += this->GetEnergyOfBar(l,b+1);
  et += this->GetEnergyOfBar(l,b-1);
  double xm = fTotE;
  if(type==0){
    xm = this->GetTotalEnergy(l);
    if(xm == 0) return 0;
  }
  return et / xm;
}

double DmpEvtBgoShower::GetER3()const
{
  double x=0;
  for(int l=0;l<BGO_LayerNO;++l){
    x += this->GetERL3(l,1);
  }
  return x;
}

double DmpEvtBgoShower::GetERL3_F2L()const
{
  double first=0;
  double last=0;
  for(int l=0;l<BGO_LayerNO;++l){
    double x = this->GetERL3(l,0);
    if(x > 0){
      last = x;
      if(first == 0){
        first = x;
      }
    }
  }
  return first / last;
}


int DmpEvtBgoShower::GetLayerIDOfMaxERL3()const
{
  double emax =0;
  int n=0;
  for(int l=0;l<BGO_LayerNO;++l){
    double e = this->GetERL3(l,0);
    if(e > emax ){
      n = l;
      emax = e;
    }
  }
  return n;
}

double DmpEvtBgoShower::GetLFractal(int nL1,int nL2)const
{
  std::vector<double> eR = this->GetERatioOfLayers();
  double lid =this->GetLayerIDOfMaxE();
  double te1=0, te2 = 0;
  for(int l=0;l<BGO_LayerNO;++l){
    if(TMath::Abs(l - lid) <= nL1){
      te1 += eR[l];
    }
    if(TMath::Abs(l - lid) <= nL2){
      te2 += eR[l];
    }
  }
  return TMath::Log10(eR[lid] / te1) / TMath::Log10(eR[lid] / te2);
}

double DmpEvtBgoShower::GetFractalBeforeEMaxLayer(int b1,int b2)const
{
  int l=this->GetLayerIDOfMaxE();
  return this->GetFractal(l-1,b1,b2);
}

double DmpEvtBgoShower::GetFractalOfCoG(int b1,int b2)const
{
  int l=this->GetLayerIDOfCoG();
  return this->GetFractal(l,b1,b2);
}

double DmpEvtBgoShower::GetLayerIDOfCoG()const
{
  double v = 0;
  double te =0;
  for(int l=0;l<BGO_LayerNO;++l){
    double e = this->GetTotalEnergy(l);
    if(e >0){
      v += e * (l+1);
      te += e;
    }
  }
  return (v / te -1);
}

double DmpEvtBgoShower::GetTMax()const
{
  int l = this->GetLayerIDOfMaxE();
  double eT = 0,v =0;
  int f = (l-1 < 0)? 0: l-1;
  int t = (l+1 > 13)? 13: l+1;
  for(int il = f;il<=t;++il){
    double x = this->GetTotalEnergy(il);
    v += x * (il+1);
    eT += x;
  }
  return (v / eT -1);
}

int DmpEvtBgoShower::GetLayerIDOfTail(double eCut,int nBar)const
{
  for(int l=BGO_LayerNO-1;l>=0;--l){
    double e = this->GetTotalEnergy(l);
    if(e >= eCut && this->GetFiredBarNumber(l) >= nBar){
      if(this->GetTotalEnergy(l-1) >= eCut){   // next layer has E
        return l;
      }
    }
  }
std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")\t\t no tail\t\tTotal E = "<<fTotE<<std::endl;
  return 0;
}

double DmpEvtBgoShower::GetRMSAtTail()const
{
  int l = this->GetLayerIDOfTail();
  return fRMS[l];
}

double DmpEvtBgoShower::GetFAtTail()const
{
  int l = this->GetLayerIDOfTail();
  return fFValue[l];
}

double DmpEvtBgoShower::GetLogFAtTail()const
{
  return TMath::Log10(this->GetFAtTail());  
}

double DmpEvtBgoShower::GetGAtTail()const
{
  int l = this->GetLayerIDOfTail();
  return this->GetGValue(l);
}

double DmpEvtBgoShower::GetLogGAtTail()const
{
  return TMath::Log10(this->GetGAtTail());
}

double DmpEvtBgoShower::GetMaxDeltaE()const
{
  std::vector<double>  xx = this->GetERatioOfLayers();
  double m =0;
  for(int l = 1;l<13;++l){       // min is 1
    double v = xx[l] - xx[l-1];
    if(v > m){
      m = v;
    }
  }
  return m;
}

double DmpEvtBgoShower::GetERL3OfMaxDeltaE()const
{
  int l = this->GetLayerIDOfMaxDeltaE();
  return this->GetERL3(l);
}

int DmpEvtBgoShower::GetLayerIDOfMaxDeltaE()const
{
  std::vector<double>  xx = this->GetERatioOfLayers();
  int n = -1;
  double m =0;
  for(int l = 1;l<13;++l){       // min is 1
    double v = xx[l] - xx[l-1];
    if(v > m){
      m = v;
      n = l;
    }
  }
  return n;
}

int DmpEvtBgoShower::GetMaxDeltaBarNo()const
{
  std::vector<int>  xx = this->GetFiredBarNumbers();//ERatioOfLayers();
  int m =0;
  for(int l = 1;l<13;++l){       // min is 1
    int v = xx[l] - xx[l-1];
    if(v > m){
      m = v;
    }
  }
  return m;
}
 
int DmpEvtBgoShower::GetLayerIDOfMaxDeltaBarNo()const
{
  std::vector<int>  xx = this->GetFiredBarNumbers();//ERatioOfLayers();
  int m =0;
  int n=0;
  for(int l = 1;l<13;++l){       // min is 1
    int v = xx[l] - xx[l-1];
    if(v > m){
      m = v;
      n = l;
    }
  }
  return n;
}

double DmpEvtBgoShower::GetERatioAtTail()const
{
  int l = this->GetLayerIDOfTail();
  return this->GetERatioOfLayer(l);
}

double DmpEvtBgoShower::GetLogERatioAtTail()const
{
  return TMath::Log10(this->GetERatioAtTail());
}

double DmpEvtBgoShower::GetMaxGValueFromTail()const
{
  double gm = 0;
  for(int l=1;l<BGO_LayerNO;++l){
    double t = this->GetGValue(l);
    if(t > gm){
      gm = t;
    }
  }
  return gm;
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
