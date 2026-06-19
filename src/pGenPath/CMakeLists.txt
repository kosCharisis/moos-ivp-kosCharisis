#--------------------------------------------------------
# The CMakeLists.txt for:                       pGenPath
# Author(s):                              kosCharisis
#--------------------------------------------------------

SET(SRC
  GenPath.cpp
  GenPath_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pGenPath ${SRC})

TARGET_LINK_LIBRARIES(pGenPath
   ${MOOS_LIBRARIES}
   geometry
   apputil
   mbutil
   m
   pthread)

