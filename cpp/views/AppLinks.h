#ifndef APPLINKS_H
#define APPLINKS_H


#include "MyJsonTools.hpp"




enum AppLinks {
  APIURL,
  UPLOADURL,
  MEDIAURL,
  STREAMURL,
  HELPURL,
  WEBURL,
  LOCALDATAURL,
  LOCALMEDIAURL,
  LOCALSTREAMURL,
  LOCALSERVERTMP,
  LOCALSERVERROOT,
  REGISTERTOKEN,
  LOGINTOKEN
};

extern std::wstring RandomLinks(AppLinks thelinks);

#endif // APPLINKS_H
