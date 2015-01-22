/*
 *  $Id: BgoShowerCreator.cxx, 2015-01-17 18:43:11 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 15/01/2015
*/

//#include <time.h>

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <math.h>
#include <fstream>

#include "TFile.h"
#include "TTree.h"

#include "DmpEvtBgoHits.h"
#include "DmpEvtBgoShower.h"
#include "DmpBgoBase.h"

#ifndef BgoShowerCreator_CXX
#define BgoShowerCreator_CXX

namespace DAMPE{
namespace Bgo{

  namespace Conf{
    TString  inputPath = "./Input/";
    TString  ouputPath = "./Output/";
    TString  inputTree = "/Event/Cal";
    TString  inputBranch = "Hits";
    long MaxEvents = 999999999;
  };

  TString GetInputFileName(TString inLine = "A2Data00_20141105_190544_Hits.root"){
    TObjArray *opt = inLine.Tokenize(" ");
    return ((TObjString*)opt->At(0))->GetString();
  }

  TString GetOutFileName(TString inLine = "A2Data00_20141105_190544_Hits.root",TString hh="BS"){
    TObjArray *opt = inLine.Tokenize(" ");
    TString out = ((TObjString*)opt->At(0))->GetString();
    out.Remove(0,out.First("_")+1);
    TString tag ="";
    for(int i=1;i<opt->GetEntriesFast();++i){
      tag += ((TObjString*)opt->At(i))->GetString()+"-";
    }
    return hh+"_"+tag+out;
  }

