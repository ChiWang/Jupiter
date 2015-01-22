/*
 *  $Id: DataQuality.C, 2015-01-01 16:59:38 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 30/11/2014
*/

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include "TTree.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TH2F.h"
#include "TF1.h"
#include "TCanvas.h"

#include "AMSSetup.h"

#ifndef AMS_Alignment_C
#define AMS_Alignment_C

namespace AMS{
using namespace Common;
// *
// *  TODO: ios has not be declared
// *
  TF1 *gausFit = new TF1("GF","gaus",0,150);
  TF1 *linearFit = new TF1("LF","pol1",-10,1200);
  void Alignment(){    // for each ladder
    TString outFilename = "alignment_"+_file;
    outFilename.Remove(outFilename.Length()-4);
    outFilename +="txt";
    cout<<_file<<"\t\t"<<outFilename<<endl;

    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(100111);

    TH1F *h_align_CoG[NLadder][3] = {{0}};  // cluster numbers, 0: s-side, 1: k-side-sensor0, 2: k-side-sensor-1
    h_align_CoG[0][0] = new TH1F("L0_S0 Reference Position","L0_S0 Reference Position",1500,0,10);
    h_align_CoG[0][1] = new TH1F("L0_S1_0 Reference Position","L0_S1 Reference Position",1500,0,10);
    h_align_CoG[0][2] = new TH1F("L0_S1_1 Reference Position","L0_S1 Reference Position",1500,0,10);
    h_align_CoG[0][0]->SetXTitle("X / cm");
    h_align_CoG[0][1]->SetXTitle("Y / cm");
    for(short i =1;i<NLadder; ++i){
      h_align_CoG[i][0] = new TH1F(Form("L%d_S0 Offset",i),Form("L%d_S0 Offset",i),1000,-2.5,1.5);
      h_align_CoG[i][1] = new TH1F(Form("L%d_S1_0 Offset",i),Form("L%d_S0 Offset",i),1000,-2.5,1.5);
      h_align_CoG[i][2] = new TH1F(Form("L%d_S1_1 Offset",i),Form("L%d_S0 Offset",i),1000,-2.5,1.5);
    }

    TH2F *h_offset_SeedAdd[NLadder-1][3] = {{0}};  // cluster numbers, 0: s-side, 1: k-side-sensor0, 2: k-side-sensor-1
    for(short i =0;i<NLadder-1; ++i){
      h_offset_SeedAdd[i][0] = new TH2F(Form("L%d_S0 Offset_SeedAdd",i+1),  Form("L%d_S0 Offset_SeedAdd",i+1),640,0,640,1000,-2.5,1.5);
      h_offset_SeedAdd[i][1] = new TH2F(Form("L%d_S1_0 Offset_SeedAdd",i+1),Form("L%d_S1 Offset_SeedAdd",i+1),384,640,1024,1000,-2.5,1.5);
      h_offset_SeedAdd[i][2] = new TH2F(Form("L%d_S1_1 Offset_SeedAdd",i+1),Form("L%d_S1 Offset_SeedAdd",i+1),384,640,1024,1000,-2.5,1.5);
    }

    for(_evtID =0;_evtID<_entries;++_evtID){
      _tree->GetEntry(_evtID);
      // one track event
      if(! N_ClustersInLadder_I(1,0)){   // both sides
        continue;
      }
      if(! ClusterNumberLessThan2_forAllS_Side()){
        continue;
      }
      // update reference
      float Posi_Ref_ladder0[2]={0.0,0.};   // ladder 0, side 0, 1
      short k_Ref_SensorID = 0;       // for k-side, short(long) ladder has 2(6) group, one group contains 2 silicon sensors. While alignmenting, we'd use vertical tracks (means, clusters of ladder 1~4 must in the same sensor(0 or 1) as the cluster which belongs to ladder 0)
      int n_cls = _evt_AMS->fAMSCls->GetEntriesFast();
      for(short ic=0;ic<n_cls;++ic){
        Cluster *aCluster = dynamic_cast<Cluster*>(_evt_AMS->fAMSCls->At(ic));
        if(aCluster->ladder == 0){
          short side = aCluster->side;
          Posi_Ref_ladder0[side] = GetPosition(aCluster,false);
          if(side == 1 && aCluster->GetSeedAdd()>(640+192)){  // 640: number of s-side readout strips(one sensor), 192: number of k-side readout strips (one sensor)
            k_Ref_SensorID = 1;
          }
          h_align_CoG[0][side+(side ==1 ? k_Ref_SensorID : 0)]->Fill(Posi_Ref_ladder0[side]);
        }
      }
      // alignment
      for(short ic=0;ic<n_cls;++ic){
        Cluster *aCluster = dynamic_cast<Cluster*>(_evt_AMS->fAMSCls->At(ic));
        short order = aCluster->ladder;
        if(order != 0){
          short side = aCluster->side;
          float offV = GetPosition(aCluster,false) - Posi_Ref_ladder0[side];
          if(side == 1){
            short k_SensorID = (aCluster->GetSeedAdd()<(640+192)) ? 0 : 1;
            if(k_SensorID == k_Ref_SensorID){
              h_align_CoG[order][side + k_SensorID]->Fill(offV);
              h_offset_SeedAdd[order-1][side+k_SensorID]->Fill(aCluster->GetSeedAdd(),offV);
            }
          }else{
            h_align_CoG[order][side]->Fill(offV);
            h_offset_SeedAdd[order-1][side]->Fill(aCluster->GetSeedAdd(),offV);
          }
        }
      }
    }

    TCanvas *c0 = new TCanvas(_file+"  Alignment Ref.",_file+"  Alignment Ref.");
    c0->Divide(1,2);
    TCanvas *c1 = new TCanvas(_file+"  Alignment",_file+"  Alignment");
    c1->Divide(2,4,0.,0.0);
    for(short s=0;s<2;++s){
      c0->cd(s+1);
      gPad->SetLogy();
      h_align_CoG[0][s]->SetLabelSize(0.06);
      h_align_CoG[0][s]->SetLabelSize(0.04,"Y");
      h_align_CoG[0][s]->SetTitleSize(0.05,"X");
      h_align_CoG[0][s]->Draw();
      float mean = h_align_CoG[0][s]->GetMean(), rms = h_align_CoG[0][s]->GetRMS();
      gausFit->SetRange(mean-rms,mean+rms);
      h_align_CoG[0][s]->Fit(gausFit,"R0Q");
      _offset[ExHall][0][s]=gausFit->GetParameter(1);
      gausFit->DrawCopy("lsame");
      if(s ==1){
        h_align_CoG[0][2]->SetLineColor(3);
        h_align_CoG[0][2]->Draw("same");
      }
    }
    _offset[ExHall][0][ZAxis]=0;

    for(short id=1;id<NLadder;++id){
      for(short s=0;s<2;++s){
        c1->cd((id-1)*2+s+1);
        gPad->SetLogy();
        if(s==0){
          h_align_CoG[id][s]->SetXTitle("X / cm");
        }else{
          h_align_CoG[id][s]->SetXTitle("Y / cm");
        }
        h_align_CoG[id][s]->SetLabelSize(0.12);
        h_align_CoG[id][s]->SetLabelSize(0.08,"Y");
        h_align_CoG[id][s]->SetTitleSize(0.04,"X");
        h_align_CoG[id][s]->Draw();
        float mean = h_align_CoG[id][s]->GetMean(), rms = h_align_CoG[id][s]->GetRMS();
        gausFit->SetRange(mean-rms,mean+rms);
        h_align_CoG[id][s]->Fit(gausFit,"R0Q");
        _offset[ExHall][id][s]=gausFit->GetParameter(1);
        gausFit->DrawCopy("lsame");
        //output<<
      }
      _offset[ExHall][id][ZAxis] = _offset[ExHall][id-1][ZAxis]+Conf::LadderDistance[ExHall][id-1];
      c1->cd((id-1)*2+2);
      h_align_CoG[id][2]->SetLineColor(3);
      h_align_CoG[id][2]->Draw("same");
    }
    ofstream outAlignment(PATH::_alignment+outFilename);
    outAlignment<<"ladder id\tx(s_side) offset mean\t\ty(k_side) offset mean\t\t z position\t\t Location: "<<ExHall<<endl;
    for(int id=0;id<NLadder;++id){
      outAlignment<<id;
      for(int ax=0;ax<3;++ax){
        outAlignment<<"\t\t"<<_offset[ExHall][id][ax];
      }
      outAlignment<<endl;
    }
    outAlignment.close();
    TCanvas *c2 = new TCanvas(_file+"  Offset_SeedAdd",_file+"  Offset_SeedAdd");
    c2->Divide(2,4,0.,0.0);
    for(short id=0;id<NLadder-1;++id){
      for(short s=0;s<2;++s){
        c2->cd(id*2+s+1);
        gStyle->SetOptStat(11111111);
        gStyle->SetOptFit(111111111);
        h_offset_SeedAdd[id][s]->SetXTitle("Seed ID");
        h_offset_SeedAdd[id][s]->SetYTitle("Offset / cm");
        h_offset_SeedAdd[id][s]->SetLabelSize(0.12);
        h_offset_SeedAdd[id][s]->SetLabelSize(0.08,"Y");
        //h_offset_SeedAdd[id][s]->SetTitleSize(0.04,"Y");
        h_offset_SeedAdd[id][s]->Draw("colz");
        //h_offset_SeedAdd[id][s]->ProfileX()->Fit(linearFit,"0Q");//QF
        //linearFit->DrawCopy("same");
        //output<<
      }
      c2->cd(id*2+2);
      h_offset_SeedAdd[id][2]->Draw("same");
    }
    //if(reload) UpdateOffset(outFilename); 
  }
};

#endif


