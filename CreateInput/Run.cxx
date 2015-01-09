

void Run(){
  FileStat_t x;
  TString libName ="libMyBgoPID";
  if(gSystem->GetPathInfo(libName,x)){
    gInterpreter->AddIncludePath("$DMPSWSYS/include");
    gInterpreter->AddIncludePath("$DMPSWWORK/include");
    gSystem->Load("$DMPSWSYS/lib/libDmpBase.so");
    gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
    gSystem->Load("$DMPSWWORK/lib/libDmpEvtBgoPIDVar.so");
    gSystem->CompileMacro("./Create_PID_Input_BGO.cxx","k",libName);
  }else{
    gSystem->Load(libName);
  }
}


void MyTest(){
  PID::Bgo::CreateInput();
}
