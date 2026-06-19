/************************************************************/
/*    NAME: kosCharisis                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
#include "GenPath.h"
#include "MBUtils.h"
#include "ACTable.h"
#include <cmath>
#include <iostream>

using namespace std;

//---------------------------------------------------------
// Constructor()

GenPath::GenPath()
{
  m_nav_x = 0; m_nav_y = 0;
  m_received_first = false;
  m_received_last = false;
  m_path_generated = false;
}


//---------------------------------------------------------
// Procedure: OnNewMail()

bool GenPath::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
  for(auto const& msg : NewMail) {
    string key = msg.GetKey();
    if(key == "VISIT_POINT") {
      string sval = msg.GetString();
      if(sval == "firstpoint") {
        m_received_points.clear();
        m_received_first = true;
      } else if(sval == "lastpoint") {
        m_received_last = true;
      } else {
        // Parse coordinate string: "x=8, y=9, id=1" [1]
        double x = stod(tokStringParse(sval, "x", ',', '='));
        double y = stod(tokStringParse(sval, "y", ',', '='));
        string id = tokStringParse(sval, "id", ',', '=');
        XYPoint new_pt(x, y);
        new_pt.set_label(id);
        m_received_points.push_back(new_pt);
      }
    } else if(key == "NAV_X") m_nav_x = msg.GetDouble();
      else if(key == "NAV_Y") m_nav_y = msg.GetDouble();
  }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool GenPath::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool GenPath::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Trigger path generation once all points are in [2, 3]
  if(m_received_first && m_received_last && !m_path_generated) {
    generatePath();
    m_path_generated = true;
  }
  AppCastingMOOSApp::PostReport();
  return(true);
}


void GenPath::generatePath() {
  XYSegList my_tour;
  double cur_x = m_nav_x;
  double cur_y = m_nav_y;

  while(!m_received_points.empty()) {
    int best_index = -1;
    double min_dist = -1;

    for(size_t i=0; i<m_received_points.size(); ++i) {
      double d = hypot(cur_x - m_received_points[i].x(), cur_y - m_received_points[i].y());
      if(best_index == -1 || d < min_dist) {
        min_dist = d;
        best_index = i;
      }
    }

    if(best_index != -1) {
      cur_x = m_received_points[best_index].x();
      cur_y = m_received_points[best_index].y();
      my_tour.add_vertex(cur_x, cur_y);
      m_received_points.erase(m_received_points.begin() + best_index);
    }
  }
  // Send the update to the Waypoint behavior [3, 4]
  Notify("WAYPOINT_UPDATE", "points = " + my_tour.get_spec());
}



//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool GenPath::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  
   //m_MissionReader.EnableVerbatimQuoting(false);
  
  //if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
  //  reportConfigWarning("No config block found for " + GetAppName());

  //STRING_LIST::iterator p;
  //for(p=sParams.begin(); p!=sParams.end(); p++) {
  //  string orig  = *p;
  //  string line  = *p;
  //  string param = tolower(biteStringX(line, '='));
  //  string value = line;

  //  bool handled = false;
  //  if(param == "foo") {
  //    handled = true;
  //  }
  //  else if(param == "bar") {
  //    handled = true;
  //  }

m_MissionReader.GetConfiguration(GetAppName(), sParams);

  for(string line : sParams) {
    string orig  = line;
    string param = toupper(MOOSChomp(line, "="));
    string value = line;

    bool handled = false;
    if(param == "VISIT_RADIUS") {
      m_visit_radius = atof(value.c_str());
      handled = true;
    }
    // Add other parameters here (e.g., vname if applicable)


    if(!handled)
      reportUnhandledConfigWarning(orig);

  }

  registerVariables();
  
  return(true);	
}

//---------------------------------------------------------
// Procedure: registerVariables()

void GenPath::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("VISIT_POINT", 0);
  Register("NAV_X", 0);
  Register("NAV_Y", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool GenPath::buildReport() 
{
  m_msgs << "Total Points Received: " << m_received_points.size() << endl;
  m_msgs << "Path Generated: " << boolToString(m_path_generated) << endl;
  //m_msgs << "mytourgetspec: " << mytour.get_spec() << endl;
  return(true);
}




