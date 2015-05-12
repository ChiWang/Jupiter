/*
 *  $Id: DmpEvtBgoShower.h, 2015-01-21 19:52:06 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 18/01/2015
*/

#ifndef DMPEVT_BgoShower_H
#define DMPEVT_BgoShower_H

#include <vector>
#include "TVector3.h"
#include "TClonesArray.h"

#define BGO_LayerNO 14
#define BGO_BarNO 22

typedef TVector3 Position;
typedef TVector3 Direction;

class DmpBgoFiredBar : public TObject
{
public:
  DmpBgoFiredBar();
  DmpBgoFiredBar(int l,int b,double e=0,double e0=0,double e1=0);
  DmpBgoFiredBar(int l,int b,double e,double e0,double e1,TVector3 v);
  ~DmpBgoFiredBar();
  void Reset();
  void LoadFrom(const DmpBgoFiredBar *&r);
  void MyPrint()const;

  int       fLayer; // 0~13
  int       fBar;   // 0~21
  double    fE;    // unit: MeV. combined E
  double    fE0;   // side 0
  double    fE1;   // side 1
  Position  fPosition;   // unit: mm

  ClassDef(DmpBgoFiredBar,1);
};

class DmpEvtBgoCluster : public TObject
{
public:
  DmpEvtBgoCluster();
  DmpEvtBgoCluster(int l, int seedBarID,double totalE=0);
  ~DmpEvtBgoCluster();

  void Reset();
  void LoadFrom(const DmpEvtBgoCluster *&r);
  int GetClusterSize()const;   // how many fired bar
  DmpBgoFiredBar *GetSeedBar()const;
  double GetWindowEnergy(int n=3)const;  // n bars around seed bar
  double GetCoGBarID()const;
  double GetTotalEnergy(int whichSide=-1)const;  // whichside = {-1,0,1}= {combined | side_0 | side_1}

  void AddNewFiredBar(DmpBgoFiredBar *aBar);
  void MyPrint()const;

public:
  int       fLayer;     // layer id, from 0~13
  int       fSeedBarID; // bar ID of max DmpBgoFiredBar::fE
  double    fTotE;      // total energy. MeV
  Position  fCenter;    // cluster center. unit: mm. NOTE:  one dimension is wrong: layer 0,2,4... y is right, x is wrong
  TClonesArray *fFiredBar;  // ordered by DmpFiredBar::fE, from max to min

  ClassDef(DmpEvtBgoCluster,1)
};

class DmpEvtBgoShower : public TObject
{
public:
  DmpEvtBgoShower();
  ~DmpEvtBgoShower();

  void Reset();
  void LoadFrom(const DmpEvtBgoShower *&r);
  void MyPrint(bool allInformation = true)const;

  DmpEvtBgoCluster* GetSeedCluster()const;
  Position GetEntryPoint()const;
  Direction GetTrackDirection()const;
  double GetTotalEnergy(int layerID = -1, int whichSide=-1)const;   // layer ID == -1, total energy of all layers.  whichside = {-1,0,1}= {combined | side_0 | side_1}
  double GetTotalRMS()const;
  double GetPileupRatio()const;     // (-1,0): some layer not fired. 0: one track. > 1: multi-track
  int GetFiredBarNumber(int layerID = -1)const;

  double GetEnergyOfBar(int layerID, int barID)const;
  double GetCoGBarIDInLayer(int layerID)const;
  Position GetCoGPositionInLayer(int layerID)const;
  double GetNormalizedRMS(int layerID)const;
  double GetGValue(int layerID)const; // define G = RMS^2 * E_total / E_layer

  double GetEnergyOfEMaxLayer()const;
  double GetEnergyRatioOfEMaxLayer()const;
  double GetWindowEnergy(int nBars=3,int nHalf=1)const;  //nHalf: include how may layers above(below) seed layer. Totally: 2*nHalf + 1 layers
  double GetWindowEnergyRatio(int nBars=3,int nHalf = 1)const;
  double GetRMSOfEMaxLayer()const;
  double GetGValueOfEMaxLayer()const;
  int GetLayerIDOfMaxE()const;
  int GetLayerIDOfMaxFiredBarNo()const;
  int GetLayerIDOfMaxGValue()const;
  int GetLayerIDOfMinGValue()const;
  int GetLayerIDOfMaxRMS()const;
  int GetLayerIDOfMinRMS()const;    // exclude un-fired layers
  double GetMaxRMS()const;
  double GetMaxFValue()const;
  double GetMinRMS()const;  // exclude un-fired layers
  double GetMaxGValue()const;
  double GetMyValue(int nHalfLayer = 2)const;

