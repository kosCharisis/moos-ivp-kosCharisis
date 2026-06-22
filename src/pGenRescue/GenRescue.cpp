/************************************************************/
/*    NAME: Mike Benjamin                                   */
/*    ORGN: MIT                                             */
/*    FILE: GenRescue.cpp                                   */
/*    DATE: April 18th, 2022                                */
/************************************************************/

#include <iterator>
#include "GenRescue.h"
#include "MBUtils.h"
#include "ColorParse.h"
#include "XYPoint.h"
#include "XYSegList.h"
#include "GeomUtils.h"
#include "PathUtils.h"
#include "XYFormatUtilsPoly.h"
#include "XYFieldGenerator.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

GenRescue::GenRescue()
{
  m_nav_x = 0;
  m_nav_y = 0;
  m_nav_x_set = false;
  m_nav_y_set = false;
  m_plan_pending = false;
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool GenRescue::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key  = msg.GetKey();
    string sval = msg.GetString();

    bool handled = true;
    if(key == "SWIMMER_ALERT") 
      handled = handleMailNewSwimmer(sval);
    else if(key == "FOUND_SWIMMER") 
      handled = handleMailFoundSwimmer(sval);
    else if(key == "NAV_X") {
      m_nav_x = msg.GetDouble();
      m_nav_x_set = true;
    }
    else if(key == "NAV_Y") {
      m_nav_y = msg.GetDouble();
      m_nav_y_set = true;
    }

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      handled = false;
    
    if(!handled)
      reportRunWarning("Unhandled Mail: " + key +"=" + sval);
    
  }
  return(true);
}
 
//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool GenRescue::OnConnectToServer()
{
  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()

bool GenRescue::Iterate()
{
  AppCastingMOOSApp::Iterate();
  
  //if(m_plan_pending)
  if(m_plan_pending && m_nav_x_set && m_nav_y_set) {
    postShortestPath();
    m_plan_pending = false;
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()

bool GenRescue::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp(); 

  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
  
  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string sLine  = *p;
    string param  = tolower(biteStringX(sLine, '='));
    string value  = sLine;
    if(param == "vname")
      m_vname = value;
  }
  
  RegisterVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables()

void GenRescue::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("SWIMMER_ALERT", 0);
  Register("FOUND_SWIMMER", 0);
  
  Register("NAV_X", 0);
  Register("NAV_Y", 0);
  
}


//---------------------------------------------------------
// Procedure: handleMailNewSwimmer()

bool GenRescue::handleMailNewSwimmer(string str)
{
  string id = tokStringParse(str, "id", ',', '=');
  double x  = tokDoubleParse(str, "x", ',', '=');
  double y  = tokDoubleParse(str, "y", ',', '=');

  if(id == "") {
    reportRunWarning("SWIMMER_ALERT missing id: " + str);
    return(false);
  }

  // Ignore repeated alerts for a swimmer we already know.
  if(m_swimmers.count(id) > 0)
    return(true);

  Swimmer swimmer;
  swimmer.id    = id;
  swimmer.x     = x;
  swimmer.y     = y;
  swimmer.found = false;

  m_swimmers[id] = swimmer;
  m_plan_pending = true;

  reportEvent("New swimmer received: id=" + id +
              ", x=" + doubleToString(x, 1) +
              ", y=" + doubleToString(y, 1));

  return(true);
}


//---------------------------------------------------------
// Procedure: handleMailFoundSwimmer()

bool GenRescue::handleMailFoundSwimmer(string str)
{
  string id = tokStringParse(str, "id", ',', '=');

  if(id == "") {
    reportRunWarning("FOUND_SWIMMER missing id: " + str);
    return(false);
  }

  auto it = m_swimmers.find(id);

  // It is possible to hear that a swimmer was found before
  // receiving that swimmer's alert. Nothing useful to remove yet.
  if(it == m_swimmers.end()) {
    reportEvent("FOUND_SWIMMER received for unknown id=" + id);
    return(true);
  }

  // Ignore duplicate FOUND_SWIMMER messages.
  if(it->second.found)
    return(true);

  it->second.found = true;
  m_plan_pending = true;

  reportEvent("Swimmer removed from route: id=" + id);
  return(true);
}

//---------------------------------------------------------
// Procedure: postShortestPath()

void GenRescue::postShortestPath()
{
  if(!m_nav_x_set || !m_nav_y_set)
    return;

  XYSegList candidate_path;

  for(const auto& entry : m_swimmers) {
    const Swimmer& swimmer = entry.second;

    if(!swimmer.found)
      candidate_path.add_vertex(swimmer.x, swimmer.y);
  }

  // No active swimmers: send an empty/null route.
  if(candidate_path.size() == 0) {
    postNullPath();
    return;
  }

  candidate_path.set_label("rescue_route");

  m_path = greedyPath(candidate_path, m_nav_x, m_nav_y);
  m_path.set_label("rescue_route");

  Notify("VIEW_SEGLIST", m_path.get_spec());

  string update_str = "points = " + m_path.get_spec_pts();

  Notify("SURVEY_UPDATE", update_str);
  reportEvent("SURVEY_UPDATE=" + update_str);
}

//---------------------------------------------------------
// Procedure: postNullPath()
//   Purpose: If a found swimmer represents the last swimmer
//            to be found, then post a survey update essentially
// 

void GenRescue::postNullPath()
{
#if 0
  if(!m_nav_x_set || !m_nav_y_set)
    return;
  if(m_map_pts.size() != 0)
    return;
  
  XYSegList segl;
  segl.add_vertex(m_nav_x, m_nav_y);
  
  // Seglist needs a name, refer when drawging and erasing
  segl.set_label("one");
  Notify("VIEW_SEGLIST", segl.get_spec());

  string update_var = "SURVEY_UPDATE";
  string update_str = "points = " + segl.get_spec_pts();

  Notify(update_var, update_str);
  reportEvent("SURVEY_UPDATE=" + update_str);
#endif
}


//---------------------------------------------------------
// Procedure: buildReport()

bool GenRescue::buildReport()
{
  m_msgs << "Known swimmers: " << m_swimmers.size() << endl;

  for(const auto& entry : m_swimmers) {
    const Swimmer& swimmer = entry.second;

    m_msgs << "  id=" << swimmer.id
           << "  x=" << doubleToString(swimmer.x, 1)
           << "  y=" << doubleToString(swimmer.y, 1)
           << "  found=" << boolToString(swimmer.found)
           << endl;
  }

  unsigned int active_count = 0;

  for(const auto& entry : m_swimmers) {
    if(!entry.second.found)
      active_count++;
  }

  m_msgs << endl;
  m_msgs << "Active swimmers: " << active_count << endl;
  m_msgs << "Plan pending:    " << boolToString(m_plan_pending) << endl;

  
  return(true);
}
