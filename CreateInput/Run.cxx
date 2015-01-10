
#include <fstream>

void Run(){
  FileStat_t x;
  TString libName ="libMyBgoPID";
  if(gSystem->GetPathInfo(libName,x)){
    gSystem->Load("$DMPSWSYS/lib/libDmpBase.so");
    gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
    gSystem->Load("$DMPSWWORK/lib/libDmpEvtBgoPIDVar.so");
    gInterpreter->AddIncludePath("$DMPSWSYS/include");
    gInterpreter->AddIncludePath("$DMPSWWORK/include");
    gSystem->CompileMacro("./Create_PID_Input_BGO.cxx","k",libName);
  }else{
    gSystem->Load(libName);
  }
}


void MyTest(){
  PID::Bgo::CreateInput();
}

void TransforAll()
{
  ifstream in_log("./Output/Input.log");
  char tmp[256];
  bool beg = false;

  while(!in_log.eof()){
    in_log.getline(tmp,256);
    TString aLine = tmp;
    if(aLine =="END"){
      break;
    }else if(aLine == "Begin"){
      beg = true;
    }else{
      if(beg && (!aLine.Contains("-2"))){
        PID::Bgo::CreateInput(aLine.Remove(40));
      }
    }
  }
  in_log.close();
}



