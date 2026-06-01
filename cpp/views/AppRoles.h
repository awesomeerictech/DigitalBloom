#ifndef APPROLES_H
#define APPROLES_H


#include "MyJsonTools.hpp"



enum AppRoles {
  NORMALUSER,
  GUEST,
  WEBMASTER,
  ADMIN,
  SUPERPOWER
};

extern std::wstring RandomRole(AppRoles theroles);



#endif // APPROLES_H
