#ifndef USERNAMERESOURCE_H
#define USERNAMERESOURCE_H

#include <Wt/WResource.h>
#include "Parser/Parser.h"
#include "CopyAssets.h"
#include "base/DeferredWidget.h"
#include "views/MainTemplate.h"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <Wt/WAbstractItemModel.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WString.h>
#include "base/CsvUtil.h"
#include "base/DataNet.h"
#include <Wt/WString.h>
#include <Wt/WText.h>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "views/AppRoles.h"
#include "views/AppLinks.h"


class UserNameResource : public Wt::WResource {
public:
  UserNameResource();
  std::string fromIstream(std::istream &stream);
  std::shared_ptr<QString> myerrormessage;
  std::shared_ptr<QString> mysuccessmessage;
  void removewidget();
  void removetpl(Wt::WTemplate * mytpl=nullptr,std::string name="");
  Wt::Signal<std::string>& usernameresourceready() { return usernameresource_; }
  void handlenameresource(std::string myuser);
  // Timer Handlers
  void mytimerendpointuser();
  // DCTOR
  virtual ~UserNameResource();

protected:
  virtual void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) override;


private:

Wt::Signal<std::string> usernameresource_;

};

#endif // USERNAMERESOURCE_H
