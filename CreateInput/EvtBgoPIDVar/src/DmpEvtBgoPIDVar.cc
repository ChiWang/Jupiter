/*   $Id: DmpEvtBgoPIDVar.cc, 2015-01-09 14:59:24+08:00 DAMPE $
 *--------------------------------------------------------
 *  Author(s):
 *
 *--------------------------------------------------------
*/

#include "DmpEvtBgoPIDVar.h"

ClassImp(DmpEvtBgoPIDVar)

DmpEvtBgoPIDVar::DmpEvtBgoPIDVar()
{
  Reset();
}

DmpEvtBgoPIDVar::~DmpEvtBgoPIDVar()
{
}

void DmpEvtBgoPIDVar::Reset()
{
  fE_LB.clear();
  for(int i=0;i<BGO_LayerNO;++i){
    fRMS2[i] = 0;
    fFValue[i] = 0;
  }
  fTotalE = 0;
  fPre_Direction.SetXYZ(0,0,0);
}

void DmpEvtBgoPIDVar::LoadFrom(DmpEvtBgoPIDVar *r)
{
  fE_LB = r->fE_LB;
  for(int i=0;i<BGO_LayerNO;++i){
    fRMS2[i] = r->fRMS2[i];
    fFValue[i] = r->fFValue[i];
  }
  fTotalE = r->fTotalE;
  fPre_Direction = r->fPre_Direction;
}



