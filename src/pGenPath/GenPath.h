/************************************************************/
/*    NAME: kosCharisis                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef GenPath_HEADER
#define GenPath_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include <vector>
#include <string>
#include "XYSegList.h"
#include "XYPoint.h"

class GenPath : public AppCastingMOOSApp
{
 public:
   GenPath();
   ~GenPath() {};

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   //bool OnConnectToServer();
   bool OnStartUp();
   bool buildReport();
   void RegisterVariables();
   bool OnConnectToServer();
   void registerVariables();


 //protected: // Standard AppCastingMOOSApp function to overload 
   //bool buildReport();

protected: // Helper functions
  void generatePath();

// protected:
  // void registerVariables();

 private: // Configuration variables

 private: // State variables
 double m_nav_x;
 double m_nav_y;

 double m_visit_radius;

 std::vector<XYPoint> m_received_points;
 bool m_received_first;
 bool m_received_last;
 bool m_path_generated;
};

#endif 
