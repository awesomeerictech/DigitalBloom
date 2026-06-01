
#include "MyJsonTools.hpp"
#include "views/Module.h"



MODULE_BEGIN(AnchorRegister)



Wt::WLink linkregister = Wt::WLink(Wt::LinkType::InternalPath, "/account-set-up/register");

std::unique_ptr<Wt::WAnchor> anchorregister =
        std::make_unique<Wt::WAnchor>(linkregister,
                        "Create One");
anchorregister->setStyleClass("text-dark");



MODULE_END(return std::move(anchorregister))
