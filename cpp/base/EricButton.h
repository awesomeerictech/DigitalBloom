#ifndef ERICBUTTON_H
#define ERICBUTTON_H



#include "precompiled.h"

class EricButton : public Wt::WPushButton
{

public:

 EricButton(int whichui) : whichui_(whichui)

 {


 }


int whichui_;

protected:

virtual void updateDom(Wt::DomElement& element, bool all) override {

if(whichui_==1) {

//Wt::DomElement *e = Wt::DomElement::getForUpdate(this, domElementType());
Wt::DomElement *e = Wt::DomElement::getForUpdate(this, Wt::DomElementType::BUTTON);
e->setAttribute("type", "button");
e->setAttribute("data-bs-target", "#carouselExampleControls");
e->setAttribute("data-bs-slide-to", "0");
e->setAttribute("class", "active");
e->setAttribute("aria-current", "true");
e->setAttribute("aria-label", "Slide 1");
Wt::WPushButton::updateDom(*e,all);



}

else if(whichui_==2) {

//Wt::DomElement *e = Wt::DomElement::getForUpdate(this, domElementType());
Wt::DomElement *e = Wt::DomElement::getForUpdate(this, Wt::DomElementType::BUTTON);
e->setAttribute("type", "button");
e->setAttribute("data-bs-target", "#carouselExampleControls");
e->setAttribute("data-bs-slide-to", "1");
e->setAttribute("aria-label", "Slide 2");
Wt::WPushButton::updateDom(*e,all);


}

else if(whichui_==3) {

//Wt::DomElement *e = Wt::DomElement::getForUpdate(this, domElementType());
Wt::DomElement *e = Wt::DomElement::getForUpdate(this, Wt::DomElementType::BUTTON);
e->setAttribute("type", "button");
e->setAttribute("data-bs-target", "#carouselExampleControls");
e->setAttribute("data-bs-slide-to", "2");
e->setAttribute("aria-label", "Slide 3");
Wt::WPushButton::updateDom(*e,all);


}

else if(whichui_==4) {

//Wt::DomElement *e = Wt::DomElement::getForUpdate(this, domElementType());
Wt::DomElement *e = Wt::DomElement::getForUpdate(this, Wt::DomElementType::BUTTON);
e->setAttribute("type", "button");
e->setAttribute("class", "carousel-control-prev");
e->setAttribute("data-bs-target", "#carouselExampleControls");
e->setAttribute("data-bs-slide-to", "prev");
Wt::WPushButton::updateDom(*e,all);


}

else if(whichui_==5) {

//Wt::DomElement *e = Wt::DomElement::getForUpdate(this, domElementType());
Wt::DomElement *e = Wt::DomElement::getForUpdate(this, Wt::DomElementType::BUTTON);
e->setAttribute("type", "button");
e->setAttribute("class", "carousel-control-prev");
e->setAttribute("data-bs-target", "#carouselExampleControls");
e->setAttribute("data-bs-slide-to", "next");
Wt::WPushButton::updateDom(*e,all);


}

else {


}



 }




};


#endif // ERICBUTTON_H
