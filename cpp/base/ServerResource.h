#ifndef SERVERRESOURCE_H
#define SERVERRESOURCE_H

#include <Wt/WResource.h>
#include "Parser/Parser.h"


class ServerResource : public Wt::WResource {
public:

ServerResource();
virtual ~ServerResource();
std::string fromIstream(std::istream &stream);
protected:
virtual void handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response);

private:


};

#endif // SERVERRESOURCE_H
