#ifndef CAROUSEL_H
#define CAROUSEL_H


#include "precompiled.h"

class Carousel : public Wt::WContainerWidget
{


protected:

virtual void updateDom(Wt::DomElement& element, bool all) override {

//Wt::DomElement *e = Wt::DomElement::getForUpdate(this, domElementType());
Wt::DomElement *e = Wt::DomElement::getForUpdate(this, Wt::DomElementType::DIV);
e->setAttribute("id", "carouselExampleControls");
e->setAttribute("class", "carousel slide  carousel-dark");
e->setAttribute("data-bs-ride", "carousel");
Wt::WContainerWidget::updateDom(*e,all);



 }




};



#endif // CAROUSEL_H
