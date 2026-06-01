
#include "MyJsonTools.hpp"
#include "views/Module.h"



MODULE_BEGIN(AnchorSetUp)


Wt::WLink linksetup = Wt::WLink(Wt::LinkType::InternalPath, "/account-set-up/");

std::unique_ptr<Wt::WAnchor> anchorsetup =
        std::make_unique<Wt::WAnchor>(linksetup,
                        "Login");

anchorsetup->setStyleClass("text-dark");





MODULE_END(return std::move(anchorsetup))
