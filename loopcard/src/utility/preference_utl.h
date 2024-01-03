#ifndef _PREFERENCE_UTL_H__

#define _PREFERENCE_UTL_H__

#include <Preferences.h> // NVS存储


void savePref_Str(const char * name, const char * pref, String value);

void savePref_Int(const char * name, const char * pref, int value);

String getPref_Str(const char * name, const char * pref);

int getPref_Int(const char * name, const char * pref);


#endif