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

  DmpBgoFiredBar *GetSeedBar()const;
  int    GetBarNumber(double eCutLow=2.5)const;
  double GetTotalEnergy(int whichSide=-1)const;  // whichside = {-1,0,1}= {combined | side_0 | side_1}
  double GetERatioOfSeedBar()const;  // n bars around seed bar
  double GetWindowEnergy(int n=2)const;  // n bars around seed bar
  double GetS1ToSn(int nBarClose2Seed = 2)const;
  double GetFractal(int nBars1,int nBars2)const;
  double GetCoGBarID()const;
  double GetRMS()const;
  double GetSeedRatio2Size()const;  // ES = Energy of seed bar / fired bar numbers

public:
  void MyPrint()const;

public:
  void AddNewFiredBar(DmpBgoFiredBar *aBar);

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

public:
  int GetLayerIDOfMaxE(int fromWhichLayer=0,bool check=true)const;
  int GetLayerIDOfMinE()const;
  int GetLayerIDOfMaxFiredBarNumber()const;
  int GetLayerIDOfMaxRMS()const;
  int GetLayerIDOfMinRMS()const;    // exclude un-fired layers
  int GetLayerIDOfMaxFValue()const;
  int GetLayerIDOfMinFValue()const;
  int GetLayerIDOfMaxGValue()const;
  int GetLayerIDOfMinGValue()const;
  int GetLayerIDOfTail(double eCut=15,int nBarCut=1)const;  // last layer has e. NOTE:  at least 15MeV(default eCut), and e of the layer before the last layer has e > eCut
  int GetLayerIDOfMaxERL3()const;
  int GetLayerIDOfMaxDeltaE()const;   // find delatE = ERatio[i] - ERatio[i-1], find the max value, return i
  int GetLayerIDOfMaxDeltaBarNo()const;   // find the position of deltaE = firedBarNum[i] - BarNum[i-1]
  double GetLayerIDOfCoG()const; // center along z axis
  double GetTMax()const; // position of E max along z axis

  Position GetEntryPoint()const;
  Direction GetTrackDirection()const;
  double GetTotalRMS(int type=-1, bool gap=false)const;      // -1: all;  0: x layers; 1: y layers
  double GetRMS()const;    // sqrt(rms_max_x**2 + rms_max_y**2)
  double GetRMSOfEMaxBarID(int type=-1)const;   // -1, all, 0: x; 1: y

  double GetTotalEnergy(int layerID = -1, int whichSide=-1)const;   // layer ID == -1, total energy of all layers.  whichside = {-1,0,1}= {combined | side_0 | side_1}
  int GetFiredBarNumber(int layerID = -1,double eCutLow=2.5)const;
  int GetFiredBarNumberAtTail()const;
  int GetMaxFiredBarNumber()const;
  std::vector<int> GetFiredBarNumbers(double eCutLow = 2.5)const;

  double GetEnergyOfBar(int layerID, int barID)const;
  double GetCoGBarIDInLayer(int layerID)const;
  Position GetCoGPositionInLayer(int layerID)const;
  double GetGValue(int layerID)const; // define G = Log10(RMS^2 * E_total / E_layer)

  double GetEnergyOfEMaxLayer()const;
  double GetEnergyOfEMinLayer()const;
  double GetERatioOfLayer(int layerID)const;
  std::vector<double> GetERatioOfLayers()const;
  double GetERatioOfEMaxLayer()const;
  double GetERatioOfCoGZ()const;     // ***
  double GetERatioOfEMinLayer()const;
  double GetERatioOfEMaxClusterInLayer(int layerID)const;
  double GetERatioOfEMaxClusterInCoG()const;
  double GetERatioOfEMinClusterInLayer(int layerID)const;
  double GetERatioOfEMinClusterInEMinLayer()const;
  double GetERatioOfEMaxClusterInMaxRMSLayer()const;
  double GetERL3(int layerID,int type=0)const;    // type 0: over E_l;    type 1: over E_total
  double GetERL3OfMaxDeltaE()const;
  double GetER3()const;  // over E_total
  double GetERL3_F2L()const; //
  double GetEMax_ETail()const;    // E_maxLayer / E at tail
  double GetWindowEnergy(int nBars=1,int nHalf=1)const;  //nHalf: include how may layers above(below) seed layer. Totally: 2*nHalf + 1 layers
  double GetWindowERatio(int nBars=1,int nHalf = 1)const;

  double GetRMSOfEMaxLayer()const;
  double GetRMSOfCoG()const;
  double GetGValueOfEMaxLayer()const;
  double GetGValueOfCoG()const;
  double GetMaxRMS()const;
  double GetMaxRMSFromTail()const;
  double GetMaxFValue()const;
  double GetMinRMS()const;  // exclude un-fired layers
  double GetMaxGValue()const;   // from layer 0
  double GetMinGValue()const;   // from layer 0
  double GetMaxGValueFromTail()const;   // from layer 1