  std::vector<DmpEvtBgoCluster*> GetAllClusterInLayer(int layerID)const;
  DmpEvtBgoCluster *GetMaxClusterInLayer(int layerID)const;
  DmpBgoFiredBar*   GetEMaxBar()const;
  std::vector<DmpBgoFiredBar*>  GetIsolatedBar(int layerID,double noise=2)const;  // if nextBar.fE < noise, the current bar is isolated
  std::vector<DmpBgoFiredBar*>  GetIsolatedBar(int layerID,double eLow,double eHigh,double noise=2)const;  // if nextBar.fE < noise, the current bar is isolated, only return isolated bar whose energy > eLow and < eHigh
  std::vector<DmpBgoFiredBar*>  GetIsolatedBarFromLayer(int layerID,double noise=2)const; // isolated bar from layerID ~ 13
  std::vector<DmpBgoFiredBar*>  GetIsolatedBarFromLayer(int layerID,double eLow, double eHi,double noise=2)const; // isolated bar from layerID ~ 13, only return bar e in range(eLow,eHi)

public: // for trigger
  bool T0(double threshold = 0.2)const;      // energy of any bar of first layer > 0.2 Mips (>4.6MeV)
  bool Group0_01(double threshold)const;    //  l0_d8_p | l1_d8_p | l2_d8_p | l3_d8_p
  bool Group0_10(double threshold)const;    //  l0_d5_p | l1_d5_p | l2_d5_p | l3_d5_p
  bool Group0_11(double threshold)const;    //  l0_d8_n | l1_d8_n | l2_d8_n | l3_d8_n
  bool Group1_00(double threshold)const;    //  (l0_d8_p | l1_d8_p) & (l2_d8_p | l3_d8_p) & (l10_d8_p | l11_d8_p) & (l12_d8_p | l13_d8_p)
  bool Group1_01(double threshold)const;    //  (l0_d8_n | l1_d8_n) & (l12_d8_n | l13_d8_n)
  bool Group1_10(double threshold)const;    //  l2_d8_p & l10_d8_p & l12_d8_p
  bool Group1_11(double threshold)const;    //  l0_d8_n & l12_d8_n
  bool Group2_00(double threshold)const;    //  l0_d8_p & l1_d8_p & l2_d8_p & l3_d8_p & l10_d8_p & l11_d8_p & l12_d8_p & l13_d8_p
  bool Group2_01(double threshold)const;    //  l0_d8_n & l1_d8_n & l12_d8_n & l13_d8_n
  bool Group2_10(double threshold)const;    //  l3_d8_p & l11_d8_p & l13_d8_p
  bool Group2_11(double threshold)const;    //  l1_d8_n & l13_d8_n
  bool Group3_0000(double threshold)const;  //  l0_d8_n & l1_d8_n & l2_d8_n & l3_d8_n
  bool Group3_0001(double threshold)const;  //  l0_d5_n & l1_d5_n & l2_d8_n & l3_d8_n
  bool Group3_0010(double threshold)const;  //  l0_d5_n & l1_d5_n & l2_d5_n & l3_d8_n
  bool Group3_0011(double threshold)const;  //  l0_d5_n & l1_d5_n & l2_d5_n & l3_d5_n
  bool Group3_0100(double threshold)const;  //  l0_d8_n & l1_d8_n & l2_d5_n & l3_d5_n
  bool Group3_0101(double threshold)const;  //  (l0_d5_n | l1_d8_n) & l2_d8_n & l3_d8_n
  bool Group3_0110(double threshold)const;  //  l0_d5_n & l1_d8_n & l2_d8_n & l3_d8_n & (l10_d8_n | l11_d8_n | l12_d8_n | l13_d8_n)
  bool Group3_0111(double threshold)const;  //  l0_d5_p & l1_d5_p & l2_d5_p & l3_d5_p & (l10_d8_p | l11_d8_p | l12_d8_p | l13_d8_p)
  bool Group3_1000(double threshold)const;  //  (l0_d5_p | l0_d5_n) & (l1_d5_p | l1_d8_n) & (l2_d5_p | l2_d8_n) (l3_d5_p | l3_d8_n)
  bool Group3_1001(double threshold)const;  //  (l0_d5_p & l1_d5_p & l2_d5_p & l3_d5_p) | (l10_d5_p & l11_d5_p & l12_d5_p & l13_d5_p)
  bool Group3_1010(double threshold)const;  //  (l0_d5_n & l1_d5_n & l2_d5_n & l3_d5_n) | (l10_d5_n & l11_d5_n & l12_d5_n & l13_d5_n)
  bool Group3_else(double threshold)const;  //  l0_d5_n & l1_d8_n & l2_d8_n & l3_d8_n
  bool Group4_000(double threshold)const;   //  l0_d8_p & l1_d8_p & l2_d8_p & l3_d8_p
  bool Group4_001(double threshold)const;   //  l0_d8_p & l1_d8_p & l2_d8_p & l3_d8_p & !(l10_d8_p | l11_d8_p | l12_d8_p | l13_d8_p)
  bool Group4_010(double threshold)const;   //  l0_d8_p & l1_d8_p & l2_d8_p & l3_d8_p & !(l10_d8_n | l11_d8_n | l12_d8_n | l13_d8_n)
  bool Group4_011(double threshold)const;   //  l0_d8_n & l1_d8_p & l2_d8_p & l3_d8_p & !(l10_d8_p | l11_d8_p | l12_d8_p | l13_d8_p)
  bool Group4_100(double threshold)const;   //  (l0_d8_p | l1_d8_p | l2_d8_p | l3_d8_p) & !(l10_d8_p | l11_d8_p | l12_d8_p | l13_d8_p)
  bool Group4_101(double threshold)const;   //  l0_d8_n & l1_d8_n & l2_d8_n & l3_d8_n & !(l10_d8_p | l11_d8_p | l12_d8_p | l13_d8_p)
  bool Group4_110(double threshold)const;   //  l0_d8_n & l1_d8_n & l2_d8_n & l3_d8_n & !(l10_d8_n | l11_d8_n | l12_d8_n | l13_d8_n)
  bool Group4_111(double threshold)const;   //  l0_d8_n & l1_d8_n & l2_d8_n & l3_d8_n & l10_d8_n & l11_d8_n & l12_d8_n & l13_d8_n

private:
  bool _triggerFromLayer(int layerID,double threshold)const;

public:
  DmpEvtBgoCluster* AddNewCluster(DmpBgoFiredBar *seedBar);
  void Calculation();   // fTotE, Direction fit parameters

public:
  double        fTotE;          // total energy. MeV
  double        fLRMS;          // Longitudinal RMS, by using total energy in each layer, and layer ID along Z axis
  double        fRMS[BGO_LayerNO];  // for each layer. if == -1: no fired bar in this layer. if == 0: only fired one bar
  double        fFValue[BGO_LayerNO]; // for each layer. if == -1: no fired bar in this layer. if == 0: only fired one bar
  double        fXZFitPar[5];      // linear fit parameter of x_z. [0,1] value of parameter 0 and 1. [2,3] error of parameter 0 and 1. [4]: Chi2
  double        fYZFitPar[5];      // linear fit parameter of y_z
  TClonesArray  *fClusters;