  long BgoShowerCreator(TString file_Rec0 = "A2Data00_20141105_190544_Hits.root testData"){
    TString inname = GetInputFileName(file_Rec0);
    TFile *input_f = TFile::Open(Conf::inputPath+inname);
    if(input_f == 0){
      return -1;
    }
    TTree *tree_i = (TTree*)(input_f->Get(Conf::inputTree));
    DmpEvtBgoHits *event_bgo = new DmpEvtBgoHits();
    tree_i->SetBranchAddress(Conf::inputBranch,&event_bgo);

    TString outName = GetOutFileName(file_Rec0,"BS");
    outName.Remove(outName.Last('_'));
    TFile *output_f = new TFile(Conf::ouputPath+outName+".root","RECREATE");
    output_f->mkdir("Event");
    TTree *tree_o = new TTree("Rec0","Rec0");
    DmpEvtBgoShower *evt_BgoShower = new DmpEvtBgoShower(); 
    tree_o->Branch("Bgo",evt_BgoShower->GetName(),&evt_BgoShower);

    //clock_t s0=clock();
    //std::cout<<"T0 = DEBUG: "<<__FILE__<<"("<<__LINE__<<")\t\t"<<s0<<std::endl;
    long et = tree_i->GetEntries();
    cout<<"\nProcessing "<<input_f->GetName()<<"\tevents = "<<et<<std::endl;
    for(long ievt = 0;ievt<et && ievt < Conf::MaxEvents;++ievt){
      //if(ievt % 500 == 0) std::cout<<" ..."<<ievt<<std::endl;
      tree_i->GetEntry(ievt);
      std::map<int,std::vector<DmpBgoFiredBar*> > firedBar; // key: layer ID
      short nBar = event_bgo->fGlobalBarID.size();
      for(short ib =0;ib<nBar;++ib){
        short gid = event_bgo->fGlobalBarID[ib];
        int lid = DmpBgoBase::GetLayerID(gid);
        int bid =DmpBgoBase::GetBarID(gid);
        firedBar[lid].push_back( new DmpBgoFiredBar(lid,bid,event_bgo->fEnergy[ib],event_bgo->fES0[ib],event_bgo->fES1[ib],event_bgo->fPosition[ib]) );
      }

      for(std::map<int, std::vector<DmpBgoFiredBar*> >::iterator it=firedBar.begin();it!=firedBar.end();++it){ // cluster finding
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//std::cout<<"\nDEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
//        for(int i=0;i<it->second.size();++i){
//          it->second.at(i)->MyPrint();
//        }
//-------------------------------------------------------------------
//-------------------------------------------------------------------
            // sort firedBar by fE
        std::vector<double> eInThisLayer;   // tmp, will delete it
        for(unsigned int i = 0;i<it->second.size();++i){
          eInThisLayer.push_back(it->second.at(i)->fE);
        }
        std::vector<DmpBgoFiredBar*>    barInThisLayer_sortByE;
        for(unsigned int i=0;i<eInThisLayer.size();++i){
          int loc = (int)(std::max_element(eInThisLayer.begin(),eInThisLayer.end()) - eInThisLayer.begin());
          barInThisLayer_sortByE.push_back(it->second.at(loc));
          eInThisLayer[loc] = -99;      // must set to a very low value
        }

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
//        for(int i=0;i<barInThisLayer_sortByE.size();++i){
//          barInThisLayer_sortByE[i]->MyPrint();
//        }
//-------------------------------------------------------------------
//-------------------------------------------------------------------
            // cluster finding
            //  step 0:     create the first cluster by the bar which has max fE
        std::vector<DmpEvtBgoCluster*>  clusterInThisLayer;
        DmpEvtBgoCluster *clus0 = evt_BgoShower->AddNewCluster(barInThisLayer_sortByE[0]);
        clusterInThisLayer.push_back(clus0);
            //  step 1:     classify other bars
        for(unsigned int ib=1;ib<barInThisLayer_sortByE.size();++ib){
          //barInThisLayer_sortByE[ib]->MyPrint();
          int clusterIndex = 0;
          int min_dis = BGO_BarNO;
          int thisBID = barInThisLayer_sortByE[ib]->fBar;
          for(unsigned int ic=0;ic<clusterInThisLayer.size();++ic){  // find nearest cluster
            int dis = thisBID - clusterInThisLayer[ic]->fSeedBarID; //????
            if(abs(dis) < min_dis){
              clusterIndex = ic;
              min_dis = dis;
            }
          }
          int refB_ID = thisBID + ((min_dis > 0) ? -1 : +1);   // the bar closest to bar_ic and near seed bar
          DmpBgoFiredBar *refB = 0;
          for(unsigned int ref_i=0;ref_i<barInThisLayer_sortByE.size();++ref_i){
            if(barInThisLayer_sortByE[ref_i]->fBar == refB_ID){
              refB = barInThisLayer_sortByE[ref_i];
              break;
            }
          }
          bool aNewSeed = false;    // is a new cluster's seed
          if(refB){
            if((barInThisLayer_sortByE[ib]->fE - refB->fE) > 23){
              aNewSeed = true;
            }
          }else{
            if(barInThisLayer_sortByE[ib]->fE > 23){
              aNewSeed = true;
            }
          }
          if(aNewSeed){
            DmpEvtBgoCluster *aNewClu = evt_BgoShower->AddNewCluster(barInThisLayer_sortByE[ib]);
            clusterInThisLayer.push_back(aNewClu);
          }else{
            clusterInThisLayer[clusterIndex]->AddNewFiredBar(barInThisLayer_sortByE[ib]);
          }
        }
      }

    //clock_t s1=clock();
    //std::cout<<"T1 = DEBUG: "<<__FILE__<<"("<<__LINE__<<")\t\t"<<s1<<std::endl;
      evt_BgoShower->Calculation();
    //clock_t s2=clock();
    //std::cout<<"T2 = DEBUG: "<<__FILE__<<"("<<__LINE__<<")\t\t"<<s2<<std::endl;
      if(evt_BgoShower->fClusters->GetEntries() == 0){
        std::cout<<"event ID = "<<ievt<<"\tfired bar num = 0"<<std::endl;
        //evt_BgoShower->MyPrint();
      }
      tree_o->Fill();
      evt_BgoShower->Reset();
    //clock_t s3=clock();
    //std::cout<<"T3 = DEBUG: "<<__FILE__<<"("<<__LINE__<<")\t\t"<<s3<<std::endl;
    }
    output_f->cd("Event");
    tree_o->Write("",TObject::kOverwrite);
    output_f->Close();

    //clock_t se=clock();
    //std::cout<<"T end = DEBUG: "<<__FILE__<<"("<<__LINE__<<")\t\t"<<se<<std::endl;
    delete event_bgo;
    delete evt_BgoShower;
    delete input_f;
    delete output_f;
    return (et>Conf::MaxEvents)?Conf::MaxEvents:et;
  }

};
};

#endif

