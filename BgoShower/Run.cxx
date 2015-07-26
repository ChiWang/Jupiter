
#include <fstream>

void Run()
{
  FileStat_t x;
  TString libName0 ="libBgoShowerCreator";
  if(gSystem->GetPathInfo(libName0,x)){
    gSystem->Load("$DMPSWSYS/lib/libDmpBase.so");
    gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
    gSystem->Load("$DMPSWWORK/lib/libDmpEvtBgoShower.so");
    gSystem->Load("$DMPSWWORK/lib/libDmpEventRaw.so");
    gSystem->Load("$DMPSWWORK/lib/libDmpEvtNudHits.so");
    gSystem->Load("$DMPSWSYS/lib/libDmpEvtSim.so");
    gInterpreter->AddIncludePath("$DMPSWWORK/include");
    gInterpreter->AddIncludePath("$DMPSWSYS/include");
    gSystem->CompileMacro("./BgoShowerCreator.cxx","k",libName0);
  }else{
    gSystem->Load(libName0);
  }

  TString libName1 ="libMyPlot";
  if(gSystem->GetPathInfo(libName1,x)){
    //gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
    gSystem->Load("$DMPSWWORK/lib/libDmpEvtBgoShower.so");
    gInterpreter->AddIncludePath("$DMPSWWORK/include");
    gInterpreter->AddIncludePath("$DMPSWSYS/include");
    gSystem->CompileMacro("./MyPlots.cxx","k",libName1);
  }else{
    gSystem->Load(libName1);
  }
}

void MyTest( TString fn = "A2Data00_20141105_190544_Hits.root testData")
{
  DAMPE::Bgo::Conf::inputPath = "./Input/";
  //DAMPE::Bgo::Conf::outputPath = "./testData/";
  DAMPE::Bgo::Conf::outputPath = "./Output/";
  //DAMPE::Bgo::Conf::MaxEvents = 2;
  DAMPE::Bgo::BgoShowerCreator(fn);
  //DAMPE::Bgo::BgoShowerCreator("A2Data00_20141111_022025_Hits.root electron M300 P93_93_0");
}

void TransforAll(TString filename = "input.list")
{
  ifstream in_log("./Output/"+filename);
  char tmp[256];
  bool beg = false;
  DAMPE::Bgo::Conf::outputPath = "./Output/";

  //DAMPE::Bgo::Conf::MaxEvents = 20;
  while(!in_log.eof()){
    in_log.getline(tmp,256);
    TString aLine = tmp;
    TString file = tmp;
    aLine.ToLower();
    if(aLine == "end"){
      break;
    }else if(aLine == "begin"){
      beg = true;
    }else if(aLine == "skip" || aLine == "calibration" || aLine == "pedestal" || aLine == "setup"){
      continue;
    }else{
      if(beg && file.Contains("root")){
        DAMPE::Bgo::BgoShowerCreator(file);
      }
    }
  }
  in_log.close();
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void MyTestSim(TString fname = "BTSimSPS_proton-400GeV_P43_68-Evts200-sim.root",TString type = "MC",double Resolution = 2.5)
{
// type: MC;    MCTruth
  DAMPE::Bgo::Conf::inputPath = "./InputSim/";
  DAMPE::Bgo::Conf::outputPath = "./OutputSim/";
  //DAMPE::Bgo::Conf::MaxEvents = 2;
  DAMPE::Bgo::BgoShowerCreator(fname,type,Resolution);
  //DAMPE::Bgo::BgoShowerCreator("A2Data00_20141111_022025_Hits.root electron M300 P93_93_0");
}

void TransforAllSim(TString filename = "input.list",TString type="MC")
{
  DAMPE::Bgo::Conf::inputPath = "./InputSim/";
  //DAMPE::Bgo::Conf::inputTree = "/Event/MCTruth";
  ifstream in_log("./InputSim/"+filename);
  char tmp[256];
  bool beg = false;
  DAMPE::Bgo::Conf::outputPath = "./OutputSim/";

  //DAMPE::Bgo::Conf::MaxEvents = 20;
  while(!in_log.eof()){
    in_log.getline(tmp,256);
    TString aLine = tmp;
    TString file = tmp;
    aLine.ToLower();
    if(aLine == "end"){
      break;
    }else if(aLine == "begin"){
      beg = true;
    }else if(aLine == "skip" || aLine == "calibration" || aLine == "pedestal" || aLine == "setup"){
      continue;
    }else{
      if(beg && file.Contains("root")){
        DAMPE::Bgo::BgoShowerCreator(file,type);
      }
    }
  }
  in_log.close();
}

