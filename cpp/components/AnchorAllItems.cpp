
#include "MyJsonTools.hpp"
#include "views/Module.h"



MODULE_BEGIN(AnchorAllItems)



Wt::WLink linkallitems = Wt::WLink(Wt::LinkType::InternalPath, "/my-items");

std::unique_ptr<Wt::WAnchor> anchorallitems =
        std::make_unique<Wt::WAnchor>(linkallitems,
                        "All Items");
anchorallitems->setStyleClass("text-dark");



MODULE_END(return std::move(anchorallitems))
