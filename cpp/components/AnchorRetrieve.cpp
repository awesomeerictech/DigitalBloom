
#include "MyJsonTools.hpp"
#include "views/Module.h"



MODULE_BEGIN(AnchorRetrieve)



Wt::WLink linkretrieve = Wt::WLink(Wt::LinkType::InternalPath, "/account-set-up/retrieve-pin");

std::unique_ptr<Wt::WAnchor> anchorretrieve =
        std::make_unique<Wt::WAnchor>(linkretrieve,
                        "Forgot Pin?");
anchorretrieve->setStyleClass("float-end");




MODULE_END(return std::move(anchorretrieve))
