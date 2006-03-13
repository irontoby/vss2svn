// SSTypes.cpp:
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SSTypes.h"

//---------------------------------------------------------------------------
extern const char* g_szActions[] = { 
  "Labeled",            // = 0
  "Created Project",    // = 1
  "Added Project",      // = 2
  "Added File",         // = 3
  "Destroyed Project",  // = 4
  "Destroyed File",     // = 5
  "Deleted Project",    // = 6
  "Deleted File",       // = 7
  "Recovered Project",  // = 8
  "Recovered File",     // = 9
  "Renamed Project",    // = 10
  "Renamed File",       // = 11
  "Moved Project From", // = 12
  "Moved Project To",   // = 13
  "Shared File",        // = 14
  "Branch File",        // = 15 ???
  "Created File",       // = 16
  "Checked In",         // = 17
  "Action 18",          // missing action 18
  "RollBack",           // = 19
  // missing known actions: branches, archives, restores

  /// --- pseudo actions ---
  "Pinned File",        // = 20
  "Unpinned File"       // = 21
};

const char* CAction::ActionToString (eAction e)
{
  if (e < countof (g_szActions))
    return g_szActions[e];
  return ("unknown");
}

