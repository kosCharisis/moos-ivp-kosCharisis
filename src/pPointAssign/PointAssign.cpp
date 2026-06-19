/************************************************************/
/*    NAME: Chris Jones                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "PointAssign.h"
#include "XYPoint.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

PointAssign::PointAssign()
{
  m_assign_by_region=true; m_vindex=0;
}

//---------------------------------------------------------
// Destructor

PointAssign::~PointAssign()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool PointAssign::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  for (auto const& msg : NewMail) {
    if (msg.GetKey() == "VISIT_POINT") {
      string sval = msg.GetString();
      
      // Handle the "bookends" [1]
      if (sval == "firstpoint" || sval == "lastpoint") {
        for (string vname : m_vehicles)
          Notify("VISIT_POINT_" + toupper(vname), sval);
        continue;
      }

      // Parse coordinates [1]
      double x = stod(tokStringParse(sval, "x", ',', '='));
      double y = stod(tokStringParse(sval, "y", ',', '='));
      string id = tokStringParse(sval, "id", ',', '=');

      string target_vehicle;
      if (m_assign_by_region) {
        // Region split: x < 87.5 is West, x >= 87.5 is East [1]
        target_vehicle = (x < 87.5) ? m_vehicles[0] : m_vehicles[1];
      } else {
        // Alternating assignment [1]
        target_vehicle = m_vehicles[m_vindex];
        m_vindex = (m_vindex + 1) % m_vehicles.size();
      }

      // Publish to specific vehicle and visualize [3, 4]
      Notify("VISIT_POINT_" + toupper(target_vehicle), sval);
      string color = (target_vehicle == "henry") ? "yellow" : "red";
      postViewPoint(x, y, id, color);
    }
  }
  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool PointAssign::OnConnectToServer()
{
   registerVariables();
   Notify("POINT_PAUSE", "false");
   //m_Comms.Notify("POINT_PAUSE", "false"); //later
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool PointAssign::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: postViewpoint()

void PointAssign::postViewPoint(double x, double y, string label, string color) 
{
  XYPoint point(x, y);
  point.set_label(label);
  point.set_color("vertex", color);
  point.set_param("vertex_size", "4");

  string spec = point.get_spec();    // gets the string representation of a point
  Notify("VIEW_POINT", spec);
}





//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool PointAssign::OnStartUp() 
{
  AppCastingMOOSApp::OnStartUp();
  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
  for (string line : sParams) {
    string param = tolower(MOOSChomp(line, "="));
    string value = line;

    if (param == "vname")
      m_vehicles.push_back(value); // Add multiple vehicles [1]
    else if (param == "assign_by_region")
      m_assign_by_region = (tolower(value) == "true");
  }
  RegisterVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables()

void PointAssign::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("VISIT_POINT", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool PointAssign::buildReport() {
  m_msgs << "Assignment Policy: " << (m_assign_by_region ? "Region" : "Alternating") << endl;
  m_msgs << "Vehicles Managed: " << m_vehicles.size() << endl;
  return true;
}




