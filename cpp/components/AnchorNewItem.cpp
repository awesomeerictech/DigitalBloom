
#include "MyJsonTools.hpp"
#include "views/Module.h"



MODULE_BEGIN(AnchorNewItem)



Wt::WLink linknewitems = Wt::WLink(Wt::LinkType::InternalPath, "/my-items/new-item");

std::unique_ptr<Wt::WAnchor> anchornewitems =
        std::make_unique<Wt::WAnchor>(linknewitems,
                        "New Item");
anchornewitems->setStyleClass("text-dark");



MODULE_END(return std::move(anchornewitems))