  ClassDef(DmpEvtBgoShower,1)
};

/*
class DmpBgoIsolatedBars
{
public:
  DmpBgoIsolatedBars();
  DmpBgoIsolatedBars(DmpEvtBgoShower *fromMe);
  DmpBgoIsolatedBars(DmpEvtBgoShower *fromMe,double eLowOfBar, double eHighOfBar,double noiseCut=2);
  DmpBgoIsolatedBars(DmpEvtBgoShower *fromMe,double eLowOfBar, double eHighOfBar,int fromLayerID,double noiseCut=2);
  ~DmpBgoIsolatedBars();

public:
  void LinkBgoShower(DmpEvtBgoShower *r){fEvtBgoShower = r;}
  void SetLayerCut(int l){fFromLayerID = l;}
  void SetNoiseCut(double v) {fECutNoise = v;}
  void SetIsolatedBarERange(double low, double h) {fECutLow = low;fECutHigh = h;}

public:
  bool UpdateEvent();   // step 0: update fIsolatedBars, step 1: update other datas
  void Reset();

private:    // paramters
  DmpEvtBgoShower   *fEvtBgoShower; // all from this event
  double            fECutLow;       // energy of isolated bar must bigger than fECutLow
  double            fECutHigh;      // energy of isolated bar must small than fECutHigh
  int               fFromLayerID;   // get Isolated from this layerID ~ 13
  double            fECutNoise;     // parameter of DmpEvtBgoShower::GetIsolatedBarFromLayer()

private:    // update those value in UpdateEvent()
  std::vector<DmpBgoFiredBar*>      fIsolatedBars; // all choosed isolated bars

public:
  int       fNIsoBars;         // number of isolated bars
  double    fETotal;        // total energy of all isolated bars
  double    fEMean;         // mean energy
  double    fRMSLayer;      // layer id of all isolated bars, get them RMS
  double    fRMSBar;        // bar id RMS
  double    fRMSBar_L0;     // bar id RMS for all isolated bar which layerID % 2 == 0
  double    fRMSBar_L1;     // bar id RMS for all isolated bar which layerID % 2 == 1
  double    fBarIDMean_L0;  // 
  double    fBarIDMean_L1;  // 
  int       fNBarID;        // number of unique bar id
  int       fNLayerID;       // number of unique layer id
  int       fFirstLID;      // first layer ID
};
*/


#endif


