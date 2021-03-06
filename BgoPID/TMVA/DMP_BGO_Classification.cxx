// @(#)root/tmva $Id$
/**********************************************************************************
 * Project   : TMVA - a ROOT-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Root Macro: TMVAClassification                                                 *
 *                                                                                *
 * This macro provides examples for the training and testing of the               *
 * TMVA classifiers.                                                              *
 *                                                                                *
 * As input data is used a toy-MC sample consisting of four Gaussian-distributed  *
 * and linearly correlated input variables.                                       *
 *                                                                                *
 * The methods to be used can be switched on and off by means of booleans, or     *
 * via the prompt command, for example:                                           *
 *                                                                                *
 *    root -l ./TMVAClassification.C\(\"Fisher,Likelihood\"\)                     *
 *                                                                                *
 * (note that the backslashes are mandatory)                                      *
 * If no method given, a default set of classifiers is used.                      *
 *                                                                                *
 * The output file "TMVA.root" can be analysed with the use of dedicated          *
 * macros (simply say: root -l <macro.C>), which can be conveniently              *
 * invoked through a GUI that will appear at the end of the run of this macro.    *
 * Launch the GUI via the command:                                                *
 *                                                                                *
 *    root -l ./TMVAGui.C                                                         *
 *                                                                                *
 **********************************************************************************/

#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
//#include "TObjString.h"
//#include "TSystem.h"
#include "TROOT.h"
#include "TMVAGui.C"


#if not defined(__CINT__) || defined(__MAKECINT__)
// needs to be included when makecint runs (ACLIC)
#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#endif

#include "DmpEvtBgoShower.h"
#include "DmpEvtHeader.h"

#define __treeName  "/Event/Rec0"

namespace Conf
{
  std::vector<TString>  signalFiles(1,"./MC_eletron-150GeV_P43-Evts32000.root");
  std::vector<TString>  backgroundFiles(1,"./MC_proton-400GeV_P43-Evts142200.root");
  //signalFiles.push_back("./MC_eletron-150GeV_P43-Evts32000.root");
  //backgroundFiles.push_back("./MC_proton-400GeV_P43-Evts72200.root");

  TString myMethodList = "MLP,MLPBNN,BDTG,BDT";
  TString BDTGOpt = "!H:!V:NTrees=40:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=3";
  TString outputFileName = "TMVA.root";//myMethodList+"__TMVA";

   // Apply additional cuts on the signal and background samples (can be different)
   TCut T0 = "Bgo.T0()";
   TCut Trig3_0000 = "Bgo.Group3_0000(0.2)";
   TCut MipsCut = "Bgo.fTotE > 200 && Bgo.fTotE <450 && Bgo.fLRMS > 3.6 && Bgo.fLRMS < 4.4 && Bgo.GetTotalRMS()>-2 && Bgo.GetTotalRMS()<2";
   TCut SignalCut = "";
   TCut BackgroundCut = "Bgo.fTotE > 135000 && Bgo.fTotE< 150000";
   void BackgroundERange(double eLow,double eHigh){
      BackgroundCut = Form("Bgo.fTotE > %f && Bgo.fTotE< %f",eLow,eHigh);
   }
   //TCut mycutc = "Bgo.fTotE >3400 && Bgo.fTotE < 4000"; // for example: TCut BackgroundCut = "abs(var1)<0.5";
   //TCut ERT2cut= "Bgo.GetERMin(2)>0";
   void PrintInfor()
   {
     cout<<"\nused method:\t" <<myMethodList<<endl;
     cout<<"outputFile:\t"<<outputFileName<<endl;
     cout<<"intput signal files:\n";
     for(size_t i=0;i<signalFiles.size();++i){
       cout<<"\t\t"<<signalFiles.at(i)<<endl;
     }
     cout<<"intput background files:\n";
     for(size_t i=0;i<backgroundFiles.size();++i){
       cout<<"\t\t"<<backgroundFiles.at(i)<<endl;
     }
     cout<<"signal cut:\t"  <<SignalCut.GetTitle()<<endl;
     cout<<"background cut:\t"<< BackgroundCut.GetTitle()<<endl;
   }
};

