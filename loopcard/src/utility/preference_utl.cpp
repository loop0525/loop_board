#include "preference_utl.h"

Preferences prefs; // nvs存储操作对象


void savePref_Str(const char * name, const char * pref, String value){
    prefs.begin(name); //命名空间初始化函数
    prefs.putString(pref, value);
}
void savePref_Int(const char * name, const char * pref, int value){
    prefs.begin(name); 
    prefs.putInt(pref, value);
}

String getPref_Str(const char * name, const char * pref){
    prefs.begin(name);
    if(prefs.isKey(pref)) { // 如果当前命名空间中有键名为"name"的元素
      return prefs.getString(pref);
    }else{
      prefs.putString(pref, "is new");
      return "is new";
    }
}

int getPref_Int(const char * name, const char * pref){
    prefs.begin(name);
    if(prefs.isKey(pref)) { // 如果当前命名空间中有键名为"name"的元素
      return prefs.getInt(pref);
    }else{
      prefs.putInt(pref, 0);
      return 0;
    }
}
    


