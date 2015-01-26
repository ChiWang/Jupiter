
void Run()
{
  FileStat_t x;TString libName = "libBgoClassify";
  if(gSystem->GetPathInfo(libName,x)){
    gSystem->Load("$ROOTSYS/lib/libTMVA.so");
    gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
    gSystem->Load("$DMPSWWORK/lib/libDmpEvtBgoShower.so");
    gInterpreter->AddIncludePath("$DMPSWSYS/include");
    gInterpreter->AddIncludePath("$DMPSWWORK/include");
    gSystem->CompileMacro("./DMP_BGO_Classification.cxx","k",libName);
  }else{
    gSystem->Load(libName);
  }
}

void MyTest(TString useMethods="BDTG,MLP,MLPBFGS",TString sf = "./4GeV_electron.root",TString bf = "./10GeV_Proton.root") //  split by ,
// TMlpANN  worse than MLP
//
{
  DMP_BGO_Classification(useMethods);
}