bool BookMethod(TMVA::Factory *fac)
{
  std::map<std::string,int> Use;
  // default methods
  {
   // --- Cut optimisation
   Use["Cuts"]            = 1;
   Use["CutsD"]           = 1;
   Use["CutsPCA"]         = 0;
   Use["CutsGA"]          = 0;
   Use["CutsSA"]          = 0;
   // 
   // --- 1-dimensional likelihood ("naive Bayes estimator")
   Use["Likelihood"]      = 1;
   Use["LikelihoodD"]     = 0; // the "D" extension indicates decorrelated input variables (see option strings)
   Use["LikelihoodPCA"]   = 1; // the "PCA" extension indicates PCA-transformed input variables (see option strings)
   Use["LikelihoodKDE"]   = 0;
   Use["LikelihoodMIX"]   = 0;
   //
   // --- Mutidimensional likelihood and Nearest-Neighbour methods
   Use["PDERS"]           = 1;
   Use["PDERSD"]          = 0;
   Use["PDERSPCA"]        = 0;
   Use["PDEFoam"]         = 1;
   Use["PDEFoamBoost"]    = 0; // uses generalised MVA method boosting
   Use["KNN"]             = 1; // k-nearest neighbour method
   //
   // --- Linear Discriminant Analysis
   Use["LD"]              = 1; // Linear Discriminant identical to Fisher
   Use["Fisher"]          = 0;
   Use["FisherG"]         = 0;
   Use["BoostedFisher"]   = 0; // uses generalised MVA method boosting
   Use["HMatrix"]         = 0;
   //
   // --- Function Discriminant analysis
   Use["FDA_GA"]          = 1; // minimisation of user-defined function using Genetics Algorithm
   Use["FDA_SA"]          = 0;
   Use["FDA_MC"]          = 0;
   Use["FDA_MT"]          = 0;
   Use["FDA_GAMT"]        = 0;
   Use["FDA_MCMT"]        = 0;
   //
   // --- Neural Networks (all are feed-forward Multilayer Perceptrons)
   Use["MLP"]             = 0; // Recommended ANN
   Use["MLPBFGS"]         = 0; // Recommended ANN with optional training method
   Use["MLPBNN"]          = 1; // Recommended ANN with BFGS training method and bayesian regulator
   Use["CFMlpANN"]        = 0; // Depreciated ANN from ALEPH
   Use["TMlpANN"]         = 0; // ROOT's own ANN
   //
   // --- Support Vector Machine 
   Use["SVM"]             = 1;
   // 
   // --- Boosted Decision Trees
   Use["BDT"]             = 1; // uses Adaptive Boost
   Use["BDTG"]            = 0; // uses Gradient Boost
   Use["BDTB"]            = 0; // uses Bagging
   Use["BDTD"]            = 0; // decorrelation + Adaptive Boost
   Use["BDTF"]            = 0; // allow usage of fisher discriminant for node splitting 
   // 
   // --- Friedman's RuleFit method, ie, an optimised series of cuts ("rules")
   Use["RuleFit"]         = 1;
  }

  // Select methods (don't look at this code - not of interest)
   if (Conf::myMethodList != "") {
      for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++){
        it->second = 0;
      }
      std::vector<TString> mlist = TMVA::gTools().SplitString(Conf::myMethodList, ',');
      for (UInt_t i=0; i<mlist.size(); i++) {
         std::string regMethod(mlist[i]);
         if (Use.find(regMethod) == Use.end()) {
            std::cout << "Method \"" << regMethod << "\" not known in TMVA under this name. Choose among the following:" << std::endl;
            for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) std::cout << it->first << " ";
            std::cout << std::endl;
            return false;
         }else{
           Use[regMethod] = 1;
         }
      }
   }

  // link to TMVA::Factory
  {
   // ---- Book MVA methods
   //
   // Please lookup the various method configuration options in the corresponding cxx files, eg:
   // src/MethoCuts.cxx, etc, or here: http://tmva.sourceforge.net/optionRef.html
   // it is possible to preset ranges in the option string in which the cut optimisation should be done:
   // "...:CutRangeMin[2]=-1:CutRangeMax[2]=1"...", where [2] is the third input variable
   // Cut optimisation
   if (Use["Cuts"])
      fac->BookMethod( TMVA::Types::kCuts, "Cuts",
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart" );

   if (Use["CutsD"])
      fac->BookMethod( TMVA::Types::kCuts, "CutsD",
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=Decorrelate" );

   if (Use["CutsPCA"])
      fac->BookMethod( TMVA::Types::kCuts, "CutsPCA",
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=PCA" );

   if (Use["CutsGA"])
      fac->BookMethod( TMVA::Types::kCuts, "CutsGA",
                           "H:!V:FitMethod=GA:CutRangeMin[0]=-10:CutRangeMax[0]=10:VarProp[1]=FMax:EffSel:Steps=30:Cycles=3:PopSize=400:SC_steps=10:SC_rate=5:SC_factor=0.95" );

   if (Use["CutsSA"])
      fac->BookMethod( TMVA::Types::kCuts, "CutsSA",
                           "!H:!V:FitMethod=SA:EffSel:MaxCalls=150000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );

   // Likelihood ("naive Bayes estimator")
   if (Use["Likelihood"])
      fac->BookMethod( TMVA::Types::kLikelihood, "Likelihood",
                           "H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmoothBkg[1]=10:NSmooth=1:NAvEvtPerBin=50" );

   // Decorrelated likelihood
   if (Use["LikelihoodD"])
      fac->BookMethod( TMVA::Types::kLikelihood, "LikelihoodD",
                           "!H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate" );

   // PCA-transformed likelihood
   if (Use["LikelihoodPCA"])
      fac->BookMethod( TMVA::Types::kLikelihood, "LikelihoodPCA",
                           "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA" ); 

   // Use a kernel density estimator to approximate the PDFs
   if (Use["LikelihoodKDE"])
      fac->BookMethod( TMVA::Types::kLikelihood, "LikelihoodKDE",
                           "!H:!V:!TransformOutput:PDFInterpol=KDE:KDEtype=Gauss:KDEiter=Adaptive:KDEFineFactor=0.3:KDEborder=None:NAvEvtPerBin=50" ); 

   // Use a variable-dependent mix of splines and kernel density estimator
   if (Use["LikelihoodMIX"])
      fac->BookMethod( TMVA::Types::kLikelihood, "LikelihoodMIX",
                           "!H:!V:!TransformOutput:PDFInterpolSig[0]=KDE:PDFInterpolBkg[0]=KDE:PDFInterpolSig[1]=KDE:PDFInterpolBkg[1]=KDE:PDFInterpolSig[2]=Spline2:PDFInterpolBkg[2]=Spline2:PDFInterpolSig[3]=Spline2:PDFInterpolBkg[3]=Spline2:KDEtype=Gauss:KDEiter=Nonadaptive:KDEborder=None:NAvEvtPerBin=50" ); 

   // Test the multi-dimensional probability density estimator
   // here are the options strings for the MinMax and RMS methods, respectively:
   //      "!H:!V:VolumeRangeMode=MinMax:DeltaFrac=0.2:KernelEstimator=Gauss:GaussSigma=0.3" );
   //      "!H:!V:VolumeRangeMode=RMS:DeltaFrac=3:KernelEstimator=Gauss:GaussSigma=0.3" );
   if (Use["PDERS"])
      fac->BookMethod( TMVA::Types::kPDERS, "PDERS",
                           "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" );

   if (Use["PDERSD"])
      fac->BookMethod( TMVA::Types::kPDERS, "PDERSD",
                           "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=Decorrelate" );

   if (Use["PDERSPCA"])
      fac->BookMethod( TMVA::Types::kPDERS, "PDERSPCA",
                           "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=PCA" );

   // Multi-dimensional likelihood estimator using self-adapting phase-space binning
   if (Use["PDEFoam"])
      fac->BookMethod( TMVA::Types::kPDEFoam, "PDEFoam",
                           "!H:!V:SigBgSeparate=F:TailCut=0.001:VolFrac=0.0666:nActiveCells=500:nSampl=2000:nBin=5:Nmin=100:Kernel=None:Compress=T" );

   if (Use["PDEFoamBoost"])
      fac->BookMethod( TMVA::Types::kPDEFoam, "PDEFoamBoost",
                           "!H:!V:Boost_Num=30:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T" );

   // K-Nearest Neighbour classifier (KNN)
   if (Use["KNN"])
      fac->BookMethod( TMVA::Types::kKNN, "KNN",
                           "H:nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" );

   // H-Matrix (chi2-squared) method
   if (Use["HMatrix"])
      fac->BookMethod( TMVA::Types::kHMatrix, "HMatrix", "!H:!V:VarTransform=None" );

   // Linear discriminant (same as Fisher discriminant)
   if (Use["LD"])
      fac->BookMethod( TMVA::Types::kLD, "LD", "H:!V:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10" );

   // Fisher discriminant (same as LD)
   if (Use["Fisher"])
      fac->BookMethod( TMVA::Types::kFisher, "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10" );

   // Fisher with Gauss-transformed input variables
   if (Use["FisherG"])
      fac->BookMethod( TMVA::Types::kFisher, "FisherG", "H:!V:VarTransform=Gauss" );

   // Composite classifier: ensemble (tree) of boosted Fisher classifiers
   if (Use["BoostedFisher"])
      fac->BookMethod( TMVA::Types::kFisher, "BoostedFisher", 
                           "H:!V:Boost_Num=20:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.2:!Boost_DetailedMonitoring" );

   // Function discrimination analysis (FDA) -- test of various fitters - the recommended one is Minuit (or GA or SA)
   if (Use["FDA_MC"])
      fac->BookMethod( TMVA::Types::kFDA, "FDA_MC",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:SampleSize=100000:Sigma=0.1" );

   if (Use["FDA_GA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
      fac->BookMethod( TMVA::Types::kFDA, "FDA_GA",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:PopSize=300:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" );

   if (Use["FDA_SA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
      fac->BookMethod( TMVA::Types::kFDA, "FDA_SA",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=SA:MaxCalls=15000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );

   if (Use["FDA_MT"])
      fac->BookMethod( TMVA::Types::kFDA, "FDA_MT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" );

   if (Use["FDA_GAMT"])
      fac->BookMethod( TMVA::Types::kFDA, "FDA_GAMT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:Cycles=1:PopSize=5:Steps=5:Trim" );

   if (Use["FDA_MCMT"])
      fac->BookMethod( TMVA::Types::kFDA, "FDA_MCMT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:SampleSize=20" );

   // TMVA ANN: MLP (recommended ANN) -- all ANNs in TMVA are Multilayer Perceptrons
   if (Use["MLP"])
      fac->BookMethod( TMVA::Types::kMLP, "MLP",
                      "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator" );

   if (Use["MLPBFGS"])
      fac->BookMethod( TMVA::Types::kMLP, "MLPBFGS",
                      "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:!UseRegulator" );

   if (Use["MLPBNN"])
      fac->BookMethod( TMVA::Types::kMLP, "MLPBNN",
                      "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator" ); // BFGS training with bayesian regulators

   // CF(Clermont-Ferrand)ANN
   if (Use["CFMlpANN"])
      fac->BookMethod( TMVA::Types::kCFMlpANN, "CFMlpANN",
                     "!H:!V:NCycles=2000:HiddenLayers=N+1,N"  ); // n_cycles:#nodes:#nodes:...  

   // Tmlp(Root)ANN
   if (Use["TMlpANN"])
      fac->BookMethod( TMVA::Types::kTMlpANN, "TMlpANN",
                      "!H:!V:NCycles=200:HiddenLayers=N+1,N:LearningMethod=BFGS:ValidationFraction=0.3"  ); // n_cycles:#nodes:#nodes:...

   // Support Vector Machine
   if (Use["SVM"])
      fac->BookMethod( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" );

   // Boosted Decision Trees
   if (Use["BDTG"]) // Gradient Boost
      fac->BookMethod( TMVA::Types::kBDT, "BDTG",Conf::BDTGOpt);
                           //"!H:!V:NTrees=40:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=3" );
      //fac->BookMethod( TMVA::Types::kBDT, "BDTG",
      //                     "!H:!V:NTrees=40:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=3" );

   if (Use["BDT"])  // Adaptive Boost
      fac->BookMethod( TMVA::Types::kBDT, "BDT",
                           "!H:!V:NTrees= 20:MinNodeSize=2.5%:MaxDepth=2:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );

   if (Use["BDTB"]) // Bagging
      fac->BookMethod( TMVA::Types::kBDT, "BDTB",
                           "!H:!V:NTrees=400:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20" );

   if (Use["BDTD"]) // Decorrelation + Adaptive Boost
      fac->BookMethod( TMVA::Types::kBDT, "BDTD",
                           "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate" );

   if (Use["BDTF"])  // Allow Using Fisher discriminant in node splitting for (strong) linearly correlated variables
      fac->BookMethod( TMVA::Types::kBDT, "BDTMitFisher",
                           "!H:!V:NTrees=50:MinNodeSize=2.5%:UseFisherCuts:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:SeparationType=GiniIndex:nCuts=20" );

   // RuleFit -- TMVA implementation of Friedman's method
   if (Use["RuleFit"])
      fac->BookMethod( TMVA::Types::kRuleFit, "RuleFit",
                           "H:!V:RuleFitModule=RFTMVA:Model=ModRuleLinear:MinImp=0.001:RuleMinDist=0.001:NTrees=20:fEventsMin=0.01:fEventsMax=0.5:GDTau=-1.0:GDTauPrec=0.01:GDStep=0.01:GDNSteps=10000:GDErrScale=1.02" );
}
  return true;
}

void DMP_BGO_Classification() // spilt by ,
{
        /*
   // The explicit loading of the shared libTMVA is done in TMVAlogon.C, defined in .rootrc
   // if you use your private .rootrc, or run from a different directory, please copy the
   // corresponding lines from .rootrc

   // methods to be processed can be given as an argument; use format:
   //
   // mylinux~> root -l TMVAClassification.C\(\"myMethod1,myMethod2,myMethod3\"\)
   //
   // if you like to use a method via the plugin mechanism, we recommend using
   //
   // mylinux~> root -l TMVAClassification.C\(\"P_myMethod\"\)
   // (an example is given for using the BDT as plugin (see below),
   // but of course the real application is when you write your own
   // method based)
   */

   std::cout << "\n\n==> Start TMVAClassification:"<<endl;

   std::cout<<"\n\tinput signal file:"<<std::endl;
   TChain  *signal = new TChain(__treeName);
   for(size_t i=0;i<Conf::signalFiles.size();++i){
     signal->AddFile(Conf::signalFiles.at(i));
     std::cout<<"\t\t"<<Conf::signalFiles.at(i)<<std::endl;
   }

   std::cout<<"\n\tinput background file:"<<std::endl;
   TChain *background = new TChain(__treeName);
   for(size_t i=0;i<Conf::backgroundFiles.size();++i){
     background->AddFile(Conf::backgroundFiles.at(i));
     std::cout<<"\t\t"<<Conf::backgroundFiles.at(i)<<std::endl;
   }
   //TFile    *input_BF = TFile::Open(BFName);
   //if(input_BF == 0){
   //  return;
   //}
   //TTree *background = (TTree*)input_BF->Get(__treeName);
   // --------------------------------------------------------------------------------------------------
   // --- Here the preparation phase begins
   // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
   Conf::outputFileName = Conf::myMethodList+Conf::outputFileName;
   TFile* outputFile = TFile::Open( Conf::outputFileName, "RECREATE" );
   std::cout<<"\n\t==> output file:\t"<<Conf::outputFileName<<std::endl;

// *
// *  TODO:
// *
   // This loads the library
   //TMVA::Tools::Instance();   // by me


   // ------------------------------------------------------------------------------------------------
   // Create the factory object. Later you can choose the methods
   // whose performance you'd like to investigate. The factory is 
   // the only TMVA object you have to interact with
   //
   // The first argument is the base of the name of all the
   // weightfiles in the directory weight/
   //
   // The second argument is the output file for the training results
   // All TMVA output can be suppressed by removing the "!" (not) in
   // front of the "Silent" argument in the option string
   TMVA::Factory *factory =  new TMVA::Factory( "TMVAClassification", outputFile,
                    "V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification");

   // Define the input variables that shall be used for the MVA training
   // note that you may also use variable expressions, such as: "3*var1/var2*abs(var3)"
   // [all types of expressions that can also be parsed by TTree::Draw( "expression" )]

   factory->AddVariable( "Bgo.GetTotalRMS()","TotalRMS","",'F');
   //factory->AddVariable( "Bgo.fFValue[8]","F[8]","",'F');
   //factory->AddVariable( "Bgo.fFValue[9]","F[9]","",'F');
   //factory->AddVariable( "Bgo.fFValue[10]","F[10]","",'F');
   //factory->AddVariable( "Bgo.fFValue[11]","F[11]","",'F');
   //factory->AddVariable( "Bgo.fFValue[12]","F[12]","",'F');
   //factory->AddVariable( "Bgo.fFValue[13]","F[13]","",'F');
   //factory->AddVariable( "Bgo.GetEMax_ETail()","EMax/ETail","",'F');
   //factory->AddVariable( "Bgo.GetEMax_ETail() / Bgo.GetTMax()","EMax/ETail/TMax","",'F');

   // ===========>  value at tail. E ratio at tail is the best
   factory->AddVariable( "Bgo.GetERatioAtTail()","ER at tail","",'F');  // better than F
   factory->AddVariable( "Bgo.GetERatioOfEMaxLayer()","ER of TMax","",'F');
   factory->AddVariable( "Bgo.GetTMax()","TMax","",'F');
   factory->AddVariable( "Bgo.GetLayerIDOfCoG()","CoG_Z","",'F');
   factory->AddVariable( "Bgo.GetERL3_F2L()","ERL3_F2L","",'F');
   //factory->AddVariable( "Bgo.GetERL3_First2CoG()","ERL3_first2CoG","",'F');

   //factory->AddVariable( "Bgo.GetERMin(2)","ERT2","",'F'); // better than F
   //factory->AddVariable( "Bgo.GetFAtTail()","F at tail","",'F');  // better than G
   //factory->AddVariable( "Bgo.GetGAtTail()","G at tail","",'F');

   //factory->AddVariable( "Bgo.fLRMS","LRMS","",'F');
   //factory->AddVariable( "Bgo.GetFiredBarNumber()","FiredBarNumber","",'I');
   //factory->AddVariable( "Bgo.GetPileupRatio()","PileupRatio","",'F');
   //factory->AddVariable( "Bgo.GetWindowERatio()","WindowERatio","",'F');
   //factory->AddVariable( "Bgo.GetRMSOfCoG()","RMSOfCoG","",'F');
   factory->AddVariable( "Bgo.GetRMSOfEMaxLayer()","RMSOfEMaxLayer","",'F');
   factory->AddVariable( "Bgo.fRMS[0]","RMSOfLayer0","",'F');
   //factory->AddVariable( "Bgo.GetGValueOfCoG()","GOfCoG","",'F');
   //factory->AddVariable( "Bgo.GetGValue(0)","G at layer 0","",'F');
   //factory->AddVariable( "Bgo.GetFractal(1,4)","Fractal","",'F');
   //factory->AddVariable( "Bgo.GetLFractal(1,3)","LFractal","",'F');
   //factory->AddVariable( "Bgo.GetLayerIDOfMaxRMS()","LayerIDOfMaxRMS","",'I');

   // You can add so-called "Spectator variables", which are not used in the MVA training,
   // but will appear in the final "TestTree" produced by TMVA. This TestTree will contain the
   // input variables, the response values of all trained MVAs, and the spectator variables
   //factory->AddSpectator( "Bgo.GetEntryPoint().x()","EntryPointX","mm",'F' );
   //factory->AddSpectator( "Bgo.GetEntryPoint().y()","EntryPointY","mm",'F' );
   //factory->AddSpectator( "Bgo.GetTrackDirection().Theta()","TrackTheta","",'F');
   //factory->AddSpectator( "Bgo.GetMaxEnergyLayerID()","MaxEnergyLayer","",'I');
   //factory->AddSpectator( "Bgo.fTotE","RecoEnergy","MeV",'F' );

   
   // global event weights per tree (see below for setting event-wise weights)
   Double_t signalWeight     = 1.0;
   Double_t backgroundWeight = 1.0;

   // You can add an arbitrary number of signal or background trees
   factory->AddSignalTree    ( signal,     signalWeight     );
   factory->AddBackgroundTree( background, backgroundWeight );

   // To give different trees for training and testing, do as follows:
   //    factory->AddSignalTree( signalTrainingTree, signalTrainWeight, "Training" );
   //    factory->AddSignalTree( signalTestTree,     signalTestWeight,  "Test" );

/*
   // Use the following code instead of the above two or four lines to add signal and background
   // training and test events "by hand"
   // NOTE that in this case one should not give expressions (such as "var1+var2") in the input
   //      variable definition, but simply compute the expression before adding the event
   //
   //     // --- begin ----------------------------------------------------------
   std::vector<Double_t> vars(7); // vector has size of number of input variables
    *  vars[0]:     position x
    *  vars[1]:     position y
    *  vars[2]:     direction xz
    *  vars[3]:     direction yz
    *  vars[4]:     TRMS
    *  vars[5]:     LRMS
    *  vars[6]:     RFRatio
    *  vars[7]:     Fired Bar number
    *  vars[8]:     Pileup Ratio
   Float_t  treevars[4], weight;
   
   // Signal
   for (UInt_t ivar=0; ivar<4; ivar++){
           signal->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
   }
   for (UInt_t i=0; i<signal->GetEntries(); i++) {
      signal->GetEntry(i);
      for (UInt_t ivar=0; ivar<4; ivar++) {
              vars[ivar] = treevars[ivar];
      }
      // add training and test events; here: first half is training, second is testing
      // note that the weight can also be event-wise
      if (i < signal->GetEntries()/2.0) factory->AddSignalTrainingEvent( vars, signalWeight );
      else                              factory->AddSignalTestEvent    ( vars, signalWeight );
   }
   
   // Background (has event weights)
   background->SetBranchAddress( "weight", &weight );
   for (UInt_t ivar=0; ivar<4; ivar++){
           background->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
   }
   for (UInt_t i=0; i<background->GetEntries(); i++) {
      background->GetEntry(i);
      for (UInt_t ivar=0; ivar<4; ivar++) {
              vars[ivar] = treevars[ivar];
      }
      // add training and test events; here: first half is training, second is testing
      // note that the weight can also be event-wise
      if (i < background->GetEntries()/2) factory->AddBackgroundTrainingEvent( vars, backgroundWeight*weight );
      else                                factory->AddBackgroundTestEvent    ( vars, backgroundWeight*weight );
   }
         // --- end ------------------------------------------------------------
   */
   //
   // --- end of tree registration 

   // Set individual event weights (the variables must exist in the original TTree)
   //    for signal    : factory->SetSignalWeightExpression    ("weight1*weight2");
   //    for background: factory->SetBackgroundWeightExpression("weight1*weight2");
// *
// *  TODO: what does the next line used for? 
// *
   //factory->SetBackgroundWeightExpression( "weight" );


   // Tell the factory how to use the training and testing events
   //
   // If no numbers of events are given, half of the events in the tree are used 
   // for training, and the other half for testing:
   //    factory->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );
   // To also specify the number of testing events, use:
   //    factory->PrepareTrainingAndTestTree( mycut,
   //                                         "NSigTrain=3000:NBkgTrain=3000:NSigTest=3000:NBkgTest=3000:SplitMode=Random:!V" );
   factory->PrepareTrainingAndTestTree( Conf::T0 && Conf::SignalCut, Conf::T0 && Conf::BackgroundCut,
                                        "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );

   //-------------------------------------------------------------------
   // Book Methods. Must after all setting
   if( ! BookMethod(factory) ) return;


   // For an example of the category classifier usage, see: TMVAClassificationCategory
   // --------------------------------------------------------------------------------------------------
   // ---- Now you can optimize the setting (configuration) of the MVAs using the set of training events
   // ---- STILL EXPERIMENTAL and only implemented for BDT's ! 
   // factory->OptimizeAllMethods("SigEffAt001","Scan");
   // factory->OptimizeAllMethods("ROCIntegral","FitGA");

   // --------------------------------------------------------------------------------------------------
   // ---- Now you can tell the factory to train, test, and evaluate the MVAs
   // Train MVAs using the set of training events
   factory->TrainAllMethods();

   // ---- Evaluate all MVAs using the set of test events
   factory->TestAllMethods();

   // ----- Evaluate and compare performance of all configured MVAs
   factory->EvaluateAllMethods();

   // --------------------------------------------------------------
   // Save the output
   outputFile->Close();

   std::cout << "\n==> Wrote root file: " << Conf::outputFileName << std::endl;
   Conf::PrintInfor();
   delete factory;
   std::cout << "\n==> TMVAClassification is done!" << std::endl;

   if (!gROOT->IsBatch()) TMVAGui( Conf::outputFileName );
}