public:     // comfirmed PID var.
  double GetERMax(int nLayers=2)const;
  double GetERMin(int nLayers=2)const;
  double GetLogFAtTail()const;
  double GetERT2()const;        // Log10(GetERMin(2))

public:
  double GetMaxDeltaE()const;  // find delatE = ERatio[i] - ERatio[i-1], find the max value and position
  int    GetMaxDeltaBarNo()const;   // find the max of deltaE = firedBarNum[i] - BarNum[i-1]
  double GetERatioAtTail()const;        // *****
  double GetLogERatioAtTail()const;
  double GetRMSAtTail()const;
  double GetFAtTail()const;
  double GetGAtTail()const;     // ***
  double GetLogGAtTail()const;     // ***

  int GetClusterNoInLayer(int lID,double seedECut_low=23, double seedECut_h = 9999999)const;
  int GetClusterNo(double eCutLow=23, double eCutHigh = 9999999)const;
  double GetFractal(int layerID, int nBars1,int nBars2)const;
  double GetFractal(int nBars1,int nBars2)const;
  double GetLFractal(int nLayers1,int nLayers2)const;
  double GetFractalBeforeEMaxLayer(int nBars1,int nBars2)const; // ***
  double GetFractalOfEMaxLayer(int nBars1,int nBars2)const; // ***
  double GetFractalOfCoG(int nBars1=1,int nBars2=3)const; // ****  CoG of Z axis
  double GetFractalOfGMaxLayer(int nBars1,int nBars2)const; // useless
  double GetFractalOfFMaxLayer(int nBars1,int nBars2)const;
  double GetFractalOfRMSMaxLayer(int nBars1,int nBars2)const;

public:
  DmpEvtBgoCluster *GetEMaxCluster()const;
  DmpEvtBgoCluster *GetEMaxClusterInEMaxLayer()const;
  DmpEvtBgoCluster *GetEMaxClusterInCoGLayer()const;
  DmpEvtBgoCluster *GetEMaxClusterInGMinLayer()const;
  DmpEvtBgoCluster *GetEMaxClusterInLayer(int layerID)const;
  DmpEvtBgoCluster *GetEMinClusterInLayer(int layerID)const;
  std::vector<DmpEvtBgoCluster*> GetAllClusterInLayer(int layerID)const;
  std::vector<double> GetERatioOfClustersInLayer(int layerID)const;

public:
  DmpBgoFiredBar*   GetBar(int layerid, int barID)const;
  DmpBgoFiredBar*   GetEMaxBar()const;
  DmpBgoFiredBar*   GetEMaxBarInLayer(int layerID)const;
  std::vector<DmpBgoFiredBar*>  GetBars(int layerID,double eBarCut_Low,double eClusterCut)const;

  std::vector<DmpBgoFiredBar*>  GetIsolatedBar(std::vector<int> layerIDs,double noise=2.5)const;  // if nextBar.fE < noise, the current bar is isolated
  DmpBgoFiredBar* GetEMaxIsolatedBar(std::vector<int> LayerIDs,double noise = 2.5)const;
  DmpBgoFiredBar* GetEMinIsolatedBar(std::vector<int> fromLayerID,double noise = 2.5)const;
  std::vector<DmpBgoFiredBar*>  GetIsolatedBar(std::vector<int> layerIDs,double eLow,double eHigh,double noise=2.5)const;  // if nextBar.fE < noise, the current bar is isolated, only return isolated bar whose energy > eLow and < eHigh
  int GetLayerIDOfMostIsolatedBars(double eLow,double eHigh,double noise=2.5)const;
  int GetIsolatedBarNo(std::vector<int> L,double elow, double eHigh, double noise = 2.5)const;
  double GetBRMSOfIsolatedBar(std::vector<int> l,double eLow,double eHigh,double noise = 2.5)const;
  double GetLRMSOfIsolatedBar(std::vector<int> l,double eLow,double eHigh,double noise = 2.5)const;

