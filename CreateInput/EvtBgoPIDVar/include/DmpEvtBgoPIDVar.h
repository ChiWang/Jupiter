/*   $Id: DmpEvtBgoPIDVar.h, 2015-01-09 14:59:42 DAMPE $
 *--------------------------------------------------------
 *  Author(s):
 *      Chi WANG (chiwang@mail.ustc.edu.cn)
 *--------------------------------------------------------
*/

#ifndef DmpEvtBgoPIDVar_H
#define DmpEvtBgoPIDVar_H

#define BGO_LayerNO 14
#define BGO_BarNO 22
#define BGO_DyNO 3

#include <map>
#include "TVector3.h"

class DmpEvtBgoPIDVar : public TObject{
/*
 *  DmpEvtBgoPIDVar
 *
 */
public:
  typedef std::map<int,double>  m_EnergiesInLayer;     // key: bar ID(0~21)
  DmpEvtBgoPIDVar();
  ~DmpEvtBgoPIDVar();

  void Reset();
  void LoadFrom(DmpEvtBgoPIDVar *r);

public:
  /*
   *  Define your data members at here
   *
   */
  std::map<int,m_EnergiesInLayer >   fE_LB;
  double        fRMS2[BGO_LayerNO];
  double        fFValue[BGO_LayerNO];
  double        fTotalE;            // MeV
  TVector3      fPre_Direction;     // provide from other sub-det

  ClassDef(DmpEvtBgoPIDVar,1)

};

#endif
