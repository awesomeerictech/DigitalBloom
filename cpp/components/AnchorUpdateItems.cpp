
#include "MyJsonTools.hpp"
#include "views/Module.h"



MODULE_BEGIN(AnchorUpdateItems)



Wt::WLink linkupdateitems = Wt::WLink(Wt::LinkType::InternalPath, "/my-items/update-item");

std::unique_ptr<Wt::WAnchor> anchorupdateitems =
        std::make_unique<Wt::WAnchor>(linkupdateitems,
                        "Update Item");
anchorupdateitems->setStyleClass("text-dark");



MODULE_END(return std::move(anchorupdateitems))