public: // for trigger
  bool T0(double threshold = 0.2)const;      // energy of any bar of first layer > 0.2 Mips (>4.6MeV)
  bool Group0_11(double threshold_L0_n8=4.35, double threshold_L1_n8=8.7,double threshold_L2_n8=8.7,double threshold_L3_n8=8.7)const;    //  l0_d8_n | l1_d8_n | l2_d8_n | l3_d8_n
  bool Group1_10(double threshold_L2_p8=0.5,double threshold_L10_P8=0.5,double threshold_L12_P8=0.5)const;    //  l2_d8_p & l10_d8_p & l12_d8_p
  bool Group2_10(double threshold_L3_p8=0.5,double threshold_L11_p8=0.5,double threshold_L13_p8=0.5)const;    //  l3_d8_p & l11_d8_p & l13_d8_p
  bool Group3_else(double threshold_L0_n5=8.7,double threshold_L1_n8=8.7,double threshold_L2_n8=8.7,double threshold_L3_n8=8.7)const;  //  l0_d5_n & l1_d8_n & l2_d8_n & l3_d8_n
  bool Group4_001(double threshold_L0_P8=0.2,double threshold_L1_P8=0.5,double threshold_L2_P8=0.5,double threshold_L3_P8=0.5,double threshold_L10_P8=0.5,double threshold_L11_P8=0.5,double threshold_L12_P8=0.5,double threshold_L13_p8=0.5)const;   //  l0_d8_p & l1_d8_p & l2_d8_p & l3_d8_p & !(l10_d8_p | l11_d8_p | l12_d8_p | l13_d8_p)

  bool Group0_01(double threshold)const;    //  l0_d8_p | l1_d8_p | l2_d8_p | l3_d8_p
  bool Group0_10(double threshold)const;    //  l0_d5_p | l1_d5_p | l2_d5_p | l3_d5_p
  bool Group1_00(double threshold)const;    //  (l0_d8_p | l1_d8_p) & (l2_d8_p | l3_d8_p) & (l10_d8_p | l11_d8_p) & (l12_d8_p | l13_d8_p)
  bool Group1_01(double threshold)const;    //  (l0_d8_n | l1_d8_n) & (l12_d8_n | l13_d8_n)
  bool Group1_11(double threshold)const;    //  l0_d8_n & l12_d8_n
  bool Group2_00(double threshold)const;    //  l0_d8_p & l1_d8_p & l2_d8_p & l3_d8_p & l10_d8_p & l11_d8_p & l12_d8_p & l13_d8_p
  bool Group2_01(double threshold)const;    //  l0_d8_n & l1_d8_n & l12_d8_n & l13_d8_n
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
  bool Group4_000(double threshold)const;   //  l0_d8_p & l1_d8_p & l2_d8_p & l3_d8_p
  bool Group4_010(double threshold)const;   //  l0_d8_p & l1_d8_p & l2_d8_p & l3_d8_p & !(l10_d8_n | l11_d8_n | l12_d8_n | l13_d8_n)
  bool Group4_011(double threshold)const;   //  l0_d8_n & l1_d8_p & l2_d8_p & l3_d8_p & !(l10_d8_p | l11_d8_p | l12_d8_p | l13_d8_p)
  bool Group4_100(double threshold)const;   //  (l0_d8_p | l1_d8_p | l2_d8_p | l3_d8_p) & !(l10_d8_p | l11_d8_p | l12_d8_p | l13_d8_p)
  bool Group4_101(double threshold)const;   //  l0_d8_n & l1_d8_n & l2_d8_n & l3_d8_n & !(l10_d8_p | l11_d8_p | l12_d8_p | l13_d8_p)
  bool Group4_110(double threshold)const;   //  l0_d8_n & l1_d8_n & l2_d8_n & l3_d8_n & !(l10_d8_n | l11_d8_n | l12_d8_n | l13_d8_n)
  bool Group4_111(double threshold)const;   //  l0_d8_n & l1_d8_n & l2_d8_n & l3_d8_n & l10_d8_n & l11_d8_n & l12_d8_n & l13_d8_n

public:
  double  CalculateTotalE(std::vector<DmpBgoFiredBar*> right)const;
  TVector3  CalculatePosition(std::vector<DmpBgoFiredBar*> right)const;
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


