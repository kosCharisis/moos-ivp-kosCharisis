/************************************************************/
/*    NAME: Chris Jones                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef PointAssign_HEADER
#define PointAssign_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "XYPoint.h"
#include <vector>
#include <string>

class PointAssign : public AppCastingMOOSApp
{
 public:
   PointAssign() ;
   ~PointAssign();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();

protected: // Helper functions
  void postViewPoint(double x, double y, std::string label, std::string color);

 private: // Configuration / State variables
  std::vector<std::string> m_vehicles;
  bool   m_assign_by_region;
  size_t m_vindex; // For alternating assignment
};

#endif 
