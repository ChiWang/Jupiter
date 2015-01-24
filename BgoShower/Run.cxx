
#include <fstream>

void Run()
{
  FileStat_t x;
  TString libName0 ="libBgoShowerCreator";
  if(gSystem->GetPathInfo(libName0,x)){
    gSystem->Load("$DMPSWSYS/lib/libDmpBase.so");
    gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
    gSystem->Load("$DMPSWWORK/lib/libDmpEvtBgoShower.so");
    gInterpreter->AddIncludePath("$DMPSWSYS/include");
    gInterpreter->AddIncludePath("$DMPSWWORK/include");
    gSystem->CompileMacro("./BgoShowerCreator.cxx","k",libName0);
  }else{
    gSystem->Load(libName0);
  }

  TString libName1 ="libMyPlot";
  if(gSystem->GetPathInfo(libName1,x)){
    //gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
    gSystem->Load("$DMPSWWORK/lib/libDmpEvtBgoShower.so");
    //gInterpreter->AddIncludePath("$DMPSWSYS/include");
    gInterpreter->AddIncludePath("$DMPSWWORK/include");
    gSystem->CompileMacro("./MyPlots.cxx","k",libName1);
  }else{
    gSystem->Load(libName1);
  }
}

void MyTest()
{
  DAMPE::Bgo::Conf::ouputPath = "./testData/";
  //DAMPE::Bgo::Conf::MaxEvents = 2;
  DAMPE::Bgo::BgoShowerCreator();
  //DAMPE::Bgo::BgoShowerCreator("A2Data00_20141111_022025_Hits.root electron M300 P93_93_0");
}

void TransforAll(TString filename = "input.list")
{
  ifstream in_log("./Output/"+filename);
  char tmp[256];
  bool beg = false;
  DAMPE::Bgo::Conf::ouputPath = "./Output/";

  //DAMPE::Bgo::Conf::MaxEvents = 20;
  while(!in_log.eof()){
    in_log.getline(tmp,256);
    TString aLine = tmp;
    if(aLine == "END"){
      break;
    }else if(aLine.Contains("Begin")){
      beg = true;
    }else if(aLine.Contains("Skip") || aLine.Contains("calibration") || aLine.Contains("pedestal") || aLine.Contains("Setup")){
      continue;
    }else{
      if(beg && aLine.Contains("root")){
        DAMPE::Bgo::BgoShowerCreator(aLine);
      }
    }
  }
  in_log.close();
}

