
#ifndef MAIN_TEMPLATE_H_
#define MAIN_TEMPLATE_H_




#include "MyJsonTools.hpp"
#include "Module.h"

class MainTemplate : public Wt::WTemplate
{
public:
  MainTemplate(const char *trKey);

  virtual void resolveString(const std::string& varName,
			     const std::vector<Wt::WString>& args,
			     std::ostream& result);
  static Wt::WString reindent(const Wt::WString& text);

private:
  std::string docUrl(const std::string& className);
  std::string getString(const std::string& varName);
  
  static std::string escape(const std::string& name);
};

#endif // MAIN_TEMPLATE_H_
