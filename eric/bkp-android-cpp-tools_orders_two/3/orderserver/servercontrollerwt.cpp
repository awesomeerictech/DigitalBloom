#include "servercontrollerwt.h"



#include "base/UserNameResource.h"
#include "base/MapperSoftware.h"
#include "base/ServerResource.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include <vector>
#include <string>
#include <csignal>
#include <atomic>
#include <iostream>
#include <archive.h>
#include <archive_entry.h>
#include <QResource>
#include "LibArchiveExtractor.h"
#include "MyJsonTools.hpp"
#include "Util/logger.h"


// Your project headers -- do not duplicate mapper or server resource implementation here.
// Make sure these headers exist in your repo and contain the definitions.
#include "base/ServerResource.h"    // defines ServerResource
// CopyAssets may be defined in your project. Include its header if you have one.
#include "CopyAssets.h"            // defines CopyAssets (cpDir)
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)

#include "tools/NotificationManager.h"

#else

#endif



// Start Server Ops



namespace {

const std::string showMenuText = "<i class='fa fa-bars' aria-hidden='true'></i> Show menu";
const std::string closeMenuText = "<i class='fa fa-bars' aria-hidden='true'></i> Close menu";

}





  MapperSoftware::MapperSoftware (const Wt::WEnvironment& myenv)
    : Wt::WApplication(myenv),openMenuButton_(nullptr),menuOpen_(false)
  {



      appvieweric_ = new  Wt::WTemplate();
      auto strv = Wt::WString::tr("logomenu1");
      appvieweric_->setTemplateText(strv);

     // Wt::WApplication::instance()->root()

      mypayload = std::make_shared<MyJsonTools>();
      myjsonutils = std::make_shared<eric::JsonUtils>();
      client_ = addChild(std::make_unique<Wt::Http::Client>());
    //  client_->setMaximumResponseSize(0);
        client_->setMaximumResponseSize(5000000);
      //  client_->setTimeout(std::chrono::milliseconds{1000});
      networkutils = std::make_shared<DataNet>(this,client_,mypayload,myjsonutils);
      networkutils->client_->done().connect(networkutils.get(), &DataNet::requestDone);
    //  networkutils->client_->bodyDataReceived().connect(networkutils.get(), &DataNet::mybodyDataReceived);
      idsfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      cryptocenterfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      catsfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      itemsfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      ordersfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      carouselfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      logisticsfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      deliveryfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      metricsfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      locationsfilter = std::make_shared<Wt::WSortFilterProxyModel>();
      idsstandardmodel = std::make_shared<Wt::WStandardItemModel>();
      cryptocenterstandardmodel = std::make_shared<Wt::WStandardItemModel>();
      catsstandardmodel = std::make_shared<Wt::WStandardItemModel>();
      itemsstandardmodel = std::make_shared<Wt::WStandardItemModel>();
      ordersstandardmodel = std::make_shared<Wt::WStandardItemModel>();
      locationsstandardmodel = std::make_shared<Wt::WStandardItemModel>();
      deliverystandardmodel = std::make_shared<Wt::WStandardItemModel>();
      metricsstandardmodel = std::make_shared<Wt::WStandardItemModel>();
      locationsstandardmodel = std::make_shared<Wt::WStandardItemModel>();
      carouselstandardmodel = std::make_shared<Wt::WStandardItemModel>();

      myformmodelnew_ = std::make_shared<Wt::WFormModel>();
      myformmodelupdate_ = std::make_shared<Wt::WFormModel>();
      mytemplateformview_ = new Wt::WTemplateFormView();
      updatetemplateformview_ = new Wt::WTemplateFormView();
      categoryModelnew_ = std::make_shared<Wt::WStandardItemModel>();
      categoryModelupdate_ = std::make_shared<Wt::WStandardItemModel>();
      categorytemplateformview_ = new Wt::WTemplateFormView();
      mycategoryformmodel_ = std::make_shared<Wt::WFormModel>();
      phoneupdatetemplateformview_ = new Wt::WTemplateFormView();
      phoneupdateformmodel_ = std::make_shared<Wt::WFormModel>();
      passupdatetemplateformview_ = new Wt::WTemplateFormView();
      passupdateformmodel_ = std::make_shared<Wt::WFormModel>();
      locinfotemplateformview_ = new Wt::WTemplateFormView();
      locinfoformmodel_ = std::make_shared<Wt::WFormModel>();
      myerrormessage = std::make_shared<QString>();
      mysuccessmessage = std::make_shared<QString>();
      m_notificationManager = NotificationManager::getInstance();






      Wt::WApplication *app = Wt::WApplication::instance();











       allitems_ = new  Wt::WTemplate();
       auto str1 = Wt::WString::tr("appview-view7");
       allitems_->setTemplateText(str1);
       allitems_->bindWidget("AnchorNewItem", AnchorNewItem());
       allitems_->bindWidget("AnchorUpdateItems", AnchorUpdateItems());


       allsettings_ = new  Wt::WTemplate();
       auto strsettings = Wt::WString::tr("uiux1-view3");
       allsettings_->setTemplateText(strsettings);


       allrequests_ = new  Wt::WTemplate();
       auto strreq = Wt::WString::tr("uiux1-view4");
       allrequests_->setTemplateText(strreq);


       helptpl1_ = new  Wt::WTemplate();
       auto strhelp1 = Wt::WString::tr("appview-view1");
       helptpl1_->setTemplateText(strhelp1);

       // create stream server and get IPs
       std::unique_ptr<StreamServer> stream_ = std::make_unique<StreamServer>(nullptr);
       QString ip4_ = stream_->getLocalIp();
       QString ip6_ = stream_->getLocalIpV6();
       std::wstring theurl_ = std::wstring();

       QString chosen_;
       QString listenIpval_;
       if (!ip4_.isEmpty()) chosen_ = ip4_;
       else if (!ip6_.isEmpty()) chosen_ = ip6_;
       else {
           WarnL << "Constructor RandomLinks QxHttpServerController::applyListenIpFrom: StreamServer has no non-loopback IP";
           theurl_ = std::wstring(); // <- RETURN empty on error
       }

       // validate (refuse loopback/wildcard)
       if (chosen_.isEmpty() || chosen_ == QLatin1String("0.0.0.0") ||
           chosen_ == QLatin1String("::") || chosen_ == QLatin1String("::1") ||
           chosen_.startsWith(QLatin1String("127."))) {
           WarnL << "Constructor RandomLinks QxHttpServerController::applyListenIpFrom: refused loopback/wildcard ip: " << chosen_.toStdString();
           theurl_ = std::wstring();
       }

       else {

         listenIpval_ = chosen_;


         }


       std::string myfirstlink1 = "http://"+listenIpval_.toStdString()+":8180/?_=/my-items/all-items&business=mybusinessname";

       Wt::WLink linkhelpaaa2 = Wt::WLink(Wt::LinkType::Url,  myfirstlink1);
       linkhelpaaa2.setTarget(Wt::LinkTarget::NewWindow);
       helppageanchor2_ = nullptr;
       helppageanchor2_ = new Wt::WAnchor(linkhelpaaa2,"Click here to start over.");
       helppageanchor2_->clicked().connect([=] {

      // qDebug() << "helppageanchor2_ clicked()" << Qt::endl;




          });


       std::unique_ptr<Wt::WAnchor> helppageanchor2(helppageanchor2_);
       helptpl1_->bindWidget("myhelp2", std::move(helppageanchor2));






        newitem_ = new  Wt::WTemplate();
        auto str2 = Wt::WString::tr("bootstrap-register");
        newitem_->setTemplateText(str2);
        newitem_->bindWidget("AnchorAllItems", AnchorAllItems());
        newitem_->bindWidget("AnchorUpdateItems", AnchorUpdateItems());



        uploadtemplate_ = new  Wt::WTemplate();
        auto upload_  = Wt::WString::tr("appview-view9");
        uploadtemplate_->setTemplateText(upload_);
        containerupload_ = new Wt::WContainerWidget();






         contentsStack_ = appvieweric_->bindNew<Wt::WStackedWidget>("contents");

         Wt::WAnimation animation(Wt::AnimationEffect::Fade,
                                  Wt::TimingFunction::Linear,
                                  200);
         contentsStack_->setTransitionAnimation(animation, true);

         /*
          * Setup the top-level menu
          */
         menu = appvieweric_->bindNew<Wt::WMenu>("menu", contentsStack_);
         menu->addStyleClass("flex-column");
         menu->setInternalPathEnabled();
         menu->setInternalBasePath("/");

         openMenuButton_ = appvieweric_->bindNew<Wt::WPushButton>("open-menu");
         openMenuButton_->setTextFormat(Wt::TextFormat::UnsafeXHTML);
         openMenuButton_->setText(showMenuText);

         openMenuButton_->clicked().connect(this, &MapperSoftware::toggleMenu);

         auto contentsCover = appvieweric_->bindNew<Wt::WContainerWidget>("contents-cover");
         contentsCover->clicked().connect(this, &MapperSoftware::closeMenu);


         addToMenu(menu, "My Items" );

       //  menu->hide();
      //   openMenuButton_->hide();
         menu->select(0);
         menu->itemAt(0)->menu()->itemAt(1)->hide();
         menu->itemAt(0)->menu()->itemAt(2)->hide();
         menu->itemAt(0)->menu()->itemAt(3)->hide();
         menu->itemAt(0)->menu()->itemAt(4)->hide();
         menu->itemAt(0)->menu()->itemAt(5)->hide();
         menu->itemAt(0)->menu()->itemAt(6)->hide();
         menu->itemAt(0)->menu()->select(0);


         const Wt::WEnvironment& env = app->environment();

         if (app->appRoot().empty()) {
           std::cerr << "!!!!!!!!!!" << std::endl
                     << "!! Warning: read the README.md file for hints on deployment,"
                     << " the approot looks suspect!" << std::endl
                     << "!!!!!!!!!!" << std::endl;
         }

         // app->setLayoutDirection(LayoutDirection::RightToLeft);

         // Choice of theme: defaults to bootstrap3 but can be overridden using
         // a theme parameter (for testing)
         const std::string *themePtr = env.getParameter("theme");
         std::string theme;
         if (!themePtr)
           theme = "bootstrap5";
         else
           theme = *themePtr;

         if (theme == "bootstrap5") {
           auto bootstrapTheme = std::make_shared<Wt::WBootstrap5Theme>();
           app->setTheme(bootstrapTheme);
         } else if (theme == "bootstrap3") {
           auto bootstrapTheme = std::make_shared<Wt::WBootstrap3Theme>();
           bootstrapTheme->setResponsive(true);
           app->setTheme(bootstrapTheme);

           // load the default bootstrap3 (sub-)theme
           app->useStyleSheet("resources/themes/bootstrap/3/bootstrap-theme.min.css");
         }

          else if (theme == "bootstrap2") {
           auto bootstrapTheme = std::make_shared<Wt::WBootstrap2Theme>();
           bootstrapTheme->setResponsive(true);
           app->setTheme(bootstrapTheme);
         } else
           app->setTheme(std::make_shared<Wt::WCssTheme>(theme));


         app->messageResourceBundle().use(app->appRoot() + "orders");
         app->messageResourceBundle().use(app->appRoot() + "views");
         app->messageResourceBundle().use(app->appRoot() + "menutxt");
         app->messageResourceBundle().use(app->appRoot() + "account");
         app->messageResourceBundle().use(app->appRoot() + "uiux1");
         app->messageResourceBundle().use(app->appRoot() + "uiux2");
         app->messageResourceBundle().use(app->appRoot() + "uiux3");
         app->messageResourceBundle().use(app->appRoot() + "uiux4");
         app->messageResourceBundle().use(app->appRoot() + "uiux5");
         app->messageResourceBundle().use(app->appRoot() + "uiux6");
         app->messageResourceBundle().use(app->appRoot() + "uiux7");
         app->messageResourceBundle().use(app->appRoot() + "uiux8");
         app->messageResourceBundle().use(app->appRoot() + "uiux9");
         app->messageResourceBundle().use(app->appRoot() + "uiux10");



         app->setTitle("Orders");

         app->useStyleSheet("resources/Font-Awesome/css/all.css");
         app->useStyleSheet("style/deliva.css");
         app->useStyleSheet("style/everywidget.css");
         app->useStyleSheet("style/dragdrop.css");
         app->useStyleSheet("style/combostyle.css");
         app->useStyleSheet("style/pygments.css");
         app->useStyleSheet("style/sample1.css");
         app->useStyleSheet("style/sample2.css");
         app->useStyleSheet("style/carousel.css");
         app->useStyleSheet("style/cards.css");
         app->useStyleSheet("style/layout.css");
         app->useStyleSheet("style/filedrop.css");
       //  app->useStyleSheet("style/mycustom.scss");

        app->internalPathChanged().connect(this, &MapperSoftware::handleSpecificPathChange);












        std::unique_ptr<Wt::WTemplate> appview(appvieweric_);
        Wt::WApplication::instance()->root()->addWidget(std::move(appview));




        Wt::WLink linkarrowleftnewitemaaa = Wt::WLink(Wt::LinkType::InternalPath, "/back");
        arrowleftnewitemanchor_ = new Wt::WAnchor(linkarrowleftnewitemaaa ,"");
        arrowleftnewitemanchor_->setStyleClass("ericcolors1");
        arrowleftnewitemanchor_->setAttributeValue("aria-current","page");
        arrowleftnewitemtxt_ = arrowleftnewitemanchor_->addNew<Wt::WText>(arrowlefticon);
        arrowleftnewitemtxt_->hide();
        arrowleftnewitemanchor_->clicked().connect([=] {

          //qDebug() << "New Item File Upload" << Qt::endl;

          menu->itemAt(0)->menu()->itemAt(3)->hide();
          menu->itemAt(0)->menu()->itemAt(2)->hide();
          menu->itemAt(0)->menu()->select(1);


          });




        std::unique_ptr<Wt::WAnchor> arrowleftnewitemanchor(arrowleftnewitemanchor_);
        uploadtemplate_->bindWidget("AnchorBnewPage", std::move(arrowleftnewitemanchor));

        Wt::WLink linkarrowleftupdateitemaaa = Wt::WLink(Wt::LinkType::InternalPath, "/back");
        arrowleftupdateitemanchor_ = new Wt::WAnchor(linkarrowleftupdateitemaaa ,"");
        arrowleftupdateitemanchor_->setStyleClass("ericcolors1");
        arrowleftupdateitemanchor_->setAttributeValue("aria-current","page");
        arrowleftupdateitemtxt_ = arrowleftupdateitemanchor_->addNew<Wt::WText>(arrowlefticon);
        arrowleftupdateitemtxt_->hide();
        arrowleftupdateitemanchor_->clicked().connect([=] {

         //qDebug() << "Update Item File Upload" << Qt::endl;

         menu->itemAt(0)->menu()->itemAt(3)->hide();
         menu->itemAt(0)->menu()->itemAt(1)->show();
         menu->itemAt(0)->menu()->itemAt(2)->show();
         menu->itemAt(0)->menu()->select(2);

          });

        std::unique_ptr<Wt::WAnchor> arrowleftupdateitemanchor(arrowleftupdateitemanchor_);
        uploadtemplate_->bindWidget("AnchorBupdatePage", std::move(arrowleftupdateitemanchor));



      //  std::unique_ptr<Wt::WAnchor> arrowlefexittupdateitemanchor(arrowleftexitupdateitemanchor_);











        timer3 = std::make_unique<Wt::WTimer>();
        timer3->setInterval(std::chrono::milliseconds(2000));
        timer3->setSingleShot(true);
        timer3->timeout().connect(this, &MapperSoftware::timeouttimer3);
        timer3->start();


        notifytimer3 = std::make_unique<Wt::WTimer>();
        notifytimer3->setInterval(std::chrono::milliseconds(50));
        notifytimer3->setSingleShot(false);
        notifytimer3->timeout().connect(this, &MapperSoftware::timeoutnotifytimer3);
        // notifytimer3->start();
















         networkutils->myrestdata().connect(this, &MapperSoftware::restdata);
         networkutils->myrestnetworkError().connect(this, &MapperSoftware::restnetworkErrordata);
         networkutils->myrestserverError().connect(this, &MapperSoftware::restserverErrordata);
         networkutils->timer1().connect(this, &MapperSoftware::thetimer1);
         networkutils->timer2().connect(this, &MapperSoftware::thetimer2);
         networkutils->timer3().connect(this, &MapperSoftware::thetimer3);




         timerloginuser = std::make_unique<Wt::WTimer>();
         timerloginuser->setInterval(std::chrono::milliseconds(2000));
         timerloginuser->setSingleShot(true);
         timerloginuser->timeout().connect(this, &MapperSoftware::loginuser);

         timerregisteruser = std::make_unique<Wt::WTimer>();
         timerregisteruser->setInterval(std::chrono::milliseconds(2000));
         timerregisteruser->setSingleShot(true);
         timerregisteruser->timeout().connect(this, &MapperSoftware::registeruser);


         timerretrievepassword = std::make_unique<Wt::WTimer>();
         timerretrievepassword->setInterval(std::chrono::milliseconds(2000));
         timerretrievepassword->setSingleShot(true);
         timerretrievepassword->timeout().connect(this, &MapperSoftware::retrievepassword);

         timergetdataall = std::make_unique<Wt::WTimer>();
         timergetdataall->setInterval(std::chrono::milliseconds(2000));
         timergetdataall->setSingleShot(true);
         timergetdataall->timeout().connect(this, &MapperSoftware::thedataall);

         timergetdatauser = std::make_unique<Wt::WTimer>();
         timergetdatauser->setInterval(std::chrono::milliseconds(200));
         timergetdatauser->setSingleShot(true);
         timergetdatauser->timeout().connect(this, &MapperSoftware::thedatauser);

         timerupdateuser = std::make_unique<Wt::WTimer>();
         timerupdateuser->setInterval(std::chrono::milliseconds(2000));
         timerupdateuser->setSingleShot(true);
         timerupdateuser->timeout().connect(this, &MapperSoftware::theupdateuser);

         timernewcategory = std::make_unique<Wt::WTimer>();
         timernewcategory->setInterval(std::chrono::milliseconds(200));
         timernewcategory->setSingleShot(true);
         timernewcategory->timeout().connect(this, &MapperSoftware::thecategorynew);

         timerdeletecategory = std::make_unique<Wt::WTimer>();
         timerdeletecategory->setInterval(std::chrono::milliseconds(100));
         timerdeletecategory->setSingleShot(true);
         timerdeletecategory->timeout().connect(this, &MapperSoftware::thedeletecategory);

         timeritemnew = std::make_unique<Wt::WTimer>();
         timeritemnew->setInterval(std::chrono::milliseconds(500));
         timeritemnew->setSingleShot(true);
         timeritemnew->timeout().connect(this, &MapperSoftware::theitemnew);


         timerdeleteitem = std::make_unique<Wt::WTimer>();
         timerdeleteitem->setInterval(std::chrono::milliseconds(50));
         timerdeleteitem->setSingleShot(true);
         timerdeleteitem->timeout().connect(this, &MapperSoftware::thedeleteitem);

         timerrequestaction = std::make_unique<Wt::WTimer>();
         timerrequestaction->setInterval(std::chrono::milliseconds(100));
         timerrequestaction->setSingleShot(true);
         timerrequestaction->timeout().connect(this, &MapperSoftware::runorderrequest);

         timergeneric1 = std::make_unique<Wt::WTimer>();
         timergeneric1->setInterval(std::chrono::milliseconds(200));
         timergeneric1->setSingleShot(true);
         timergeneric1->timeout().connect(this, &MapperSoftware::thegenericdata1);

         timergeneric2 = std::make_unique<Wt::WTimer>();
         timergeneric2->setInterval(std::chrono::milliseconds(200));
         timergeneric2->setSingleShot(true);
         timergeneric2->timeout().connect(this, &MapperSoftware::thegenericdata2);

         timerupdatepassword = std::make_unique<Wt::WTimer>();
         timerupdatepassword->setInterval(std::chrono::milliseconds(1000));
         timerupdatepassword->setSingleShot(true);
         timerupdatepassword->timeout().connect(this, &MapperSoftware::theupdatepassword);

         timerupdatephonenumber = std::make_unique<Wt::WTimer>();
         timerupdatephonenumber->setInterval(std::chrono::milliseconds(1000));
         timerupdatephonenumber->setSingleShot(true);
         timerupdatephonenumber->timeout().connect(this, &MapperSoftware::theupdatephonenumber);

         timerlogout = std::make_unique<Wt::WTimer>();
         timerlogout->setInterval(std::chrono::milliseconds(200));
         timerlogout->setSingleShot(true);
         timerlogout->timeout().connect(this, &MapperSoftware::thelogout);


         timerneworder = std::make_unique<Wt::WTimer>();
         timerneworder->setInterval(std::chrono::milliseconds(200));
         timerneworder->setSingleShot(true);
         timerneworder->timeout().connect(this, &MapperSoftware::theordernew);

         timerdeleteorder = std::make_unique<Wt::WTimer>();
         timerdeleteorder->setInterval(std::chrono::milliseconds(200));
         timerdeleteorder->setSingleShot(true);
         timerdeleteorder->timeout().connect(this, &MapperSoftware::thedeleteorder);

         timernewlogistics = std::make_unique<Wt::WTimer>();
         timernewlogistics->setInterval(std::chrono::milliseconds(2000));
         timernewlogistics->setSingleShot(true);
         timernewlogistics->timeout().connect(this, &MapperSoftware::thelogisticsnew);


         timerdeletelogistics = std::make_unique<Wt::WTimer>();
         timerdeletelogistics->setInterval(std::chrono::milliseconds(2000));
         timerdeletelogistics->setSingleShot(true);
         timerdeletelogistics->timeout().connect(this, &MapperSoftware::thedeletelogistics);

         timernewdelivery = std::make_unique<Wt::WTimer>();
         timernewdelivery->setInterval(std::chrono::milliseconds(2000));
         timernewdelivery->setSingleShot(true);
         timernewdelivery->timeout().connect(this, &MapperSoftware::thedeliverynew);


         timerdeletedelivery = std::make_unique<Wt::WTimer>();
         timerdeletedelivery->setInterval(std::chrono::milliseconds(2000));
         timerdeletedelivery->setSingleShot(true);
         timerdeletedelivery->timeout().connect(this, &MapperSoftware::thedeletedelivery);

         timernewlocation = std::make_unique<Wt::WTimer>();
         timernewlocation->setInterval(std::chrono::milliseconds(2000));
         timernewlocation->setSingleShot(true);
         timernewlocation->timeout().connect(this, &MapperSoftware::thelocationnew);


         timerdeletelocation = std::make_unique<Wt::WTimer>();
         timerdeletelocation->setInterval(std::chrono::milliseconds(2000));
         timerdeletelocation->setSingleShot(true);
         timerdeletelocation->timeout().connect(this, &MapperSoftware::thedeletelocation);

         timernewmetrics = std::make_unique<Wt::WTimer>();
         timernewmetrics->setInterval(std::chrono::milliseconds(2000));
         timernewmetrics->setSingleShot(true);
         timernewmetrics->timeout().connect(this, &MapperSoftware::themetricsnew);


         timerdeletemetrics = std::make_unique<Wt::WTimer>();
         timerdeletemetrics->setInterval(std::chrono::milliseconds(2000));
         timerdeletemetrics->setSingleShot(true);
         timerdeletemetrics->timeout().connect(this, &MapperSoftware::thedeletemetrics);

         timerdeletewidget = std::make_unique<Wt::WTimer>();
         timerdeletewidget->setInterval(std::chrono::milliseconds(2000));
         timerdeletewidget->setSingleShot(true);
         timerdeletewidget->timeout().connect(this, &MapperSoftware::removewidget);


         timercarousel = std::make_unique<Wt::WTimer>();
         timercarousel->setInterval(std::chrono::milliseconds(30000));
         timercarousel->setSingleShot(false);
         timercarousel->timeout().connect(this, &MapperSoftware::mycarousel);

         timerstartup = std::make_unique<Wt::WTimer>();
         timerstartup->setInterval(std::chrono::milliseconds(10));
         timerstartup->setSingleShot(true);
         timerstartup->timeout().connect(this, &MapperSoftware::mystartup);

         timerdelay = std::make_unique<Wt::WTimer>();
         timerdelay->setInterval(std::chrono::milliseconds(60000));
         timerdelay->setSingleShot(false);
         timerdelay->timeout().connect(this, &MapperSoftware::mydelay);


         Wt::WLink linkloginaaa = Wt::WLink(Wt::LinkType::InternalPath, "/login");
         signiniconanchor1_ = new Wt::WAnchor(linkloginaaa,"");
         signiniconanchor1_->setStyleClass("ericcolors1");
         signiniconanchor1_->setAttributeValue("aria-current","page");
         signiniconscope1_ = signiniconanchor1_->addNew<Wt::WText>(signinicon);
         signiniconscope1_->show();





         signiniconanchor1_->clicked().connect([=] {


             if(timercarousel->isActive()) {

               timercarousel->stop();

             }

             if(appvieweric_->isVisible()) {

                 if(accregister_)
                     accregister_->hide();
                 if(accretrieve_)
                     accretrieve_->hide();
                 if(accsetup_)
                    accsetup_->show();

                   appvieweric_->hide();


             }

             else {



             }

         });






         Wt::WLink linksgoaaa = Wt::WLink(Wt::LinkType::InternalPath, "/signout");
         signouticonanchor1_ = new Wt::WAnchor(linksgoaaa,"");
         signouticonanchor1_->setStyleClass("ericcolors1");
         signouticonanchor1_->setAttributeValue("aria-current","page");
         signouticonscope1_ = signouticonanchor1_->addNew<Wt::WText>(signouticon);
         signouticonscope1_->hide();


         signouticonanchor1_->clicked().connect([=] {

         //qDebug() << "signouticonanchor1_ clicked()" << Qt::endl;

         std::string statusfile;
         statusfile = mypayload->TOKEN_DIRIDS;
         statusfile.append(loggeduserdue.toStdString());
         statusfile.append("/status.txt");
         std::ofstream statusout(statusfile,std::ios::trunc);

         if(statusout) {

             statusout << "out";
             statusout.close();


         }


         QVariantMap info;
         info.insert("user",loggeduserdue);

         QString idsid = "";
         std::string idsfile;
         idsfile = mypayload->TOKEN_DIRIDS;
         idsfile.append(loggeduserdue.toStdString());
         idsfile.append("/ids.txt");



         if(std::ifstream is{idsfile, std::ios_base::in}) {

             auto size = is.tellg();
             char * str;
             std::string content = mypayload->fromIstream(is.read(str,size));
             idsid = QString::fromStdString(content);
              // construct string to stream size
            //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
              is.close();

           }

         info.insert("ids_id",idsid);
         timerlogout->start();
         networkutils->logout(info);




           });




         Wt::WLink linkscategories = Wt::WLink(Wt::LinkType::InternalPath, "/categories/back");
         arrowleftexitcategoriesanchor_ = new Wt::WAnchor(linkscategories,"");
         arrowleftexitcategoriesanchor_->setStyleClass("ericcolors1");
         arrowleftexitcategoriesanchor_->setAttributeValue("aria-current","page");
         arrowleftexitcategoriestxt_ = arrowleftexitcategoriesanchor_->addNew<Wt::WText>(arrowlefticon);
         arrowleftexitcategoriestxt_->show();

         arrowleftexitcategoriesanchor_->clicked().connect([=] {


             menu->itemAt(0)->menu()->itemAt(0)->show();
             if(loggeduserdue==userdue) {

               menu->itemAt(0)->menu()->itemAt(1)->show();

             }
             menu->itemAt(0)->menu()->itemAt(2)->hide();
             menu->itemAt(0)->menu()->itemAt(3)->hide();
             menu->itemAt(0)->menu()->itemAt(4)->hide();
             menu->itemAt(0)->menu()->itemAt(5)->hide();
             menu->itemAt(0)->menu()->select(0);




          });


         Wt::WLink linkhelptpl1 = Wt::WLink(Wt::LinkType::InternalPath, "/help-page/back");
         arrowleftexithelptpl1anchor_ = new Wt::WAnchor(linkhelptpl1,"");
         arrowleftexithelptpl1anchor_->setStyleClass("ericcolors1");
         arrowleftexithelptpl1anchor_->setAttributeValue("aria-current","page");
         arrowleftexithelptpl1txt_ = arrowleftexithelptpl1anchor_->addNew<Wt::WText>(arrowlefticon);
         arrowleftexithelptpl1txt_->show();

         arrowleftexithelptpl1anchor_->clicked().connect([=] {


             menu->itemAt(0)->menu()->itemAt(0)->show();
             if(loggeduserdue==userdue) {

               menu->itemAt(0)->menu()->itemAt(1)->show();

             }
             menu->itemAt(0)->menu()->itemAt(2)->hide();
             menu->itemAt(0)->menu()->itemAt(3)->hide();
             menu->itemAt(0)->menu()->itemAt(4)->hide();
             menu->itemAt(0)->menu()->itemAt(5)->hide();
             menu->itemAt(0)->menu()->itemAt(6)->hide();
             menu->itemAt(0)->menu()->select(0);


             // "All Items"
             // "New Item"
             // "Update Item"
             // "File Upload"
             // "Settings"
             // "Requests"
             // "Help Page"




          });


         Wt::WLink linksrequests = Wt::WLink(Wt::LinkType::InternalPath, "/requests/back");
         arrowleftexitrequestsanchor_ = new Wt::WAnchor(linksrequests,"");
         arrowleftexitrequestsanchor_->setStyleClass("ericcolors1");
         arrowleftexitrequestsanchor_->setAttributeValue("aria-current","page");
         arrowleftexitrequeststxt_ = arrowleftexitrequestsanchor_->addNew<Wt::WText>(arrowlefticon);
         arrowleftexitrequeststxt_->show();

         arrowleftexitrequestsanchor_->clicked().connect([=] {

             menu->itemAt(0)->menu()->itemAt(0)->show();
             if(loggeduserdue==userdue) {

             menu->itemAt(0)->menu()->itemAt(1)->show();

             }
             menu->itemAt(0)->menu()->itemAt(2)->hide();
             menu->itemAt(0)->menu()->itemAt(3)->hide();
             menu->itemAt(0)->menu()->itemAt(4)->hide();
             menu->itemAt(0)->menu()->itemAt(5)->hide();
             menu->itemAt(0)->menu()->select(0);



          });

       //  categoriesbtn_ = new Wt::WPushButton("Settings");
       //  categoriesbtn_->setStyleClass("btn btn-secondary");
         Wt::WLink linkscats = Wt::WLink(Wt::LinkType::InternalPath, "/settings");
         categoriesbtn_ = new Wt::WAnchor(linkscats,"");
         categoriesbtn_->setStyleClass("ericcolors1");
         categoriesbtn_->setAttributeValue("aria-current","page");
         categoriesbtntxt_ = categoriesbtn_->addNew<Wt::WText>(settingsicon);
         categoriesbtntxt_->show();
         categoriesbtn_->clicked().connect([=] {

             if(timercarousel->isActive()) {

                 timercarousel->stop();

             }

             if(loggeduserdue.isEmpty()) {

                 if(appvieweric_->isVisible()) {

                     if(accregister_)
                         accregister_->hide();
                     if(accretrieve_)
                         accretrieve_->hide();
                     if(accsetup_)
                        accsetup_->show();

                       appvieweric_->hide();


                 }

                 else {



                 }


             }

             else if(!loggeduserdue.isEmpty() && (loggeduserdue==userdue)) {

                 menu->itemAt(0)->menu()->itemAt(0)->hide();
                 menu->itemAt(0)->menu()->itemAt(1)->hide();
                 menu->itemAt(0)->menu()->itemAt(2)->hide();
                 menu->itemAt(0)->menu()->itemAt(3)->hide();
                 menu->itemAt(0)->menu()->itemAt(4)->show();
                 menu->itemAt(0)->menu()->itemAt(5)->hide();
                 menu->itemAt(0)->menu()->select(4);




             }


           else {


           customToast("appview-view20","Only the owner of the business can do that!");
           timerdeletewidget->start();


             }



          });

        // requestsbtn_ = new Wt::WPushButton("Requests");
        // requestsbtn_->setStyleClass("btn btn-secondary");
         Wt::WLink linksreqs = Wt::WLink(Wt::LinkType::InternalPath, "/requests");
         requestsbtn_ = new Wt::WAnchor(linksreqs,"");
         requestsbtn_->setStyleClass("ericcolors1");
         requestsbtn_->setAttributeValue("aria-current","page");
         requestsbtntxt_ = requestsbtn_->addNew<Wt::WText>(carticonc);
         requestsbtntxt_->show();
         requestsbtn_->clicked().connect([=] {

             if(timercarousel->isActive()) {

                timercarousel->stop();

               }

            if(loggeduserdue.isEmpty()) {

                 if(appvieweric_->isVisible()) {

                     if(accregister_)
                         accregister_->hide();
                     if(accretrieve_)
                         accretrieve_->hide();
                     if(accsetup_)
                        accsetup_->show();

                       appvieweric_->hide();


                 }

                 else {



                 }


             }

             else if(!loggeduserdue.isEmpty() && !userdue.isEmpty()) {


                 if(loggeduserdue==userdue) {


                 }

                 else {


                 }

                 menu->itemAt(0)->menu()->itemAt(0)->hide();
                 menu->itemAt(0)->menu()->itemAt(1)->hide();
                 menu->itemAt(0)->menu()->itemAt(2)->hide();
                 menu->itemAt(0)->menu()->itemAt(3)->hide();
                 menu->itemAt(0)->menu()->itemAt(4)->hide();
                 menu->itemAt(0)->menu()->itemAt(5)->show();
                 menu->itemAt(0)->menu()->select(5);








             }


             else {


           customToast("appview-view20","You need to login to access this page!");
           timerdeletewidget->start();


             }




          });



         std::unique_ptr<Wt::WAnchor> arrowleftexitcategoriesanchor(arrowleftexitcategoriesanchor_);
         std::unique_ptr<Wt::WAnchor> arrowleftexitrequestsanchor(arrowleftexitrequestsanchor_);
         std::unique_ptr<Wt::WAnchor> arrowleftexithelptpl1anchor(arrowleftexithelptpl1anchor_);
        /* std::unique_ptr<Wt::WPushButton> categoriesbtn(categoriesbtn_);
         std::unique_ptr<Wt::WPushButton> requestsbtn(requestsbtn_); */
         std::unique_ptr<Wt::WAnchor> categoriesbtn(categoriesbtn_);
         std::unique_ptr<Wt::WAnchor> requestsbtn(requestsbtn_);
         std::unique_ptr<Wt::WAnchor> anchorsgoaaa(signouticonanchor1_);
         std::unique_ptr<Wt::WAnchor> anchorloginaaa(signiniconanchor1_);


         header1_ = new Wt::WTemplate();
         auto strheader1 = Wt::WString::tr("appview-view24");
         header1_->setTemplateText(strheader1);

         header1_->bindWidget("AnchorLogin", std::move(anchorloginaaa));
         header1_->bindWidget("AnchorSGO", std::move(anchorsgoaaa));
         header1_->bindWidget("categoriesbtn", std::move(categoriesbtn));
         header1_->bindWidget("requestsbtn", std::move(requestsbtn));

         searchitems_ = new Wt::WLineEdit();
         searchitems_->setAttributeValue("type","search");
         searchitems_->setAttributeValue("placeholder","Search");
         searchitems_->setAttributeValue("aria-label","Search");
         searchitems_->setStyleClass("form-control me-2");
         searchitems_->textInput().connect([=] {

          header1_->setFocus();
          searchitems_->setFocus();
          qDebug() << "itemsmodel count "  << itemsstandardmodel->rowCount() << Qt::endl;
          qDebug() << "carouselmodel count "  << carouselstandardmodel->rowCount() << Qt::endl;
          if(itemsstandardmodel->rowCount()>0 && carouselstandardmodel->rowCount()>0) {
          if(itemsfilter->rowCount() > 0 && carouselfilter->rowCount() > 0) {


         // qDebug() << "Filters not empty! "  << Qt::endl;
          std::string myinputtxt = searchitems_->text().toUTF8();
          std::string spattern = R"(()" + myinputtxt + R"()(.*))";
        //  ordersfilter->setFilterRegExp(std::make_unique<std::regex>(spattern));
          mypatternitem.reset(new std::regex(spattern));
          mypatterncarousel.reset(new std::regex(spattern));
          itemsfilter->setFilterRegExp(std::move(mypatternitem));
          carouselfilter->setFilterRegExp(std::move(mypatterncarousel));
          thedatauserftr();




          }

          else {


           // qDebug() << "Empty Filters!"  << Qt::endl;
            itemsfilter.reset(new Wt::WSortFilterProxyModel());
            itemsfilter->setSourceModel(itemsstandardmodel);
            itemsfilter->setDynamicSortFilter(true);
            itemsfilter->setFilterKeyColumn(6);
            itemsfilter->setFilterRole(Wt::ItemDataRole::Display);
            carouselfilter.reset(new Wt::WSortFilterProxyModel());
            carouselfilter->setSourceModel(carouselstandardmodel);
            carouselfilter->setDynamicSortFilter(true);
            carouselfilter->setFilterKeyColumn(6);
            carouselfilter->setFilterRole(Wt::ItemDataRole::Display);
            thedatauserftr();


          }

          }


         });

        std::unique_ptr<Wt::WLineEdit> searchitems(searchitems_);
        header1_->bindWidget("searchitems", std::move(searchitems));
        std::unique_ptr<Wt::WTemplate> header1(header1_);
        allitems_->bindWidget("myheader1", std::move(header1));



         allsettings_->bindWidget("AnchorAllItems", std::move(arrowleftexitcategoriesanchor));
         allrequests_->bindWidget("AnchorAllItems", std::move(arrowleftexitrequestsanchor));
         helptpl1_->bindWidget("AnchorAllItems", std::move(arrowleftexithelptpl1anchor));

         mycategoryformmodel_->addField(CategoryField);
         mycategoryformmodel_->setValidator(CategoryField, createLongTextValidator(CategoryField));
         auto viewcatme = Wt::WString::tr("uiux1-view6");
         categorytemplateformview_->setTemplateText(viewcatme);
         categorytemplateformview_->addFunction("id", &Wt::WTemplate::Functions::id);
         categorytemplateformview_->addFunction("block", &Wt::WTemplate::Functions::id);
         categorytemplateformview_->setFormWidget(CategoryField ,
                       std::make_unique<Wt::WLineEdit>());
         auto uploadbtn = std::make_unique<Wt::WPushButton>("Add category");
         uploadbtn->setStyleClass("btn btn-primary btn-lg");
         uploadbtn->clicked().connect([=] {



         categorytemplateformview_->updateModel(mycategoryformmodel_.get());

         if (mycategoryformmodel_->validate()) {

             QVariantMap info;
             info.insert("user",loggeduserdue);
             info.insert("name",QString::fromStdString(Wt::asString(mycategoryformmodel_->value(CategoryField)).toUTF8()));

             QString idsid = "";
             std::string idsfile;
             idsfile = mypayload->TOKEN_DIRIDS;
             idsfile.append(loggeduserdue.toStdString());
             idsfile.append("/ids.txt");



             if(std::ifstream is{idsfile, std::ios_base::in}) {

                 auto size = is.tellg();
                 char * str;
                 std::string content = mypayload->fromIstream(is.read(str,size));
                 idsid = QString::fromStdString(content);
                  // construct string to stream size
                //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
                  is.close();

               }

             info.insert("ids_id",idsid);
             timernewcategory->start();
             networkutils->newcategory(info);

             categorytemplateformview_->updateView(mycategoryformmodel_.get());


         } else {

             categorytemplateformview_->updateView(mycategoryformmodel_.get());
         }


         });


         categorytemplateformview_->bindWidget("submit-button",std::move(uploadbtn) );

         //   auto passvalidator = std::make_shared<Wt::WRegExpValidator>("(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9]).{8,}");
         // auto passvalidator = std::make_shared<Wt::WRegExpValidator>("(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}");
         // passvalidator->setMandatory(true);
         auto passvalidator = std::make_shared<Wt::WRegExpValidator>("^[0-9]{4}$");
         passvalidator->setMandatory(true);
         auto phonevalidator = std::make_shared<Wt::WRegExpValidator>("[\+]?[(]?[0-9]{3}[)]?[-\s\.]?[0-9]{3}[-\s\.]?[0-9]{4,6}");
         phonevalidator->setMandatory(true);
         auto emailvalidator = std::make_shared<Wt::WRegExpValidator>("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}");
         emailvalidator->setMandatory(true);

         passupdateformmodel_->addField(PasswordField);
         passupdateformmodel_->setValidator(PasswordField, passvalidator);
         auto viewpassme = Wt::WString::tr("uiux1-view8");
         passupdatetemplateformview_->setTemplateText(viewpassme);
         passupdatetemplateformview_->addFunction("id", &Wt::WTemplate::Functions::id);
         passupdatetemplateformview_->addFunction("block", &Wt::WTemplate::Functions::id);
         passupdatetemplateformview_->setFormWidget(PasswordField ,
                       std::make_unique<Wt::WLineEdit>());
         auto passbtn = std::make_unique<Wt::WPushButton>("Change Password");
         passbtn->setStyleClass("btn btn-primary btn-lg");
         passbtn->clicked().connect([=] {



         passupdatetemplateformview_->updateModel(passupdateformmodel_.get());

         if (passupdateformmodel_->validate()) {

             QVariantMap info;
             info.insert("user",loggeduserdue);
             info.insert("pass",QString::fromStdString(Wt::asString(passupdateformmodel_->value(PasswordField)).toUTF8()));

             QString idsid = "";
             std::string idsfile;
             idsfile = mypayload->TOKEN_DIRIDS;
             idsfile.append(loggeduserdue.toStdString());
             idsfile.append("/ids.txt");



             if(std::ifstream is{idsfile, std::ios_base::in}) {

                 auto size = is.tellg();
                 char * str;
                 std::string content = mypayload->fromIstream(is.read(str,size));
                 idsid = QString::fromStdString(content);
                  // construct string to stream size
                //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
                  is.close();

               }

             info.insert("ids_id",idsid);
             timerupdatepassword->start();
             networkutils->passwordupdate(info);

             passupdatetemplateformview_->updateView(passupdateformmodel_.get());


         } else {

             passupdatetemplateformview_->updateView(passupdateformmodel_.get());
         }


         });

         passupdatetemplateformview_->bindWidget("submit-password-button",std::move(passbtn) );



         phoneupdateformmodel_->addField(PhoneField);
         phoneupdateformmodel_->setValidator(PhoneField, phonevalidator);
         auto viewphoneme = Wt::WString::tr("uiux1-view9");
         phoneupdatetemplateformview_->setTemplateText(viewphoneme);
         phoneupdatetemplateformview_->addFunction("id", &Wt::WTemplate::Functions::id);
         phoneupdatetemplateformview_->addFunction("block", &Wt::WTemplate::Functions::id);
         phoneupdatetemplateformview_->setFormWidget(PhoneField ,
                       std::make_unique<Wt::WLineEdit>());
         auto phonebtn = std::make_unique<Wt::WPushButton>("Change phone number");
         phonebtn->setStyleClass("btn btn-primary btn-lg");
         phonebtn->clicked().connect([=] {



         phoneupdatetemplateformview_->updateModel(phoneupdateformmodel_.get());

         if(phoneupdateformmodel_->validate()) {

             QVariantMap info;
             info.insert("user",loggeduserdue);
             info.insert("phone",QString::fromStdString(Wt::asString(phoneupdateformmodel_->value(PhoneField)).toUTF8()));

             QString idsid = "";
             std::string idsfile;
             idsfile = mypayload->TOKEN_DIRIDS;
             idsfile.append(loggeduserdue.toStdString());
             idsfile.append("/ids.txt");



             if(std::ifstream is{idsfile, std::ios_base::in}) {

                 auto size = is.tellg();
                 char * str;
                 std::string content = mypayload->fromIstream(is.read(str,size));
                 idsid = QString::fromStdString(content);
                  // construct string to stream size
                //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
                  is.close();

               }

             info.insert("ids_id",idsid);
             timerupdatephonenumber->start();
             networkutils->phonenumberupdate(info);

             phoneupdatetemplateformview_->updateView(phoneupdateformmodel_.get());


         } else {

             phoneupdatetemplateformview_->updateView(phoneupdateformmodel_.get());
         }


         });


         phoneupdatetemplateformview_->bindWidget("submit-phone-button",std::move(phonebtn) );

         // Start Location


         locinfoformmodel_->addField(LocationField);
         locinfoformmodel_->setValidator(LocationField, createOptionalValidator());
         auto viewlocme = Wt::WString::tr("uiux1-view10");
         locinfotemplateformview_->setTemplateText(viewlocme);
         locinfotemplateformview_->addFunction("id", &Wt::WTemplate::Functions::id);
         locinfotemplateformview_->addFunction("block", &Wt::WTemplate::Functions::id);
         locinfotemplateformview_->setFormWidget(LocationField ,
                       std::make_unique<Wt::WLineEdit>());
         auto locationbtn = std::make_unique<Wt::WPushButton>("Tell us your location");
         locationbtn->setStyleClass("btn btn-primary btn-lg");
         locationbtn->clicked().connect([=] {

         locinfotemplateformview_->updateModel(locinfoformmodel_.get());

         if(locinfoformmodel_->validate()) {

             QString mylocinfoto =  QString::fromStdString(Wt::asString(locinfoformmodel_->value(LocationField)).toUTF8());
             if( mylocinfoto.isEmpty() || mylocinfoto.isNull()) {

                 reqlocationto = "later";
                 reqlocationfrom = "later";
                }

             else {

                 reqlocationto = mylocinfoto;
                 reqlocationfrom = "later";
             }

             ordersmap.insert("locationto",mylocinfoto);
             ordersmap.insert("locationfrom",reqlocationfrom);
            // qInfo() << "Location: " << QString::fromStdString(Wt::asString(locinfoformmodel_->value(LocationField)).toUTF8()) << Qt::endl;
             locinfotemplateformview_->updateView(locinfoformmodel_.get());


         } else {

             locinfotemplateformview_->updateView(locinfoformmodel_.get());
         }


         });


         locinfotemplateformview_->bindWidget("submit-loc-button",std::move(locationbtn) );

         // End location


         std::unique_ptr<Wt::WTemplateFormView> categorytemplateformview(categorytemplateformview_);
         std::unique_ptr<Wt::WTemplateFormView> passupdatetemplateformview(passupdatetemplateformview_);
         std::unique_ptr<Wt::WTemplateFormView> phoneupdatetemplateformview(phoneupdatetemplateformview_);
         std::unique_ptr<Wt::WTemplateFormView> locinfotemplateformview(locinfotemplateformview_);
         allsettings_->bindWidget("mycatformview", std::move(categorytemplateformview));
         allsettings_->bindWidget("mypassformview", std::move(passupdatetemplateformview));
         allsettings_->bindWidget("myphoneview", std::move(phoneupdatetemplateformview));
         allrequests_->bindWidget("mylocview", std::move(locinfotemplateformview));




         execbtn = new Wt::WPushButton("Execute requests");

         execbtn->setStyleClass("btn btn-primary btn-lg");
         execbtn->clicked().connect([=] {


             if(reqlocationto.isEmpty() || reqlocationto.isNull()) {

                 reqlocationto = "later";

                }

            if(reqlocationfrom.isEmpty() || reqlocationfrom.isNull()) {

                 reqlocationfrom = "later";
             }

          if(ordersmap.count()>0) {

           QList<QVariant> actualvalues = ordersmap.values("itemsid");

           for (int i = 0; i < actualvalues.size(); ++i) {

              //  qDebug()  << "itemsid QList<QVariant> requestpg is: " << actualvalues.at(i).toString()  << Qt::endl;
              reqcounter++;


              }

             runorderrequest();

          }
          else {

        customToast("appview-view20","Make a request first!");
        timerdeletewidget->start();

          }



           });

        std::unique_ptr<Wt::WPushButton> executebtn(execbtn);
        allrequests_->bindWidget("myexecutebtn", std::move(executebtn));






         accsetup_ = new  Wt::WTemplate();
         auto setup = Wt::WString::tr("bootstrap-account-setup");
         accsetup_->setTemplateText(setup);
         std::unique_ptr<Wt::WTemplate>  myaccsetup(accsetup_);
         Wt::WLink linkarrowleftaaa = Wt::WLink(Wt::LinkType::InternalPath, "/back");
         arrowlefticonanchor1_ = new Wt::WAnchor(linkarrowleftaaa ,"");
         arrowlefticonanchor1_->setStyleClass("ericcolors1");
         arrowlefticonanchor1_->setAttributeValue("aria-current","page");
         arrowlefticonscope1_ = arrowlefticonanchor1_->addNew<Wt::WText>(arrowlefticon);
         arrowlefticonscope1_->show();
         arrowlefticonanchor1_->clicked().connect([=] {

             if(!appvieweric_->isVisible()) {

                 if(accregister_)
                     accregister_->hide();
                 if(accretrieve_)
                     accretrieve_->hide();
                 if(accsetup_)
                    accsetup_->hide();

                   appvieweric_->show();


             }

             else {



             }

           });

         Wt::WLink linkhomeaaa = Wt::WLink(Wt::LinkType::InternalPath, "/home");
         homeiconanchor1_ = new Wt::WAnchor(linkhomeaaa,"");
         homeiconanchor1_->setStyleClass("ericcolors1");
         homeiconanchor1_->setAttributeValue("aria-current","page");
         homeiconscope1_ = homeiconanchor1_->addNew<Wt::WText>(homeicon);
         homeiconscope1_->show();
         homeiconanchor1_->clicked().connect([=] {



           });



         Wt::WApplication::instance()->root()->addWidget(std::move(myaccsetup));
       //  std::unique_ptr<Wt::WAnchor> homeiconanchor1(homeiconanchor1_);
         std::unique_ptr<Wt::WAnchor> arrowlefticonanchor1(arrowlefticonanchor1_);
       //  accsetup_->bindWidget("AnchorHPage", std::move(homeiconanchor1));
         accsetup_->bindWidget("AnchorBPage", std::move(arrowlefticonanchor1));
         accsetup_->bindWidget("AnchorRetrieve", AnchorRetrieve());
         accsetup_->bindWidget("AnchorRegister", AnchorRegister());
         editusersetupscope = new Wt::WLineEdit();
         editusersetupscope->setStyleClass("form-control");
         editusersetupscope->setValidator(createNameValidator());
         editusersetupscope->setFocus(true);
         std::unique_ptr<Wt::WLineEdit>  editusersetup(editusersetupscope);
         accsetup_->bindWidget("username", std::move(editusersetup));
         showhideeyescope1 = new Wt::WContainerWidget();
         showhideeyescope1->setStyleClass("input-group-text");
         showhideeyescope1->setInline(true);
         showeyescope1 = showhideeyescope1->addNew<Wt::WText>(showeye);
         showeyescope1->show();
         hideeyescope1 = showhideeyescope1->addNew<Wt::WText>(hideeye);
         hideeyescope1->hide();
         std::unique_ptr<Wt::WContainerWidget> showhideeyeaction1(showhideeyescope1);
         accsetup_->bindWidget("hideshoweye1", std::move(showhideeyeaction1));
         editpasssetupscope = new Wt::WLineEdit();
         editpasssetupscope->setEchoMode(Wt::EchoMode::Password);
         editpasssetupscope->setStyleClass("input form-control");
         editpasssetupscope->setValidator(createNameValidator());
         editpasssetupscope->setFocus(true);
         std::unique_ptr<Wt::WLineEdit>  editpasssetup(editpasssetupscope);
         accsetup_->bindWidget("password", std::move(editpasssetup));
         showeyescope1->clicked().connect([=] {
             hideeyescope1->show();
             showeyescope1->hide();
             editpasssetupscope->setEchoMode(Wt::EchoMode::Normal);



         });
      hideeyescope1->clicked().connect([=] {
             showeyescope1->show();
             hideeyescope1->hide();
             editpasssetupscope->setEchoMode(Wt::EchoMode::Password);
         });
         checkloginsetupscope = new Wt::WCheckBox();
         checkloginsetupscope->setStyleClass("form-check-input");
         std::unique_ptr<Wt::WCheckBox> checkloginsetup (checkloginsetupscope);
         accsetup_->bindWidget("checklogin", std::move(checkloginsetup));
         loginbtnscope=new Wt::WPushButton("Login");
         loginbtnscope->setStyleClass("btn btn-primary ms-auto");
         std::unique_ptr<Wt::WPushButton>  loginbtn(loginbtnscope);
         accsetup_->bindWidget("loginbtn", std::move(loginbtn));
         accsetup_->hide();
         // end  set up

         // start register

         accregister_ = new  Wt::WTemplate();
         auto register_ = Wt::WString::tr("bootstrap-register");
         accregister_->setTemplateText(register_);
         std::unique_ptr<Wt::WTemplate>  myaccregister(accregister_);
         Wt::WApplication::instance()->root()->addWidget(std::move(myaccregister));
         accregister_->bindWidget("AnchorSetUp", AnchorSetUp());
         edituserregscope = new Wt::WLineEdit();
         edituserregscope->setStyleClass("form-control");
         edituserregscope->setValidator(createNameValidator());
         edituserregscope->setFocus(true);
         std::unique_ptr<Wt::WLineEdit>  edituserreg(edituserregscope);
         accregister_->bindWidget("username", std::move(edituserreg));
         editemailregscope = new Wt::WLineEdit();
         editemailregscope->setStyleClass("form-control");
         editemailregscope->setValidator(emailvalidator);
         editemailregscope->setFocus(true);
         std::unique_ptr<Wt::WLineEdit>  editemailreg(editemailregscope);
         accregister_->bindWidget("email", std::move(editemailreg));
         showhideeyescope2 = new Wt::WContainerWidget();
         showhideeyescope2->setStyleClass("input-group-text");
         showhideeyescope2->setInline(true);
         showeyescope2 = showhideeyescope2->addNew<Wt::WText>(showeye);
         showeyescope2->show();
         hideeyescope2 = showhideeyescope2->addNew<Wt::WText>(hideeye);
         hideeyescope2->hide();
         std::unique_ptr<Wt::WContainerWidget> showhideeyeaction2(showhideeyescope2);
         accregister_->bindWidget("hideshoweye2", std::move(showhideeyeaction2));
         showhideeyescope3 = new Wt::WContainerWidget();
         showhideeyescope3->setStyleClass("input-group-text");
         showhideeyescope3->setInline(true);
         showeyescope3 = showhideeyescope3->addNew<Wt::WText>(showeye);
         showeyescope3->show();
         hideeyescope3 = showhideeyescope3->addNew<Wt::WText>(hideeye);
         hideeyescope3->hide();
         std::unique_ptr<Wt::WContainerWidget> showhideeyeaction3(showhideeyescope3);
         accregister_->bindWidget("hideshoweye3", std::move(showhideeyeaction3));
         showhideeyescope4 = new Wt::WContainerWidget();
         showhideeyescope4->setStyleClass("input-group-text");
         showhideeyescope4->setInline(true);
         showeyescope4 = showhideeyescope4->addNew<Wt::WText>(showeye);
         showeyescope4->show();
         hideeyescope4 = showhideeyescope4->addNew<Wt::WText>(hideeye);
         hideeyescope4->hide();
         std::unique_ptr<Wt::WContainerWidget> showhideeyeaction4(showhideeyescope4);
         accregister_->bindWidget("hideshoweye4", std::move(showhideeyeaction4));
         edithintregscope = new Wt::WLineEdit();
         edithintregscope->setEchoMode(Wt::EchoMode::Password);
         edithintregscope->setStyleClass("form-control");
         edithintregscope->setValidator(createNameValidator());
         edithintregscope->setFocus(true);
         std::unique_ptr<Wt::WLineEdit>  edithintreg(edithintregscope);
         accregister_->bindWidget("hint", std::move(edithintreg));
         editphoneregscope=new Wt::WLineEdit();
         editphoneregscope->setStyleClass("form-control");
         editphoneregscope->setValidator(phonevalidator);
         editphoneregscope->setFocus(true);
         std::unique_ptr<Wt::WLineEdit>  editphonereg(editphoneregscope);
         accregister_->bindWidget("phone", std::move(editphonereg));
         editpassregscope= new Wt::WLineEdit();
         editpassregscope->setEchoMode(Wt::EchoMode::Password);
         editpassregscope->setStyleClass("form-control");
         editpassregscope->setValidator(passvalidator);
         editpassregscope->setFocus(true);
         std::unique_ptr<Wt::WLineEdit>  editpassreg(editpassregscope);
         accregister_->bindWidget("password", std::move(editpassreg));
         editpassreenterscope = new Wt::WLineEdit();
         editpassreenterscope->setStyleClass("form-control");
         editpassreenterscope->setEchoMode(Wt::EchoMode::Password);
         reentervalidatorscope = new Wt::WRegExpValidator();
         std::shared_ptr<Wt::WRegExpValidator>  reentervalidator(reentervalidatorscope);
         reentervalidator->setMandatory(true);
         editpassreenterscope->setValidator(reentervalidator);
         editpassreenterscope->setFocus(true);
         std::unique_ptr<Wt::WLineEdit>  editpassreenter(editpassreenterscope);
         accregister_->bindWidget("passwordreenter", std::move(editpassreenter));
         registerbtnscope = new Wt::WPushButton("Register");
         registerbtnscope->setStyleClass("btn btn-primary ms-auto");
         std::unique_ptr<Wt::WPushButton>  registerbtn(registerbtnscope);
         accregister_->bindWidget("registerbtn", std::move(registerbtn));
         accregister_->hide();
         showeyescope2->clicked().connect([=] {
             hideeyescope2->show();
             showeyescope2->hide();
             edithintregscope->setEchoMode(Wt::EchoMode::Normal);


         });
       hideeyescope2->clicked().connect([=] {
             showeyescope2->show();
             hideeyescope2->hide();
             edithintregscope->setEchoMode(Wt::EchoMode::Password);
         });
         showeyescope3->clicked().connect([=] {
             hideeyescope3->show();
             showeyescope3->hide();
             editpassregscope->setEchoMode(Wt::EchoMode::Normal);


         });
        hideeyescope3->clicked().connect([=] {
             showeyescope3->show();
             hideeyescope3->hide();
             editpassregscope->setEchoMode(Wt::EchoMode::Password);
         });
        showeyescope4->clicked().connect([=] {
            hideeyescope4->show();
            showeyescope4->hide();
            editpassreenterscope->setEchoMode(Wt::EchoMode::Normal);


        });

        hideeyescope4->clicked().connect([=] {
            showeyescope4->show();
            hideeyescope4->hide();
            editpassreenterscope->setEchoMode(Wt::EchoMode::Password);
        });

         // end register

         // start retrieve


          accretrieve_ = new  Wt::WTemplate();
          auto retrieve_  = Wt::WString::tr("bootstrap-retrieve");
          accretrieve_ ->setTemplateText(retrieve_ );
          std::unique_ptr<Wt::WTemplate>  accretrieve(accretrieve_);
          Wt::WApplication::instance()->root()->addWidget(std::move(accretrieve));
          accretrieve_->bindWidget("AnchorSetUp", AnchorSetUp());
          edituserretrievescope = new Wt::WLineEdit();
          edituserretrievescope->setStyleClass("form-control");
          edituserretrievescope->setValidator(createNameValidator());
          edituserretrievescope->setFocus(true);
          std::unique_ptr<Wt::WLineEdit>  edituserretrieve(edituserretrievescope);
          accretrieve_->bindWidget("username", std::move(edituserretrieve));
          edithintretrievescope = new Wt::WLineEdit();
          edithintretrievescope->setStyleClass("form-control");
          edithintretrievescope->setValidator(createNameValidator());
          edithintretrievescope->setFocus(true);
          edithintretrievescope->setEchoMode(Wt::EchoMode::Password);
          std::unique_ptr<Wt::WLineEdit>  edithintretrieve(edithintretrievescope);
          accretrieve_->bindWidget("hint", std::move(edithintretrieve));
          showhideeyescope5 = new Wt::WContainerWidget();
          showhideeyescope5->setStyleClass("input-group-text");
          showhideeyescope5->setInline(true);
          showeyescope5 = showhideeyescope5->addNew<Wt::WText>(showeye);
          showeyescope5->show();
          hideeyescope5 = showhideeyescope5->addNew<Wt::WText>(hideeye);
          hideeyescope5->hide();
          std::unique_ptr<Wt::WContainerWidget> showhideeyeaction5(showhideeyescope5);
          accretrieve_->bindWidget("hideshoweye5", std::move(showhideeyeaction5));
          showeyescope5->clicked().connect([=] {
              hideeyescope5->show();
              showeyescope5->hide();
              edithintretrievescope->setEchoMode(Wt::EchoMode::Normal);

          });
       hideeyescope5->clicked().connect([=] {
              showeyescope5->show();
              hideeyescope5->hide();
              edithintretrievescope->setEchoMode(Wt::EchoMode::Password);
          });
         retrievebtnretrievescope= new Wt::WPushButton("Retrieve");
         retrievebtnretrievescope->setStyleClass("btn btn-primary ms-auto");
          std::unique_ptr<Wt::WPushButton>  retrievebtnretrieve(retrievebtnretrievescope);
          accretrieve_->bindWidget("retrievebtn", std::move(retrievebtnretrieve));
          accretrieve_->hide();


         // end retrieve








      // Start handling login here


     (editusersetupscope->validate() != Wt::ValidationState::Valid ||
                 editpasssetupscope->validate() != Wt::ValidationState::Valid
                 ) ? loginbtnscope->setDisabled(true) : loginbtnscope->setDisabled(false) ;

      editpasssetupscope->keyWentUp().connect([&] {


          (editpasssetupscope->validate() != Wt::ValidationState::Valid) ? loginbtnscope->setDisabled(true)
                                              : loginbtnscope->setDisabled(false) ;

          });







         loginbtnscope->clicked().connect([=] {


             if  (editpasssetupscope->validate() != Wt::ValidationState::Valid ||
                 editusersetupscope->validate() != Wt::ValidationState::Valid ) {
                 editpasssetupscope->setText("");
                 loginbtnscope->setDisabled(true);


             }

             else {

                 // Proceed to Sign In


                QVariantMap info;
                info.insert("user",QString::fromStdString(editusersetupscope->text().toUTF8()));
                info.insert("pass",QString::fromStdString(editpasssetupscope->text().toUTF8()));
                timercarousel->stop();
                timerloginuser->start();
                networkutils->loginuser(info);



             }



         });



         // End login operations here



       // Start handling Registration here



         (edituserregscope->validate() != Wt::ValidationState::Valid ||
          editemailregscope->validate() != Wt::ValidationState::Valid ||
          edithintregscope->validate() != Wt::ValidationState::Valid  ||
          editphoneregscope->validate() != Wt::ValidationState::Valid ||
          editpassregscope->validate() != Wt::ValidationState::Valid  ||
          editpassreenterscope->validate() != Wt::ValidationState::Valid
         ) ? registerbtnscope->setDisabled(true) : registerbtnscope->setDisabled(false) ;



          editpassreenterscope->keyWentUp().connect([&] {


              (editpassreenterscope->validate() != Wt::ValidationState::Valid) ? registerbtnscope->setDisabled(true)
                                                  : registerbtnscope->setDisabled(false) ;

              });



          registerbtnscope->clicked().connect([=] {


              if (edituserregscope->validate() != Wt::ValidationState::Valid ||
                   editemailregscope->validate() != Wt::ValidationState::Valid ||
                   edithintregscope->validate() != Wt::ValidationState::Valid  ||
                   editphoneregscope->validate() != Wt::ValidationState::Valid ||
                   editpassregscope->validate() != Wt::ValidationState::Valid  ||
                    editpassreenterscope->validate() != Wt::ValidationState::Valid
                      )



              {
                  editpassreenterscope->setText("");
                  editpassregscope->setText("");
                  registerbtnscope->setDisabled(true);


              }

              else {

                  // Proceed to Register a new User


                  QVariantMap info;
                  info.insert("user",QString::fromStdString(edituserregscope->text().toUTF8()));
                  info.insert("pass",QString::fromStdString(editpassregscope->text().toUTF8()));
                  info.insert("hint",QString::fromStdString(edithintregscope->text().toUTF8()));
                  info.insert("phone",QString::fromStdString(editphoneregscope->text().toUTF8()));
                  info.insert("email",QString::fromStdString(editemailregscope->text().toUTF8()));
                  timercarousel->stop();
                  timerregisteruser->start();
                  networkutils->registeruser(info);




              }



          });




          // End Registration here


       // Start handling Retrieve Password here

      (edituserretrievescope->validate() != Wt::ValidationState::Valid ||
                      edithintretrievescope->validate() != Wt::ValidationState::Valid
                  ) ? retrievebtnretrievescope->setDisabled(true) : retrievebtnretrievescope->setDisabled(false) ;


       edithintretrievescope->keyWentUp().connect([&] {


      (edithintretrievescope->validate() != Wt::ValidationState::Valid) ? retrievebtnretrievescope->setDisabled(true)
                                                  : retrievebtnretrievescope->setDisabled(false) ;

         });


       retrievebtnretrievescope->clicked().connect([=] {


           if  (edituserretrievescope->validate() != Wt::ValidationState::Valid ||
               edithintretrievescope->validate() != Wt::ValidationState::Valid ) {
               edithintretrievescope->setText("");
               retrievebtnretrievescope->setDisabled(true);


           }

           else {

               // Proceed to Retrieve Password

               QVariantMap info;
               info.insert("user",QString::fromStdString(edituserretrievescope->text().toUTF8()));
               info.insert("hint",QString::fromStdString(edithintretrievescope->text().toUTF8()));
               timercarousel->stop();
               timerretrievepassword->start();
               networkutils->retrievepassword(info);


           }



       });



    // End Retrieve Password here




 const std::string *queries0 = env.getParameter("business");  // ?business=value
 std::string parameter0 = "";






// https://0.0.0.0:8180/?_=/my-items/all-items&business=ericm
 // https://0.0.0.0:8180?business=ericm


       if(queries0) {


       allitems_->bindEmpty("items");
       allitems_->bindString("isactive","active",Wt::TextFormat::Plain);
       parameter0 =  queries0->c_str();
     //  qInfo() <<"parameter0 is: " << QString::fromStdString(parameter0) << Qt::endl;
       networkutils->getuseritems(QString::fromStdString(parameter0));
       userdue = QString::fromStdString(parameter0);
       timergetdatauser->start();
       showCustomWidgetT("spinner1","myspinner1",allitems_);
       isstartup = true;


          }


      else {

       //qDebug() <<"No Parameter" << Qt::endl;
       allitems_->bindString("isactive","inactive",Wt::TextFormat::Plain);
       allitems_->bindEmpty("items");
       allitems_->bindEmpty("myspinner1");
       allitems_->bindEmpty("bizname");
       allitems_->bindEmpty("bizphone");
       menu->itemAt(0)->menu()->itemAt(0)->hide();
       menu->itemAt(0)->menu()->itemAt(1)->hide();
       menu->itemAt(0)->menu()->itemAt(2)->hide();
       menu->itemAt(0)->menu()->itemAt(3)->hide();
       menu->itemAt(0)->menu()->itemAt(4)->hide();
       menu->itemAt(0)->menu()->itemAt(5)->hide();
       menu->itemAt(0)->menu()->itemAt(6)->show();
       arrowleftexithelptpl1txt_->hide();
       menu->itemAt(0)->menu()->select(6);


        }




  menu->itemAt(0)->menu()->itemSelected().connect(this, &MapperSoftware::handlemenuitems);


  initModelnew();
  createmodelnew();
  createviewnew();
  initModelupdate();
  createmodelupdate();
  createviewupdate();




  }

  MapperSoftware::~MapperSoftware() {




  }





void MapperSoftware::runorderrequest() {

QList<QVariant> actualvalues = ordersmap.values("itemsid");
int allreqs = ordersmap.count("itemsid");
if(allreqs>0) {

if(execbtn) {


  execbtn->disable();
  locinfotemplateformview_->disable();



}


if(reqcounter<=allreqs && !(makereqcount<0)) {




makereqcount = reqcounter-1;
reqcounter = makereqcount;
QVariantMap info = QVariantMap();
info.insert("ids_id",reqidids);
info.insert("recipient",reqrecipient);
info.insert("deliverer",reqdeliverer);
info.insert("locationto",reqlocationto);
info.insert("locationfrom",reqlocationfrom);
info.insert("user",loggeduserdue);
info.insert("item",actualvalues.at(makereqcount).toString());
if(!timerneworder->isActive()) {

   timerneworder->start();

}


QString donotify = QString("A new order awaits your delivery: customer: %1, location: %2")
                       .arg(loggeduserdue)
                       .arg(reqlocationto);

qInfo() << "Notification is: " << donotify << Qt::endl;

m_notificationManager->setNotification("New Order",donotify, 2,103);

networkutils->newrequest(info);





}

else {



makereqcount = 0;
reqcounter =0;
ordersmap.clear();


}




if(!timerrequestaction->isActive() && reqcounter>0){

timerrequestaction->start();

}

else {

if(timerrequestaction->isActive()){

timerrequestaction->stop();

}

if((reqcounter==0) && (makereqcount==0)) {

ordersmap.clear();

if(execbtn) {


 execbtn->enable();
 locinfotemplateformview_->enable();



}

//qDebug() << "reqcounter final is: " << reqcounter << Qt::endl;
//qDebug() << "makereqcount final is: " << makereqcount << Qt::endl;
customToast("appview-view19","All requests made!");
timerdeletewidget->start();




}



}



}







/*  QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
    while (j != ordersmap.end() && j.key() == "itemsid") {
    qDebug() << "itemsid requestpg  is: " << j.value().toString() << Qt::endl;
    isnew = false;
     ++j;

} */

return;

}




void MapperSoftware::mytimerendpointuser() {


    //qDebug() << "timerendpointuser timer timeout" << Qt::endl;
    //allitems_->bindEmpty("items");

    timercarousel->stop();
     networkutils->getallitems();
     timergetdataall->start();
   //  showCustomWidgetT("spinner1","myspinner1",mapper_->allitems_);
   //  isstartup = true;

}




void MapperSoftware::removewidget() {

if(deletewidget) {


Wt::WApplication::instance()->root()->removeWidget(deletewidget);
deletewidget = nullptr;

 }

if(delcustomwidget) {

Wt::WApplication::instance()->root()->removeWidget(delcustomwidget);
delcustomwidget = nullptr;

}



}

void MapperSoftware::removetpl(Wt::WTemplate * mytpl,std::string name) {

/*if(deletewidget) {


Wt::WApplication::instance()->root()->removeWidget(deletewidget);
deletewidget = nullptr;

 } */

if(delcustomwidget) {

Wt::WApplication::instance()->root()->removeWidget(delcustomwidget);
delcustomwidget = nullptr;

}

if(delcustomtpl) {

mytpl->removeWidget(delcustomtpl);
mytpl->bindEmpty(name);

}

}

void MapperSoftware::showToast(const Wt::WString& msg) {

auto mywidget = std::make_unique<Wt::WText>(msg);

if(deletewidget) {


 Wt::WApplication::instance()->root()->removeWidget(deletewidget);
 deletewidget = Wt::WApplication::instance()->root()->addWidget(std::move(mywidget));

}

else {


 deletewidget = Wt::WApplication::instance()->root()->addWidget(std::move(mywidget));

}



}

void MapperSoftware::customToast(const Wt::WString& tpl, std::string msg) {

auto mywidget = std::make_unique<Wt::WTemplate>();
auto mywidget_  = Wt::WString::tr(tpl.toUTF8().c_str());
mywidget->setTemplateText(mywidget_);
mywidget->bindString("mymsg",msg,Wt::TextFormat::Plain);

if(deletewidget) {


Wt::WApplication::instance()->root()->removeWidget(deletewidget);
deletewidget = Wt::WApplication::instance()->root()->addWidget(std::move(mywidget));

}

else {


deletewidget = Wt::WApplication::instance()->root()->addWidget(std::move(mywidget));

}


}

void MapperSoftware::customToastUD(const Wt::WString& tpl,QVariantMap msg) {

auto mywidget = std::make_unique<Wt::WTemplate>();
auto mywidget_  = Wt::WString::tr(tpl.toUTF8().c_str());
mywidget->setTemplateText(mywidget_);
mywidget->bindString("custname",msg.value("username").toString().toStdString(),Wt::TextFormat::Plain);
mywidget->bindString("custphone",msg.value("phone").toString().toStdString(),Wt::TextFormat::Plain);
mywidget->bindString("custemail",msg.value("email").toString().toStdString(),Wt::TextFormat::Plain);
mywidget->bindString("custid",msg.value("idsid").toString().toStdString(),Wt::TextFormat::Plain);

if(deletewidget) {


Wt::WApplication::instance()->root()->removeWidget(deletewidget);
deletewidget = Wt::WApplication::instance()->root()->addWidget(std::move(mywidget));

}

else {


deletewidget = Wt::WApplication::instance()->root()->addWidget(std::move(mywidget));

}

}

void MapperSoftware::customToastID(const Wt::WString& tpl,QVariantMap msg) {

auto mywidget = std::make_unique<Wt::WTemplate>();
auto mywidget_  = Wt::WString::tr(tpl.toUTF8().c_str());
mywidget->setTemplateText(mywidget_);
mywidget->bindString("itemname",msg.value("name").toString().toStdString(),Wt::TextFormat::Plain);
mywidget->bindString("itemprice",msg.value("price").toString().toStdString(),Wt::TextFormat::Plain);
mywidget->bindString("itemquantity",msg.value("quantity").toString().toStdString(),Wt::TextFormat::Plain);
mywidget->bindString("itemcategory",msg.value("category").toString().toStdString(),Wt::TextFormat::Plain);
mywidget->bindString("itemid",msg.value("itemid").toString().toStdString(),Wt::TextFormat::Plain);

if(deletewidget) {


Wt::WApplication::instance()->root()->removeWidget(deletewidget);
deletewidget = Wt::WApplication::instance()->root()->addWidget(std::move(mywidget));

}

else {


deletewidget = Wt::WApplication::instance()->root()->addWidget(std::move(mywidget));

}

}

void MapperSoftware::showCustomWidgetC(const Wt::WString& tpl, Wt::WContainerWidget  * mywid) {

auto mywidget = std::make_unique<Wt::WTemplate>();
auto mywidget_  = Wt::WString::tr(tpl.toUTF8().c_str());
mywidget->setTemplateText(mywidget_);


if(delcustomwidget) {


mywid->removeWidget(delcustomwidget);
delcustomwidget = mywid->addWidget(std::move(mywidget));

}

else {


delcustomwidget = mywid->addWidget(std::move(mywidget));

}


}

void MapperSoftware::showCustomWidgetT(const Wt::WString& tpl,std::string name , Wt::WTemplate * mytpl) {

auto mywidget = std::make_unique<Wt::WTemplate>();
auto mywidget_  = Wt::WString::tr(tpl.toUTF8().c_str());
mywidget->setTemplateText(mywidget_);


if(delcustomtpl) {


mytpl->removeWidget(delcustomtpl);
delcustomtpl = mytpl->bindWidget(name,std::move(mywidget));

}

else {


delcustomtpl = mytpl->bindWidget(name,std::move(mywidget));

}


}




void MapperSoftware::mystartup() {


return;

}

void MapperSoftware::mydelay() {



return;

}


void MapperSoftware::mycarousel() {

if(mysuccessmessage->isEmpty()) {


customToast("appview-view20","Please refresh the page!");
timerdeletewidget->start();



 }

else {

QString * str = mysuccessmessage.get();
QString data = *str;

//qDebug() << "Data mycarousel is: " << data << Qt::endl;



try {

QVariantList mylist = myjsonutils->toVariantList(data);
QJsonArray arr= QJsonArray::fromVariantList(mylist);
QJsonDocument itemsdoc = QJsonDocument();
QJsonArray itemsarr;

for (auto i = 0; i < arr.size(); ++i) {  // start items
  if (arr[i].isObject()) {

    auto dataObject = arr[i].toObject();
    QJsonValue myvalue = dataObject.value("list_itemsids");
    QJsonArray myarr = myvalue.toArray();
    for (auto i = 0; i < myarr.size(); ++i) {
      if (myarr[i].isObject()) {

      auto dataObjectmyarr = myarr[i].toObject();
      itemsdoc.setObject(dataObjectmyarr);
      mypayload->modifyJsonValue(itemsdoc, "ids_id");
    /*  mypayload->modifyJsonValue(itemsdoc, "list_delimux");
      mypayload->modifyJsonValue(itemsdoc, "list_ordersmu"); */
      itemsarr << itemsdoc.object();





      }

    }





  }

}  // end items



QJsonDocument newdoc3(itemsarr);
QString mynewjson3 = newdoc3.toJson(QJsonDocument::JsonFormat::Indented);
//   //qDebug() << "mynewjson3 is: " << qPrintable(mynewjson3) << Qt::endl;
jsoncons::ojson j3 = jsoncons::ojson::parse(mynewjson3.toStdString());
std::string output3;
jsoncons::csv::csv_options ioptions3;
ioptions3.quote_style(jsoncons::csv::quote_style_kind::nonnumeric);
jsoncons::csv::encode_csv(j3, output3, ioptions3);
// std::cout << "csv output3 is: " << output3 << "\n\n";
carouselstandardmodel->clear();
carouselstandardmodel = csvModel(output3);

//carouselfilter
carouselfilter->setSourceModel(carouselstandardmodel);
carouselfilter->setDynamicSortFilter(true);
carouselfilter->setFilterKeyColumn(6);
// carouselfilter->sort(1);
//  std::regex myreg1 {R"(\w{2}\s∗\d{5}(−\d{4})?)"};
//  std::string itempattern = R"(^item.$)";
//std::string myinputtxt = "ericm";
//std::string spattern = R"(()" + myinputtxt + R"()(.*))";
//carouselfilter->setFilterRegExp(std::make_unique<std::regex>(spattern));
// carouselfilter->setFilterRegExp(std::make_unique<std::regex>("(eri)(.*)"));
carouselfilter->setFilterRole(Wt::ItemDataRole::Display);
int allrows = carouselfilter->rowCount();

/*


std::string s = R"(abc)"
                    R"( followed by not a newline: \n)"
                    " which is then followed by a non-raw literal that's concatenated \n with"
                    " an embedded non-raw newline";

 */

auto carousel = std::make_unique<Wt::WContainerWidget>();
carousel->setStyleClass("carousel-inner");
auto item1 = std::make_unique<Wt::WContainerWidget>();
item1->setStyleClass("carousel-item active");
auto item2 = std::make_unique<Wt::WContainerWidget>();
item2->setStyleClass("carousel-item");
auto item3 = std::make_unique<Wt::WContainerWidget>();
item3->setStyleClass("carousel-item");


if(allrows>0) {




for (int card = globalcarousel; card < carouselcount; ++card) {


 counter++;



 }



if(counter<=allrows) {

// //qDebug() << "counter is: " << counter << Qt::endl;
 globalcarousel = counter;
 carouselcount = counter +3;


}

else {


globalcarousel = 0;
carouselcount = 3;
counter =0;


}



int thecount = globalcarousel;
//int rem = allrows % 3;


//  int rem = allrows % 3;
// int car3 = allrows / 3 ;

//   //qDebug() << "allrows are: " << allrows << Qt::endl;
//qDebug() << "rem is: " << rem << Qt::endl;
//   //qDebug() << "car3 is: " << car3 << Qt::endl;



// qInfo() << "allrows are: " << allrows << Qt::endl;
// qInfo() << "counter is: " << counter << Qt::endl;
// qInfo() << "globalcarousel is: " << globalcarousel << Qt::endl;
// qInfo() << "carouselcount is: " << carouselcount << Qt::endl;
// qInfo() << "thecount is: " << thecount << Qt::endl;





if(thecount+1==allrows) {

 thecount=0;

}


//  https://www.youtube.com/embed/rRoy6I4gKWU?autoplay=1
// https://www.youtube.com/v/rRoy6I4gKWU?autoplay=1









auto appview13aaa_ = std::make_unique<Wt::WTemplate>();
appview13aaa_->setTemplateText("<img src='${imgsrc}' class='img-fluid' />");
Wt::cpp17::any datamedia2aaa = carouselfilter->data(thecount,5);
Wt::WString mediaaaa = Wt::cpp17::any_cast<Wt::WString>(datamedia2aaa);
Wt::cpp17::any mydatastandardizedmediaaaa  = carouselfilter->data(thecount,10);
Wt::WString datastandardizedmediaaaa = Wt::cpp17::any_cast<Wt::WString>(mydatastandardizedmediaaaa);
std::string mysrcaaa = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaaa.toUTF8();
Wt::cpp17::any datadesaaa = carouselfilter->data(thecount,2);
Wt::WString desaaa = Wt::cpp17::any_cast<Wt::WString>(datadesaaa);
Wt::cpp17::any datanameaaa = carouselfilter->data(thecount,6);
Wt::WString nameaaa = Wt::cpp17::any_cast<Wt::WString>(datanameaaa);
Wt::cpp17::any mydatausernameaaa = carouselfilter->data(thecount,11);
Wt::WString datausernameaaa = Wt::cpp17::any_cast<Wt::WString>(mydatausernameaaa);
Wt::cpp17::any mydataitemuuidaaa = carouselfilter->data(thecount,3);
Wt::WString dataitemuuidaaa = Wt::cpp17::any_cast<Wt::WString>(mydataitemuuidaaa);
std::string mydesaaa = desaaa.toUTF8();
std::string mynameaaa = nameaaa.toUTF8();
std::string mediasrcaaa = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaaa.toUTF8();
appview13aaa_->bindString("imgsrc",mediasrcaaa,Wt::TextFormat::Plain);
item1->addWidget(std::move(appview13aaa_));

/*Wt::WLink linkitemsaaa = Wt::WLink(Wt::LinkType::InternalPath, "/items/name/"+mynameaaa);

std::unique_ptr<Wt::WAnchor> anchoritemsaaa = std::make_unique<Wt::WAnchor>(linkitemsaaa,"Request"); */







auto anchoritemsaaa = std::make_unique<Wt::WPushButton>("Request");
anchoritemsaaa->setStyleClass("btn btn-lg btn-outline-info btn-rounded");
auto itemsaaa = anchoritemsaaa.get();


auto anchoritems1aaa = std::make_unique<Wt::WPushButton>("Cancel Request");
anchoritems1aaa->setStyleClass("btn btn-lg btn-outline-danger btn-rounded");
anchoritems1aaa->hide();
auto items1aaa = anchoritems1aaa.get();

if(loggeduserdue==userdue) {

anchoritemsaaa->hide();
anchoritems1aaa->hide();

}


else {

 anchoritemsaaa->show();

 }

anchoritemsaaa->clicked().connect([=] {


if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaaa.toUTF8())) {

    itemsaaa->hide();
    items1aaa->show();
   //  qDebug() << "btnscope3_ clicked()!" << Qt::endl;
     if(timercarousel->isActive()) {

      timercarousel->stop();

     }




      QString idsid = "";
      std::string idsfile;
      idsfile = mypayload->TOKEN_DIRIDS;
      idsfile.append(loggeduserdue.toStdString());
      idsfile.append("/ids.txt");



      if(std::ifstream is{idsfile, std::ios_base::in}) {

          auto size = is.tellg();
          char * str;
          std::string content = mypayload->fromIstream(is.read(str,size));
          idsid = QString::fromStdString(content);
           // construct string to stream size
         //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
           is.close();

        }



if(ordersmap.count("itemsid")<1) {

isnew = true;
ordersmap.clear();

}




if(isnew) {

ordersmap.insert("ids_id",businessuuid);
ordersmap.insert("recipient",idsid);
ordersmap.insert("deliverer",businessuuid);
// ordersmap.insert("locationto","later");
// ordersmap.insert("locationfrom","later");
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaaa.toUTF8()));
//  qDebug() << "isnew is: " << isnew << Qt::endl;



}

else {

//  qDebug() << "isnew is: " << isnew << Qt::endl;

 }

QList<QVariant> values = ordersmap.values("itemsid");
for (int i = 0; i < values.size(); ++i) {

if(values.at(i).toString()==QString::fromStdString(dataitemuuidaaa.toUTF8())) {


inmap = true;

}

else {


inmap = false;

}

}


if(!inmap) {

//  qDebug()  << "Adding itemuuid in DS!" << Qt::endl;
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaaa.toUTF8()));



}

else {

//   qDebug()  << "itemuuid already in DS!" << Qt::endl;

}


QList<QVariant> ids_idvalues = ordersmap.values("ids_id");
for (int i = 0; i < ids_idvalues.size(); ++i) {

//  qDebug()  << "ids_id QList<QVariant> is: " << ids_idvalues.at(i).toString()  << Qt::endl;
reqidids = ids_idvalues.at(i).toString();


}

QList<QVariant> recipientvalues = ordersmap.values("recipient");
for (int i = 0; i < recipientvalues.size(); ++i) {

// qDebug()  << "recipient QList<QVariant> is: " << recipientvalues.at(i).toString()  << Qt::endl;
reqrecipient = recipientvalues.at(i).toString();


}


QList<QVariant> deliverervalues = ordersmap.values("deliverer");
for (int i = 0; i < deliverervalues.size(); ++i) {

// qDebug()  << "deliverer QList<QVariant> is: " << deliverervalues.at(i).toString()  << Qt::endl;
reqdeliverer = deliverervalues.at(i).toString();


}


QList<QVariant> locationtovalues = ordersmap.values("locationto");
for (int i = 0; i < locationtovalues.size(); ++i) {

//  qDebug()  << "locationto QList<QVariant> is: " << locationtovalues.at(i).toString()  << Qt::endl;
reqlocationto = locationtovalues.at(i).toString();

}


QList<QVariant> locationfromvalues = ordersmap.values("locationfrom");
for (int i = 0; i < locationfromvalues.size(); ++i) {

//   qDebug()  << "locationfrom QList<QVariant> is: " << locationfromvalues.at(i).toString()  << Qt::endl;
reqlocationfrom = locationfromvalues.at(i).toString();


}

QList<QVariant> actualvalues = ordersmap.values("itemsid");
for (int i = 0; i < actualvalues.size(); ++i) {

//  qDebug()  << "itemsid QList<QVariant> is: " << actualvalues.at(i).toString()  << Qt::endl;


}




QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
while (j != ordersmap.end() && j.key() == "itemsid") {
// qDebug() << "itemsid   is: " << j.value().toString() << Qt::endl;
isnew = false;
++j;

}








}

else {


customToast("appview-view20","Please sign in and ensure that you are not requesting items from your own business!");
timerdeletewidget->start();


}




});


anchoritems1aaa->clicked().connect([=] {



if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaaa.toUTF8())) {

//  qDebug() << "btnscope3a_ clicked()!" << Qt::endl;
itemsaaa->show();
items1aaa->hide();

QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

         auto size = is.tellg();
         char * str;
         std::string content = mypayload->fromIstream(is.read(str,size));
         idsid = QString::fromStdString(content);
          // construct string to stream size
        //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
          is.close();

}


if(ordersmap.count("itemsid") == 1) {

     isnew = true;
     ordersmap.remove("ids_id",idsid);
     ordersmap.remove("recipient",idsid);
     ordersmap.remove("deliverer",businessuuid);
     ordersmap.remove("locationto",reqlocationto);
     ordersmap.remove("locationfrom",reqlocationfrom);
     ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaaa.toUTF8()));


}

else {

  ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaaa.toUTF8()));



}


   /*  QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
     while (j != ordersmap.end() && j.key() == "itemsid") {
     qDebug() << "itemsid after remove is: " << j.value().toString() << Qt::endl;
      ++j;

        } */










}

else {


customToast("appview-view20","Only the owner of the business can do that!");
timerdeletewidget->start();


}




});






if(delitem1) {

allitems_->removeWidget(delitem1);
allitems_->bindEmpty("itemlinkaaa");
delitem1 = allitems_->bindWidget("itemlinkaaa", std::move(anchoritemsaaa));


}

else {


delitem1 = allitems_->bindWidget("itemlinkaaa", std::move(anchoritemsaaa));


}


if(delitem1a) {

allitems_->removeWidget(delitem1a);
allitems_->bindEmpty("itemlink1aaa");
delitem1a = allitems_->bindWidget("itemlink1aaa", std::move(anchoritems1aaa));


}

else {


delitem1a = allitems_->bindWidget("itemlink1aaa", std::move(anchoritems1aaa));



}





//qDebug() << "appview13aaa_ the count is: " << thecount << Qt::endl;



//thecount = carouselcount-rem;

if(thecount<allrows) {

 thecount = thecount+1;

}

else if(thecount+2==allrows) {


thecount=thecount-1;



}

else if(thecount==allrows) {

  if(thecount>2) {

  thecount=thecount-1;

 }

 else {

   thecount=thecount-1;

 }

}



else {

thecount=0;

}



//qDebug() << "appview13aab_ the count is: " << thecount << Qt::endl;



auto appview13aab_ = std::make_unique<Wt::WTemplate>();
appview13aab_->setTemplateText("<img src='${imgsrc}' class='img-fluid' />");
Wt::cpp17::any datamedia2aab = carouselfilter->data(thecount,5);
Wt::WString mediaaab = Wt::cpp17::any_cast<Wt::WString>(datamedia2aab);
Wt::cpp17::any mydatastandardizedmediaaab  = carouselfilter->data(thecount,10);
Wt::WString datastandardizedmediaaab = Wt::cpp17::any_cast<Wt::WString>(mydatastandardizedmediaaab);
std::string mysrcaab = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaab.toUTF8();
Wt::cpp17::any datadesaab = carouselfilter->data(thecount,2);
Wt::WString desaab = Wt::cpp17::any_cast<Wt::WString>(datadesaab);
Wt::cpp17::any datanameaab = carouselfilter->data(thecount,6);
Wt::WString nameaab = Wt::cpp17::any_cast<Wt::WString>(datanameaab);
Wt::cpp17::any mydatausernameaab = carouselfilter->data(thecount,11);
Wt::WString datausernameaab = Wt::cpp17::any_cast<Wt::WString>(mydatausernameaab);
Wt::cpp17::any mydataitemuuidaab = carouselfilter->data(thecount,3);
Wt::WString dataitemuuidaab = Wt::cpp17::any_cast<Wt::WString>(mydataitemuuidaab);
std::string mydesaab = desaab.toUTF8();
std::string mynameaab = nameaab.toUTF8();
std::string mediasrcaab = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaab.toUTF8();
appview13aab_->bindString("imgsrc",mediasrcaab,Wt::TextFormat::Plain);
item2->addWidget(std::move(appview13aab_));

//qInfo() << "media src: " << QString::fromStdString(mediasrcaab) << Qt::endl;



/* Wt::WLink linkitemsaab = Wt::WLink(Wt::LinkType::InternalPath, "/items/name/"+mynameaab);
std::unique_ptr<Wt::WAnchor> anchoritemsaab = std::make_unique<Wt::WAnchor>(linkitemsaab,"Request"); */
auto anchoritemsaab = std::make_unique<Wt::WPushButton>("Request");
anchoritemsaab->setStyleClass("btn btn-lg btn-outline-info btn-rounded");
auto itemsaab = anchoritemsaab.get();


auto anchoritems2aab = std::make_unique<Wt::WPushButton>("Cancel Request");
anchoritems2aab->setStyleClass("btn btn-lg btn-outline-danger btn-rounded");
anchoritems2aab->hide();
auto items2aab = anchoritems2aab.get();

if(loggeduserdue==userdue) {

anchoritemsaab->hide();
anchoritems2aab->hide();

}


else {

 anchoritemsaab->show();


 }

anchoritemsaab->clicked().connect([=] {


if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaab.toUTF8())) {

    itemsaab->hide();
    items2aab->show();
   //  qDebug() << "btnscope3_ clicked()!" << Qt::endl;
     if(timercarousel->isActive()) {

      timercarousel->stop();

     }




      QString idsid = "";
      std::string idsfile;
      idsfile = mypayload->TOKEN_DIRIDS;
      idsfile.append(loggeduserdue.toStdString());
      idsfile.append("/ids.txt");



      if(std::ifstream is{idsfile, std::ios_base::in}) {

          auto size = is.tellg();
          char * str;
          std::string content = mypayload->fromIstream(is.read(str,size));
          idsid = QString::fromStdString(content);
           // construct string to stream size
         //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
           is.close();

        }



if(ordersmap.count("itemsid")<1) {

isnew = true;
ordersmap.clear();

}




if(isnew) {

ordersmap.insert("ids_id",businessuuid);
ordersmap.insert("recipient",idsid);
ordersmap.insert("deliverer",businessuuid);
ordersmap.insert("locationto",reqlocationto);
ordersmap.insert("locationfrom",reqlocationfrom);
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaab.toUTF8()));
//  qDebug() << "isnew is: " << isnew << Qt::endl;



}

else {

//  qDebug() << "isnew is: " << isnew << Qt::endl;

 }

QList<QVariant> values = ordersmap.values("itemsid");
for (int i = 0; i < values.size(); ++i) {

if(values.at(i).toString()==QString::fromStdString(dataitemuuidaab.toUTF8())) {


inmap = true;

}

else {


inmap = false;

}

}


if(!inmap) {

//  qDebug()  << "Adding itemuuid in DS!" << Qt::endl;
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaab.toUTF8()));



}

else {

//   qDebug()  << "itemuuid already in DS!" << Qt::endl;

}


QList<QVariant> ids_idvalues = ordersmap.values("ids_id");
for (int i = 0; i < ids_idvalues.size(); ++i) {

//  qDebug()  << "ids_id QList<QVariant> is: " << ids_idvalues.at(i).toString()  << Qt::endl;
reqidids = ids_idvalues.at(i).toString();


}

QList<QVariant> recipientvalues = ordersmap.values("recipient");
for (int i = 0; i < recipientvalues.size(); ++i) {

// qDebug()  << "recipient QList<QVariant> is: " << recipientvalues.at(i).toString()  << Qt::endl;
reqrecipient = recipientvalues.at(i).toString();


}


QList<QVariant> deliverervalues = ordersmap.values("deliverer");
for (int i = 0; i < deliverervalues.size(); ++i) {

// qDebug()  << "deliverer QList<QVariant> is: " << deliverervalues.at(i).toString()  << Qt::endl;
reqdeliverer = deliverervalues.at(i).toString();


}


QList<QVariant> locationtovalues = ordersmap.values("locationto");
for (int i = 0; i < locationtovalues.size(); ++i) {

//  qDebug()  << "locationto QList<QVariant> is: " << locationtovalues.at(i).toString()  << Qt::endl;
reqlocationto = locationtovalues.at(i).toString();

}


QList<QVariant> locationfromvalues = ordersmap.values("locationfrom");
for (int i = 0; i < locationfromvalues.size(); ++i) {

//   qDebug()  << "locationfrom QList<QVariant> is: " << locationfromvalues.at(i).toString()  << Qt::endl;
reqlocationfrom = locationfromvalues.at(i).toString();


}

QList<QVariant> actualvalues = ordersmap.values("itemsid");
for (int i = 0; i < actualvalues.size(); ++i) {

//  qDebug()  << "itemsid QList<QVariant> is: " << actualvalues.at(i).toString()  << Qt::endl;


}




QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
while (j != ordersmap.end() && j.key() == "itemsid") {
// qDebug() << "itemsid   is: " << j.value().toString() << Qt::endl;
isnew = false;
++j;

}








}

else {


customToast("appview-view20","Please sign in and ensure that you are not requesting items from your own business!");
timerdeletewidget->start();


}


});

anchoritems2aab->clicked().connect([=] {


if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaab.toUTF8())) {

//  qDebug() << "btnscope3a_ clicked()!" << Qt::endl;
itemsaab->show();
items2aab->hide();

QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

         auto size = is.tellg();
         char * str;
         std::string content = mypayload->fromIstream(is.read(str,size));
         idsid = QString::fromStdString(content);
          // construct string to stream size
        //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
          is.close();

}


if(ordersmap.count("itemsid") == 1) {

     isnew = true;
     ordersmap.remove("ids_id",idsid);
     ordersmap.remove("recipient",idsid);
     ordersmap.remove("deliverer",businessuuid);
     ordersmap.remove("locationto",reqlocationto);
     ordersmap.remove("locationfrom",reqlocationfrom);
     ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaab.toUTF8()));


}

else {

  ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaab.toUTF8()));



}


   /*  QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
     while (j != ordersmap.end() && j.key() == "itemsid") {
     qDebug() << "itemsid after remove is: " << j.value().toString() << Qt::endl;
      ++j;

        } */










}

else {


customToast("appview-view20","Only the owner of the business can do that!");
timerdeletewidget->start();


}



});




if(delitem2) {

allitems_->removeWidget(delitem2);
allitems_->bindEmpty("itemlinkaab");
delitem2 = allitems_->bindWidget("itemlinkaab", std::move(anchoritemsaab));

}

else {


delitem2 = allitems_->bindWidget("itemlinkaab", std::move(anchoritemsaab));


}


if(delitem2b) {

allitems_->removeWidget(delitem2b);
allitems_->bindEmpty("itemlink2aab");
delitem2b = allitems_->bindWidget("itemlink2aab", std::move(anchoritems2aab));

}

else {


delitem2b = allitems_->bindWidget("itemlink2aab", std::move(anchoritems2aab));


}

/*thecount = (carouselcount-rem)+1;

if(thecount==allrows) {

 thecount=0;

}

else {

 thecount = (carouselcount-rem)+1;

}  */

if(thecount<allrows) {

 thecount = thecount+1;

}



else if(thecount==allrows) {



  thecount=thecount-1;



}



else {

thecount=0;

}



//qDebug() << "appview13aac_  the count is: " << thecount << Qt::endl;







auto appview13aac_ = std::make_unique<Wt::WTemplate>();
appview13aac_->setTemplateText("<img src='${imgsrc}' class='img-fluid' />");
Wt::cpp17::any datamedia2aac = carouselfilter->data(thecount,5);
Wt::WString mediaaac = Wt::cpp17::any_cast<Wt::WString>(datamedia2aac);
Wt::cpp17::any mydatastandardizedmediaaac  = carouselfilter->data(thecount,10);
Wt::WString datastandardizedmediaaac = Wt::cpp17::any_cast<Wt::WString>(mydatastandardizedmediaaac);
std::string mysrcaac = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaac.toUTF8();
Wt::cpp17::any datadesaac = carouselfilter->data(thecount,2);
Wt::WString desaac = Wt::cpp17::any_cast<Wt::WString>(datadesaac);
Wt::cpp17::any datanameaac = carouselfilter->data(thecount,6);
Wt::cpp17::any mydatausernameaac = carouselfilter->data(thecount,11);
Wt::WString datausernameaac = Wt::cpp17::any_cast<Wt::WString>(mydatausernameaac);
Wt::cpp17::any mydataitemuuidaac = carouselfilter->data(thecount,3);
Wt::WString dataitemuuidaac = Wt::cpp17::any_cast<Wt::WString>(mydataitemuuidaac);
Wt::WString nameaac = Wt::cpp17::any_cast<Wt::WString>(datanameaac);
std::string mydesaac = desaac.toUTF8();
std::string mynameaac = nameaac.toUTF8();
std::string mediasrcaac = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaac.toUTF8();
appview13aac_->bindString("imgsrc",mediasrcaac,Wt::TextFormat::Plain);
item3->addWidget(std::move(appview13aac_));

/*Wt::WLink linkitemsaac = Wt::WLink(Wt::LinkType::InternalPath, "/items/name/"+mynameaac);
std::unique_ptr<Wt::WAnchor> anchoritemsaac = std::make_unique<Wt::WAnchor>(linkitemsaac,"Request"); */
auto anchoritemsaac = std::make_unique<Wt::WPushButton>("Request");
anchoritemsaac->setStyleClass("btn btn-lg btn-outline-info btn-rounded");
auto ptraac = anchoritemsaac.get();

auto anchoritems3aac = std::make_unique<Wt::WPushButton>("Cancel Request");
anchoritems3aac->setStyleClass("btn btn-lg btn-outline-danger btn-rounded");
auto ptr3aac = anchoritems3aac.get();
anchoritems3aac->hide();

if(loggeduserdue==userdue) {

anchoritemsaac->hide();
anchoritems3aac->hide();

}


else {

 anchoritemsaac->show();


 }

anchoritemsaac->clicked().connect([=] {

if(loggeduserdue.isEmpty()) {

 if(appvieweric_->isVisible()) {

   if(accregister_)
        accregister_->hide();
     if(accretrieve_)
           accretrieve_->hide();
      if(accsetup_)
           accsetup_->show();

              appvieweric_->hide();


        }

else  {




        }


    }

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaac.toUTF8())) {

ptr3aac->show();
ptraac->hide();
//  qDebug() << "btnscope3_ clicked()!" << Qt::endl;
// if(timercarousel->isActive()) {

//  timercarousel->stop();

//  } // modified 23/11/2025




QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

auto size = is.tellg();
char * str;
std::string content = mypayload->fromIstream(is.read(str,size));
idsid = QString::fromStdString(content);
// construct string to stream size
//qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
is.close();

}



if(ordersmap.count("itemsid")<1) {

isnew = true;
ordersmap.clear();

}




if(isnew) {

ordersmap.insert("ids_id",businessuuid);
ordersmap.insert("recipient",idsid);
ordersmap.insert("deliverer",businessuuid);
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaac.toUTF8()));
//  qDebug() << "isnew is: " << isnew << Qt::endl;



}

else {

//  qDebug() << "isnew is: " << isnew << Qt::endl;

}

QList<QVariant> values = ordersmap.values("itemsid");
for (int i = 0; i < values.size(); ++i) {

if(values.at(i).toString()==QString::fromStdString(dataitemuuidaac.toUTF8())) {


    inmap = true;

}

else {


    inmap = false;

    }

}


if(!inmap) {

//  qDebug()  << "Adding itemuuid in DS!" << Qt::endl;
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaac.toUTF8()));


}

else {

    //   qDebug()  << "itemuuid already in DS!" << Qt::endl;

}


QList<QVariant> ids_idvalues = ordersmap.values("ids_id");
for (int i = 0; i < ids_idvalues.size(); ++i) {

    //  qDebug()  << "ids_id QList<QVariant> is: " << ids_idvalues.at(i).toString()  << Qt::endl;
    reqidids = ids_idvalues.at(i).toString();


}

QList<QVariant> recipientvalues = ordersmap.values("recipient");
for (int i = 0; i < recipientvalues.size(); ++i) {

    // qDebug()  << "recipient QList<QVariant> is: " << recipientvalues.at(i).toString()  << Qt::endl;
    reqrecipient = recipientvalues.at(i).toString();


}


QList<QVariant> deliverervalues = ordersmap.values("deliverer");
for (int i = 0; i < deliverervalues.size(); ++i) {

// qDebug()  << "deliverer QList<QVariant> is: " << deliverervalues.at(i).toString()  << Qt::endl;
reqdeliverer = deliverervalues.at(i).toString();


}


QList<QVariant> locationtovalues = ordersmap.values("locationto");
for (int i = 0; i < locationtovalues.size(); ++i) {

    //  qDebug()  << "locationto QList<QVariant> is: " << locationtovalues.at(i).toString()  << Qt::endl;
    reqlocationto = locationtovalues.at(i).toString();

}


QList<QVariant> locationfromvalues = ordersmap.values("locationfrom");
for (int i = 0; i < locationfromvalues.size(); ++i) {

    //   qDebug()  << "locationfrom QList<QVariant> is: " << locationfromvalues.at(i).toString()  << Qt::endl;
    reqlocationfrom = locationfromvalues.at(i).toString();


}

QList<QVariant> actualvalues = ordersmap.values("itemsid");
for (int i = 0; i < actualvalues.size(); ++i) {

    //  qDebug()  << "itemsid QList<QVariant> is: " << actualvalues.at(i).toString()  << Qt::endl;


    }



QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
while (j != ordersmap.end() && j.key() == "itemsid") {
    // qDebug() << "itemsid   is: " << j.value().toString() << Qt::endl;
    isnew = false;
    ++j;

    }








    }

 else {


    customToast("appview-view20","Please sign in and ensure that you are not requesting items from your own business!");
    timerdeletewidget->start();


    }





});

anchoritems3aac->clicked().connect([=] {


if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaac.toUTF8())) {

//  qDebug() << "btnscope3a_ clicked()!" << Qt::endl;
ptr3aac->hide();
ptraac->show();

QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

         auto size = is.tellg();
         char * str;
         std::string content = mypayload->fromIstream(is.read(str,size));
         idsid = QString::fromStdString(content);
          // construct string to stream size
        //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
          is.close();

}


if(ordersmap.count("itemsid") == 1) {

   isnew = true;
   ordersmap.remove("ids_id",idsid);
   ordersmap.remove("recipient",idsid);
   ordersmap.remove("deliverer",businessuuid);
   ordersmap.remove("locationto",reqlocationto);
   ordersmap.remove("locationfrom",reqlocationfrom);
   ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaac.toUTF8()));


}

else {

  ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaac.toUTF8()));



}


   /*  QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
     while (j != ordersmap.end() && j.key() == "itemsid") {
     qDebug() << "itemsid after remove is: " << j.value().toString() << Qt::endl;
      ++j;

        } */










}

else {


customToast("appview-view20","Only the owner of the business can do that!");
timerdeletewidget->start();


}




});




if(delitem3) {

allitems_->removeWidget(delitem3);
allitems_->bindEmpty("itemlinkaac");
delitem3 = allitems_->bindWidget("itemlinkaac", std::move(anchoritemsaac));

}

else {


delitem3 = allitems_->bindWidget("itemlinkaac", std::move(anchoritemsaac));


}

if(delitem3c) {

allitems_->removeWidget(delitem3c);
allitems_->bindEmpty("itemlink3aac");
delitem3c = allitems_->bindWidget("itemlink3aac", std::move(anchoritems3aac));

}

else {


delitem3c = allitems_->bindWidget("itemlink3aac", std::move(anchoritems3aac));


}





   carousel->addWidget(std::move(item1));
   carousel->addWidget(std::move(item2));
   carousel->addWidget(std::move(item3));

   std::string match1 = "https://www.youtube.com/embed";
   std::string match2 = "https://www.youtube.com/v";
   std::string spattern1 = "("+match1+")(.*)";
   std::string spattern2 = "("+match2+")(.*)";
   std::string myinputtxt1aaa = datastandardizedmediaaaa.toUTF8();


   auto appviewordinaryimage1aaa_ = std::make_unique<Wt::WTemplate>();
   auto strviewordinaryimage1aaa = Wt::WString::tr("ordinaryimagesaaa");
   appviewordinaryimage1aaa_->setTemplateText(strviewordinaryimage1aaa);
   appviewordinaryimage1aaa_->bindString("imgsrcaaa",mysrcaaa,Wt::TextFormat::Plain);


   auto appviewordinaryimage2aaa_ = std::make_unique<Wt::WTemplate>();
   auto strviewordinaryimage2aaa = Wt::WString::tr("ordinaryimagesaaa");
   appviewordinaryimage2aaa_->setTemplateText(strviewordinaryimage2aaa);
   appviewordinaryimage2aaa_->bindString("imgsrcaaa",mysrcaaa,Wt::TextFormat::Plain);
   auto appviewordinaryimage2aaa = appviewordinaryimage2aaa_.get();



   auto appviewembedyoutube1aaa_ = std::make_unique<Wt::WTemplate>();
   auto embedyoutube1aaa = Wt::WString::tr("youtubeembed1aaa");
   appviewembedyoutube1aaa_->setTemplateText(embedyoutube1aaa);
   appviewembedyoutube1aaa_->bindString("standardizedmediaaaa",datastandardizedmediaaaa.toUTF8(),Wt::TextFormat::Plain);
   auto appviewembedyoutube1aaa = appviewembedyoutube1aaa_.get();


   auto appviewembedyoutube2aaa_ = std::make_unique<Wt::WTemplate>();
   auto embedyoutube2aaa = Wt::WString::tr("youtubeembed2aaa");
   appviewembedyoutube2aaa_->setTemplateText(embedyoutube2aaa);
   appviewembedyoutube2aaa_->bindWidget("ordinaryimagesaaa", std::move(appviewordinaryimage2aaa_));
   appviewembedyoutube2aaa_->bindWidget("youtubeembed1aaa", std::move(appviewembedyoutube1aaa_));






   appviewordinaryimage2aaa->clicked().connect([=] {

   if((std::regex_match(myinputtxt1aaa, std::regex(spattern1)))) {

      // appviewembedyoutube1aaa->show();
      // appviewordinaryimage2aaa->hide();

     }

   else {



   }



   });



   auto flashcontaineraaa = std::make_unique<Wt::WContainerWidget>();
   auto flashaaa = flashcontaineraaa->addNew<Wt::WFlashObject>(datastandardizedmediaaaa.toUTF8());
   flashaaa->setFlashParameter("allowFullScreen", "true");
   // flashaaa->setFlashParameter("allowFullScreen", "yes");
   auto myposteraaa = std::make_unique<Wt::WImage>(Wt::WLink(mysrcaaa));
   myposteraaa->setStyleClass("img-fluid card-img-top");
   myposteraaa->setAttributeValue("width" ,"100%");
   myposteraaa->setAttributeValue("height" ,"100%");
   flashaaa->setAlternativeContent(std::move(myposteraaa));



   auto containervideoaaa = std::make_unique<Wt::WContainerWidget>();
   containervideoaaa->setStyleClass("card");
   auto videoplyaaa = containervideoaaa->addNew<Wt::WVideo>();
   videoplyaaa->addSource(Wt::WLink(mysrcaaa));
   auto myposter2aaa = std::make_unique<Wt::WImage>(Wt::WLink(datastandardizedmediaaaa.toUTF8()));
   myposter2aaa->setStyleClass("img-fluid card-img-top");
   myposter2aaa->setAttributeValue("width" ,"100%");
   myposter2aaa->setAttributeValue("height" ,"100%");
   videoplyaaa->setAlternativeContent(std::move(myposter2aaa));





   if(datastandardizedmediaaaa.toUTF8()=="later" || datastandardizedmediaaaa.toUTF8()=="null" || datastandardizedmediaaaa.toUTF8()=="nill" || datastandardizedmediaaaa.toUTF8()=="empty" || datastandardizedmediaaaa.toUTF8()=="N/A") {

    allitems_->bindWidget("mediasrcaaa", std::move(appviewordinaryimage1aaa_));

   }

   else if((std::regex_match(myinputtxt1aaa, std::regex(spattern1)))) {

   //  appviewembedyoutube1aaa->hide();
     appviewordinaryimage2aaa->hide();
   allitems_->bindWidget("mediasrcaaa", std::move(appviewembedyoutube2aaa_));



   }

   else if((std::regex_match(myinputtxt1aaa, std::regex(spattern2)))) {


    allitems_->bindWidget("mediasrcaaa", std::move(flashcontaineraaa));



   }

   else if(datastandardizedmediaaaa.toUTF8()=="localvideo" ) {

     allitems_->bindWidget("mediasrcaaa", std::move(containervideoaaa));

    }

   else {

    allitems_->bindWidget("mediasrcaaa", std::move(containervideoaaa));


   }






std::string myinputtxt1aab = datastandardizedmediaaab.toUTF8();


auto appviewordinaryimage1aab_ = std::make_unique<Wt::WTemplate>();
auto strviewordinaryimage1aab = Wt::WString::tr("ordinaryimagesaab");
appviewordinaryimage1aab_->setTemplateText(strviewordinaryimage1aab);
appviewordinaryimage1aab_->bindString("imgsrcaab",mysrcaab,Wt::TextFormat::Plain);

auto appviewordinaryimage2aab_ = std::make_unique<Wt::WTemplate>();
auto strviewordinaryimage2aab = Wt::WString::tr("ordinaryimagesaab");
appviewordinaryimage2aab_->setTemplateText(strviewordinaryimage2aab);
appviewordinaryimage2aab_->bindString("imgsrcaab",mysrcaab,Wt::TextFormat::Plain);
auto appviewordinaryimage2aab = appviewordinaryimage2aab_.get();



auto appviewembedyoutube1aab_ = std::make_unique<Wt::WTemplate>();
auto embedyoutube1aab = Wt::WString::tr("youtubeembed1aab");
appviewembedyoutube1aab_->setTemplateText(embedyoutube1aab);
appviewembedyoutube1aab_->bindString("standardizedmediaaab",datastandardizedmediaaab.toUTF8(),Wt::TextFormat::Plain);
auto appviewembedyoutube1aab = appviewembedyoutube1aab_.get();


auto appviewembedyoutube2aab_ = std::make_unique<Wt::WTemplate>();
auto embedyoutube2aab = Wt::WString::tr("youtubeembed2aab");
appviewembedyoutube2aab_->setTemplateText(embedyoutube2aab);
appviewembedyoutube2aab_->bindWidget("ordinaryimagesaab", std::move(appviewordinaryimage2aab_));
appviewembedyoutube2aab_->bindWidget("youtubeembed1aab", std::move(appviewembedyoutube1aab_));





appviewordinaryimage2aab->clicked().connect([=] {

if((std::regex_match(myinputtxt1aab, std::regex(spattern1)))) {

      // appviewembedyoutube1aab->show();
      // appviewordinaryimage2aab->hide();

     }

   else {



   }



});


auto flashcontaineraab = std::make_unique<Wt::WContainerWidget>();
auto flashaab = flashcontaineraab->addNew<Wt::WFlashObject>(datastandardizedmediaaab.toUTF8());
flashaab->setFlashParameter("allowFullScreen", "true");
// flashaab->setFlashParameter("allowFullScreen", "yes");
auto myposteraab = std::make_unique<Wt::WImage>(Wt::WLink(mysrcaab));
myposteraab->setStyleClass("img-fluid card-img-top");
myposteraab->setAttributeValue("width" ,"100%");
myposteraab->setAttributeValue("height" ,"100%");
flashaab->setAlternativeContent(std::move(myposteraab));


auto containervideoaab = std::make_unique<Wt::WContainerWidget>();
containervideoaab->setStyleClass("card");
auto videoplyaab = containervideoaab->addNew<Wt::WVideo>();
videoplyaab->addSource(Wt::WLink(mysrcaab));
auto myposter2aab = std::make_unique<Wt::WImage>(Wt::WLink(datastandardizedmediaaab.toUTF8()));
myposter2aab->setStyleClass("img-fluid card-img-top");
myposter2aab->setAttributeValue("width" ,"100%");
myposter2aab->setAttributeValue("height" ,"100%");
videoplyaab->setAlternativeContent(std::move(myposter2aab));





if(datastandardizedmediaaab.toUTF8()=="later" || datastandardizedmediaaab.toUTF8()=="null" || datastandardizedmediaaab.toUTF8()=="nill" || datastandardizedmediaaab.toUTF8()=="empty" || datastandardizedmediaaab.toUTF8()=="N/A") {

allitems_->bindWidget("mediasrcaab", std::move(appviewordinaryimage1aab_));

}

else if((std::regex_match(myinputtxt1aab, std::regex(spattern1)))) {

//  appviewembedyoutube1aab->hide();
appviewordinaryimage2aab->hide();
allitems_->bindWidget("mediasrcaab", std::move(appviewembedyoutube2aab_));



}

else if((std::regex_match(myinputtxt1aab, std::regex(spattern2)))) {


 allitems_->bindWidget("mediasrcaab", std::move(flashcontaineraab));



}

else if(datastandardizedmediaaab.toUTF8()=="localvideo" ) {

allitems_->bindWidget("mediasrcaab", std::move(containervideoaab));

}

else {

allitems_->bindWidget("mediasrcaab", std::move(containervideoaab));


}


std::string myinputtxt1aac = datastandardizedmediaaac.toUTF8();


auto appviewordinaryimage1aac_ = std::make_unique<Wt::WTemplate>();
auto strviewordinaryimage1aac = Wt::WString::tr("ordinaryimagesaac");
appviewordinaryimage1aac_->setTemplateText(strviewordinaryimage1aac);
appviewordinaryimage1aac_->bindString("imgsrcaac",mysrcaac,Wt::TextFormat::Plain);

auto appviewordinaryimage2aac_ = std::make_unique<Wt::WTemplate>();
auto strviewordinaryimage2aac = Wt::WString::tr("ordinaryimagesaac");
appviewordinaryimage2aac_->setTemplateText(strviewordinaryimage2aac);
appviewordinaryimage2aac_->bindString("imgsrcaac",mysrcaac,Wt::TextFormat::Plain);
auto appviewordinaryimage2aac = appviewordinaryimage2aac_.get();



auto appviewembedyoutube1aac_ = std::make_unique<Wt::WTemplate>();
auto embedyoutube1aac = Wt::WString::tr("youtubeembed1aac");
appviewembedyoutube1aac_->setTemplateText(embedyoutube1aac);
appviewembedyoutube1aac_->bindString("standardizedmediaaac",datastandardizedmediaaac.toUTF8(),Wt::TextFormat::Plain);
auto appviewembedyoutube1aac = appviewembedyoutube1aac_.get();


auto appviewembedyoutube2aac_ = std::make_unique<Wt::WTemplate>();
auto embedyoutube2aac = Wt::WString::tr("youtubeembed2aac");
appviewembedyoutube2aac_->setTemplateText(embedyoutube2aac);
appviewembedyoutube2aac_->bindWidget("ordinaryimagesaac", std::move(appviewordinaryimage2aac_));
appviewembedyoutube2aac_->bindWidget("youtubeembed1aac", std::move(appviewembedyoutube1aac_));





appviewordinaryimage2aac->clicked().connect([=] {

if((std::regex_match(myinputtxt1aac, std::regex(spattern1)))) {

      // appviewembedyoutube1aac->show();
      // appviewordinaryimage2aac->hide();

     }

   else {



   }



});


auto flashcontaineraac = std::make_unique<Wt::WContainerWidget>();
auto flashaac = flashcontaineraac->addNew<Wt::WFlashObject>(datastandardizedmediaaac.toUTF8());
flashaac->setFlashParameter("allowFullScreen", "true");
// flashaac->setFlashParameter("allowFullScreen", "yes");
auto myposteraac = std::make_unique<Wt::WImage>(Wt::WLink(mysrcaac));
myposteraac->setStyleClass("img-fluid card-img-top");
myposteraac->setAttributeValue("width" ,"100%");
myposteraac->setAttributeValue("height" ,"100%");
flashaac->setAlternativeContent(std::move(myposteraac));


auto containervideoaac = std::make_unique<Wt::WContainerWidget>();
containervideoaac->setStyleClass("card");
auto videoplyaac = containervideoaac->addNew<Wt::WVideo>();
videoplyaac->addSource(Wt::WLink(mysrcaac));
auto myposter2aac = std::make_unique<Wt::WImage>(Wt::WLink(datastandardizedmediaaac.toUTF8()));
myposter2aac->setStyleClass("img-fluid card-img-top");
myposter2aac->setAttributeValue("width" ,"100%");
myposter2aac->setAttributeValue("height" ,"100%");
videoplyaac->setAlternativeContent(std::move(myposter2aac));





if(datastandardizedmediaaac.toUTF8()=="later" || datastandardizedmediaaac.toUTF8()=="null" || datastandardizedmediaaac.toUTF8()=="nill" || datastandardizedmediaaac.toUTF8()=="empty" || datastandardizedmediaaac.toUTF8()=="N/A") {

allitems_->bindWidget("mediasrcaac", std::move(appviewordinaryimage1aac_));

}

else if((std::regex_match(myinputtxt1aac, std::regex(spattern1)))) {

//  appviewembedyoutube1aac->hide();
appviewordinaryimage2aac->hide();
allitems_->bindWidget("mediasrcaac", std::move(appviewembedyoutube2aac_));



}

else if((std::regex_match(myinputtxt1aac, std::regex(spattern2)))) {


 allitems_->bindWidget("mediasrcaac", std::move(flashcontaineraac));



}

else if(datastandardizedmediaaac.toUTF8()=="localvideo" ) {

allitems_->bindWidget("mediasrcaac", std::move(containervideoaac));

}

else {

allitems_->bindWidget("mediasrcaac", std::move(containervideoaac));


}




  /* allitems_->bindString("fav1",mediasrcaaa,Wt::TextFormat::Plain);
   allitems_->bindString("fav2",mediasrcaab,Wt::TextFormat::Plain);
   allitems_->bindString("fav3",mediasrcaac,Wt::TextFormat::Plain); */


   allitems_->bindString("des1",mydesaaa,Wt::TextFormat::Plain);
   allitems_->bindString("des2",mydesaab,Wt::TextFormat::Plain);
   allitems_->bindString("des3",mydesaac,Wt::TextFormat::Plain);
   allitems_->bindString("name1",mynameaaa,Wt::TextFormat::Plain);
   allitems_->bindString("name2",mynameaab,Wt::TextFormat::Plain);
   allitems_->bindString("name3",mynameaac,Wt::TextFormat::Plain);
 //  allitems_->bindWidget("favourite",std::move(carousel));




}

else {

 //qDebug()<< "Model is empty!" << Qt::endl;
 allitems_->bindEmpty("itemlink1aaa");
 allitems_->bindEmpty("itemlink2aab");
 allitems_->bindEmpty("itemlink3aac");
 allitems_->bindEmpty("mediasrcaaa");
 allitems_->bindEmpty("mediasrcaab");
 allitems_->bindEmpty("mediasrcaac");
 allitems_->bindEmpty("fav1");
 allitems_->bindEmpty("fav2");
 allitems_->bindEmpty("fav3");
 allitems_->bindEmpty("des1");
 allitems_->bindEmpty("des2");
 allitems_->bindEmpty("des3");
 allitems_->bindEmpty("name1");
 allitems_->bindEmpty("name2");
 allitems_->bindEmpty("name3");


}


} // end try



catch(std::exception & e) {

  std::cout << "exception from mycarousel is: " << e.what() << std::endl;

}


 }

/*if(!timergetdatauser->isActive()) {

timergetdatauser->start();
networkutils->getuseritems(userdue);



}


if(!(timercarousel->isActive())) {

   timercarousel->start();
}  */




}



void MapperSoftware::mycarouselftr() {



try {


int allrows = carouselfilter->rowCount();

/*


std::string s = R"(abc)"
                    R"( followed by not a newline: \n)"
                    " which is then followed by a non-raw literal that's concatenated \n with"
                    " an embedded non-raw newline";

 */

auto carousel = std::make_unique<Wt::WContainerWidget>();
carousel->setStyleClass("carousel-inner");
auto item1 = std::make_unique<Wt::WContainerWidget>();
item1->setStyleClass("carousel-item active");
auto item2 = std::make_unique<Wt::WContainerWidget>();
item2->setStyleClass("carousel-item");
auto item3 = std::make_unique<Wt::WContainerWidget>();
item3->setStyleClass("carousel-item");


if(allrows>0) {




for (int card = globalcarousel; card < carouselcount; ++card) {


 counter++;



 }



if(counter<=allrows) {

// //qDebug() << "counter is: " << counter << Qt::endl;
 globalcarousel = counter;
 carouselcount = counter +3;


   }

else {


globalcarousel = 0;
carouselcount = 3;
counter =0;


}



int thecount = globalcarousel;
//int rem = allrows % 3;


//  int rem = allrows % 3;
// int car3 = allrows / 3 ;

//   //qDebug() << "allrows are: " << allrows << Qt::endl;
//qDebug() << "rem is: " << rem << Qt::endl;
//   //qDebug() << "car3 is: " << car3 << Qt::endl;



//qDebug() << "allrows are: " << allrows << Qt::endl;
//qDebug() << "counter is: " << counter << Qt::endl;
//qDebug() << "globalcarousel is: " << globalcarousel << Qt::endl;
//qDebug() << "carouselcount is: " << carouselcount << Qt::endl;
//qDebug() << "thecount is: " << thecount << Qt::endl;





if(thecount+1==allrows) {

 thecount=0;

}




auto appview13aaa_ = std::make_unique<Wt::WTemplate>();
appview13aaa_->setTemplateText("<img src='${imgsrc}' class='img-fluid' />");
Wt::cpp17::any datamedia2aaa = carouselfilter->data(thecount,5);
Wt::WString mediaaaa = Wt::cpp17::any_cast<Wt::WString>(datamedia2aaa);
Wt::cpp17::any mydatastandardizedmediaaaa  = carouselfilter->data(thecount,10);
Wt::WString datastandardizedmediaaaa = Wt::cpp17::any_cast<Wt::WString>(mydatastandardizedmediaaaa);
std::string mysrcaaa = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaaa.toUTF8();
Wt::cpp17::any datadesaaa = carouselfilter->data(thecount,2);
Wt::WString desaaa = Wt::cpp17::any_cast<Wt::WString>(datadesaaa);
Wt::cpp17::any datanameaaa = carouselfilter->data(thecount,6);
Wt::WString nameaaa = Wt::cpp17::any_cast<Wt::WString>(datanameaaa);
Wt::cpp17::any mydatausernameaaa = carouselfilter->data(thecount,11);
Wt::WString datausernameaaa = Wt::cpp17::any_cast<Wt::WString>(mydatausernameaaa);
Wt::cpp17::any mydataitemuuidaaa = carouselfilter->data(thecount,3);
Wt::WString dataitemuuidaaa = Wt::cpp17::any_cast<Wt::WString>(mydataitemuuidaaa);
std::string mydesaaa = desaaa.toUTF8();
std::string mynameaaa = nameaaa.toUTF8();
std::string mediasrcaaa = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaaa.toUTF8();
appview13aaa_->bindString("imgsrc",mediasrcaaa,Wt::TextFormat::Plain);
item1->addWidget(std::move(appview13aaa_));

/*Wt::WLink linkitemsaaa = Wt::WLink(Wt::LinkType::InternalPath, "/items/name/"+mynameaaa);

std::unique_ptr<Wt::WAnchor> anchoritemsaaa = std::make_unique<Wt::WAnchor>(linkitemsaaa,"Request"); */
auto anchoritemsaaa = std::make_unique<Wt::WPushButton>("Request");
anchoritemsaaa->setStyleClass("btn btn-lg btn-outline-info btn-rounded");
auto itemsaaa = anchoritemsaaa.get();


auto anchoritems1aaa = std::make_unique<Wt::WPushButton>("Cancel Request");
anchoritems1aaa->setStyleClass("btn btn-lg btn-outline-danger btn-rounded");
anchoritems1aaa->hide();
auto items1aaa = anchoritems1aaa.get();

if(loggeduserdue==userdue) {

anchoritemsaaa->hide();
anchoritems1aaa->hide();

}


else {

 anchoritemsaaa->show();


 }

anchoritemsaaa->clicked().connect([=] {


if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaaa.toUTF8())) {

    itemsaaa->hide();
    items1aaa->show();
   //  qDebug() << "btnscope3_ clicked()!" << Qt::endl;
     if(timercarousel->isActive()) {

     // timercarousel->stop();  //eric ruin

     }




      QString idsid = "";
      std::string idsfile;
      idsfile = mypayload->TOKEN_DIRIDS;
      idsfile.append(loggeduserdue.toStdString());
      idsfile.append("/ids.txt");



      if(std::ifstream is{idsfile, std::ios_base::in}) {

          auto size = is.tellg();
          char * str;
          std::string content = mypayload->fromIstream(is.read(str,size));
          idsid = QString::fromStdString(content);
           // construct string to stream size
         //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
           is.close();

        }



if(ordersmap.count("itemsid")<1) {

isnew = true;
ordersmap.clear();

}




if(isnew) {

ordersmap.insert("ids_id",businessuuid);
ordersmap.insert("recipient",idsid);
ordersmap.insert("deliverer",businessuuid);
ordersmap.insert("locationto",reqlocationto);
ordersmap.insert("locationfrom",reqlocationfrom);
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaaa.toUTF8()));
//  qDebug() << "isnew is: " << isnew << Qt::endl;



}

else {

//  qDebug() << "isnew is: " << isnew << Qt::endl;

 }

QList<QVariant> values = ordersmap.values("itemsid");
for (int i = 0; i < values.size(); ++i) {

if(values.at(i).toString()==QString::fromStdString(dataitemuuidaaa.toUTF8())) {


inmap = true;

}

else {


inmap = false;

}

}


if(!inmap) {

//  qDebug()  << "Adding itemuuid in DS!" << Qt::endl;
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaaa.toUTF8()));



}

else {

//   qDebug()  << "itemuuid already in DS!" << Qt::endl;

}


QList<QVariant> ids_idvalues = ordersmap.values("ids_id");
for (int i = 0; i < ids_idvalues.size(); ++i) {

//  qDebug()  << "ids_id QList<QVariant> is: " << ids_idvalues.at(i).toString()  << Qt::endl;
reqidids = ids_idvalues.at(i).toString();


}

QList<QVariant> recipientvalues = ordersmap.values("recipient");
for (int i = 0; i < recipientvalues.size(); ++i) {

// qDebug()  << "recipient QList<QVariant> is: " << recipientvalues.at(i).toString()  << Qt::endl;
reqrecipient = recipientvalues.at(i).toString();


}


QList<QVariant> deliverervalues = ordersmap.values("deliverer");
for (int i = 0; i < deliverervalues.size(); ++i) {

// qDebug()  << "deliverer QList<QVariant> is: " << deliverervalues.at(i).toString()  << Qt::endl;
reqdeliverer = deliverervalues.at(i).toString();


}


QList<QVariant> locationtovalues = ordersmap.values("locationto");
for (int i = 0; i < locationtovalues.size(); ++i) {

//  qDebug()  << "locationto QList<QVariant> is: " << locationtovalues.at(i).toString()  << Qt::endl;
reqlocationto = locationtovalues.at(i).toString();

}


QList<QVariant> locationfromvalues = ordersmap.values("locationfrom");
for (int i = 0; i < locationfromvalues.size(); ++i) {

//   qDebug()  << "locationfrom QList<QVariant> is: " << locationfromvalues.at(i).toString()  << Qt::endl;
reqlocationfrom = locationfromvalues.at(i).toString();


}

QList<QVariant> actualvalues = ordersmap.values("itemsid");
for (int i = 0; i < actualvalues.size(); ++i) {

//  qDebug()  << "itemsid QList<QVariant> is: " << actualvalues.at(i).toString()  << Qt::endl;


}




QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
while (j != ordersmap.end() && j.key() == "itemsid") {
// qDebug() << "itemsid   is: " << j.value().toString() << Qt::endl;
isnew = false;
++j;

}








}

else {


customToast("appview-view20","Please sign in and ensure that you are not requesting items from your own business!");
timerdeletewidget->start();


}




});


anchoritems1aaa->clicked().connect([=] {



if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaaa.toUTF8())) {

//  qDebug() << "btnscope3a_ clicked()!" << Qt::endl;
itemsaaa->show();
items1aaa->hide();

QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

         auto size = is.tellg();
         char * str;
         std::string content = mypayload->fromIstream(is.read(str,size));
         idsid = QString::fromStdString(content);
          // construct string to stream size
        //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
          is.close();

}


if(ordersmap.count("itemsid") == 1) {

     isnew = true;
     ordersmap.remove("ids_id",idsid);
     ordersmap.remove("recipient",idsid);
     ordersmap.remove("deliverer",businessuuid);
     ordersmap.remove("locationto",reqlocationto);
     ordersmap.remove("locationfrom",reqlocationfrom);
     ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaaa.toUTF8()));


}

else {

  ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaaa.toUTF8()));



}


   /*  QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
     while (j != ordersmap.end() && j.key() == "itemsid") {
     qDebug() << "itemsid after remove is: " << j.value().toString() << Qt::endl;
      ++j;

        } */










}

else {


customToast("appview-view20","Only the owner of the business can do that!");
timerdeletewidget->start();


}




});






if(delitem1) {

allitems_->removeWidget(delitem1);
allitems_->bindEmpty("itemlinkaaa");
delitem1 = allitems_->bindWidget("itemlinkaaa", std::move(anchoritemsaaa));


}

else {


delitem1 = allitems_->bindWidget("itemlinkaaa", std::move(anchoritemsaaa));


}


if(delitem1a) {

allitems_->removeWidget(delitem1a);
allitems_->bindEmpty("itemlink1aaa");
delitem1a = allitems_->bindWidget("itemlink1aaa", std::move(anchoritems1aaa));


}

else {


delitem1a = allitems_->bindWidget("itemlink1aaa", std::move(anchoritems1aaa));



}





//qDebug() << "appview13aaa_ the count is: " << thecount << Qt::endl;



//thecount = carouselcount-rem;

thecount = thecount+1;


if(thecount==allrows) {

 thecount=0;

}


//qDebug() << "appview13aab_ the count is: " << thecount << Qt::endl;



auto appview13aab_ = std::make_unique<Wt::WTemplate>();
appview13aab_->setTemplateText("<img src='${imgsrc}' class='img-fluid' />");
Wt::cpp17::any datamedia2aab = carouselfilter->data(thecount,5);
Wt::WString mediaaab = Wt::cpp17::any_cast<Wt::WString>(datamedia2aab);
Wt::cpp17::any mydatastandardizedmediaaab  = carouselfilter->data(thecount,10);
Wt::WString datastandardizedmediaaab = Wt::cpp17::any_cast<Wt::WString>(mydatastandardizedmediaaab);
std::string mysrcaab = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaab.toUTF8();
Wt::cpp17::any datadesaab = carouselfilter->data(thecount,2);
Wt::WString desaab = Wt::cpp17::any_cast<Wt::WString>(datadesaab);
Wt::cpp17::any datanameaab = carouselfilter->data(thecount,6);
Wt::WString nameaab = Wt::cpp17::any_cast<Wt::WString>(datanameaab);
Wt::cpp17::any mydatausernameaab = carouselfilter->data(thecount,11);
Wt::WString datausernameaab = Wt::cpp17::any_cast<Wt::WString>(mydatausernameaab);
Wt::cpp17::any mydataitemuuidaab = carouselfilter->data(thecount,3);
Wt::WString dataitemuuidaab = Wt::cpp17::any_cast<Wt::WString>(mydataitemuuidaab);
std::string mydesaab = desaab.toUTF8();
std::string mynameaab = nameaab.toUTF8();
std::string mediasrcaab = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaab.toUTF8();
appview13aab_->bindString("imgsrc",mediasrcaab,Wt::TextFormat::Plain);
item2->addWidget(std::move(appview13aab_));


/* Wt::WLink linkitemsaab = Wt::WLink(Wt::LinkType::InternalPath, "/items/name/"+mynameaab);
std::unique_ptr<Wt::WAnchor> anchoritemsaab = std::make_unique<Wt::WAnchor>(linkitemsaab,"Request"); */
auto anchoritemsaab = std::make_unique<Wt::WPushButton>("Request");
anchoritemsaab->setStyleClass("btn btn-lg btn-outline-info btn-rounded");
auto itemsaab = anchoritemsaab.get();


auto anchoritems2aab = std::make_unique<Wt::WPushButton>("Cancel Request");
anchoritems2aab->setStyleClass("btn btn-lg btn-outline-danger btn-rounded");
anchoritems2aab->hide();
auto items2aab = anchoritems2aab.get();

if(loggeduserdue==userdue) {

anchoritemsaab->hide();
anchoritems2aab->hide();

}


else {

 anchoritemsaab->show();


 }

anchoritemsaab->clicked().connect([=] {


if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaab.toUTF8())) {

    itemsaab->hide();
    items2aab->show();
   //  qDebug() << "btnscope3_ clicked()!" << Qt::endl;
     if(timercarousel->isActive()) {

     // timercarousel->stop(); //eric ruin


     }




      QString idsid = "";
      std::string idsfile;
      idsfile = mypayload->TOKEN_DIRIDS;
      idsfile.append(loggeduserdue.toStdString());
      idsfile.append("/ids.txt");



      if(std::ifstream is{idsfile, std::ios_base::in}) {

          auto size = is.tellg();
          char * str;
          std::string content = mypayload->fromIstream(is.read(str,size));
          idsid = QString::fromStdString(content);
           // construct string to stream size
         //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
           is.close();

        }



if(ordersmap.count("itemsid")<1) {

isnew = true;
ordersmap.clear();

}




if(isnew) {

ordersmap.insert("ids_id",businessuuid);
ordersmap.insert("recipient",idsid);
ordersmap.insert("deliverer",businessuuid);
ordersmap.insert("locationto",reqlocationto);
ordersmap.insert("locationfrom",reqlocationfrom);
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaab.toUTF8()));
//  qDebug() << "isnew is: " << isnew << Qt::endl;



}

else {

//  qDebug() << "isnew is: " << isnew << Qt::endl;

 }

QList<QVariant> values = ordersmap.values("itemsid");
for (int i = 0; i < values.size(); ++i) {

if(values.at(i).toString()==QString::fromStdString(dataitemuuidaab.toUTF8())) {


inmap = true;

}

else {


inmap = false;

}

}


if(!inmap) {

//  qDebug()  << "Adding itemuuid in DS!" << Qt::endl;
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaab.toUTF8()));



}

else {

//   qDebug()  << "itemuuid already in DS!" << Qt::endl;

}


QList<QVariant> ids_idvalues = ordersmap.values("ids_id");
for (int i = 0; i < ids_idvalues.size(); ++i) {

//  qDebug()  << "ids_id QList<QVariant> is: " << ids_idvalues.at(i).toString()  << Qt::endl;
reqidids = ids_idvalues.at(i).toString();


}

QList<QVariant> recipientvalues = ordersmap.values("recipient");
for (int i = 0; i < recipientvalues.size(); ++i) {

// qDebug()  << "recipient QList<QVariant> is: " << recipientvalues.at(i).toString()  << Qt::endl;
reqrecipient = recipientvalues.at(i).toString();


}


QList<QVariant> deliverervalues = ordersmap.values("deliverer");
for (int i = 0; i < deliverervalues.size(); ++i) {

// qDebug()  << "deliverer QList<QVariant> is: " << deliverervalues.at(i).toString()  << Qt::endl;
reqdeliverer = deliverervalues.at(i).toString();


}


QList<QVariant> locationtovalues = ordersmap.values("locationto");
for (int i = 0; i < locationtovalues.size(); ++i) {

//  qDebug()  << "locationto QList<QVariant> is: " << locationtovalues.at(i).toString()  << Qt::endl;
reqlocationto = locationtovalues.at(i).toString();

}


QList<QVariant> locationfromvalues = ordersmap.values("locationfrom");
for (int i = 0; i < locationfromvalues.size(); ++i) {

//   qDebug()  << "locationfrom QList<QVariant> is: " << locationfromvalues.at(i).toString()  << Qt::endl;
reqlocationfrom = locationfromvalues.at(i).toString();


}

QList<QVariant> actualvalues = ordersmap.values("itemsid");
for (int i = 0; i < actualvalues.size(); ++i) {

//  qDebug()  << "itemsid QList<QVariant> is: " << actualvalues.at(i).toString()  << Qt::endl;


}




QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
while (j != ordersmap.end() && j.key() == "itemsid") {
// qDebug() << "itemsid   is: " << j.value().toString() << Qt::endl;
isnew = false;
++j;

}








}

else {


customToast("appview-view20","Please sign in and ensure that you are not requesting items from your own business!");
timerdeletewidget->start();


}


});

anchoritems2aab->clicked().connect([=] {


if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaab.toUTF8())) {

//  qDebug() << "btnscope3a_ clicked()!" << Qt::endl;
itemsaab->show();
items2aab->hide();

QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

         auto size = is.tellg();
         char * str;
         std::string content = mypayload->fromIstream(is.read(str,size));
         idsid = QString::fromStdString(content);
          // construct string to stream size
        //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
          is.close();

}


if(ordersmap.count("itemsid") == 1) {

     isnew = true;
     ordersmap.remove("ids_id",idsid);
     ordersmap.remove("recipient",idsid);
     ordersmap.remove("deliverer",businessuuid);
     ordersmap.remove("locationto",reqlocationto);
     ordersmap.remove("locationfrom",reqlocationfrom);
     ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaab.toUTF8()));


}

else {

  ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaab.toUTF8()));



}


   /*  QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
     while (j != ordersmap.end() && j.key() == "itemsid") {
     qDebug() << "itemsid after remove is: " << j.value().toString() << Qt::endl;
      ++j;

        } */










}

else {


customToast("appview-view20","Only the owner of the business can do that!");
timerdeletewidget->start();


}



});




if(delitem2) {

allitems_->removeWidget(delitem2);
allitems_->bindEmpty("itemlinkaab");
delitem2 = allitems_->bindWidget("itemlinkaab", std::move(anchoritemsaab));

}

else {


delitem2 = allitems_->bindWidget("itemlinkaab", std::move(anchoritemsaab));


}


if(delitem2b) {

allitems_->removeWidget(delitem2b);
allitems_->bindEmpty("itemlink2aab");
delitem2b = allitems_->bindWidget("itemlink2aab", std::move(anchoritems2aab));

}

else {


delitem2b = allitems_->bindWidget("itemlink2aab", std::move(anchoritems2aab));


}

/*thecount = (carouselcount-rem)+1;

if(thecount==allrows) {

 thecount=0;

}

else {

 thecount = (carouselcount-rem)+1;

}  */

thecount = thecount+1;


if(thecount==allrows) {

 thecount=0;

}


//qDebug() << "appview13aac_  the count is: " << thecount << Qt::endl;







auto appview13aac_ = std::make_unique<Wt::WTemplate>();
appview13aac_->setTemplateText("<img src='${imgsrc}' class='img-fluid' />");
Wt::cpp17::any datamedia2aac = carouselfilter->data(thecount,5);
Wt::WString mediaaac = Wt::cpp17::any_cast<Wt::WString>(datamedia2aac);
Wt::cpp17::any mydatastandardizedmediaaac  = carouselfilter->data(thecount,10);
Wt::WString datastandardizedmediaaac = Wt::cpp17::any_cast<Wt::WString>(mydatastandardizedmediaaac);
std::string mysrcaac = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaac.toUTF8();
Wt::cpp17::any datadesaac = carouselfilter->data(thecount,2);
Wt::WString desaac = Wt::cpp17::any_cast<Wt::WString>(datadesaac);
Wt::cpp17::any datanameaac = carouselfilter->data(thecount,6);
Wt::cpp17::any mydatausernameaac = carouselfilter->data(thecount,11);
Wt::WString datausernameaac = Wt::cpp17::any_cast<Wt::WString>(mydatausernameaac);
Wt::cpp17::any mydataitemuuidaac = carouselfilter->data(thecount,3);
Wt::WString dataitemuuidaac = Wt::cpp17::any_cast<Wt::WString>(mydataitemuuidaac);
Wt::WString nameaac = Wt::cpp17::any_cast<Wt::WString>(datanameaac);
std::string mydesaac = desaac.toUTF8();
std::string mynameaac = nameaac.toUTF8();
std::string mediasrcaac = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+mediaaac.toUTF8();
appview13aac_->bindString("imgsrc",mediasrcaac,Wt::TextFormat::Plain);
item3->addWidget(std::move(appview13aac_));




/*Wt::WLink linkitemsaac = Wt::WLink(Wt::LinkType::InternalPath, "/items/name/"+mynameaac);
std::unique_ptr<Wt::WAnchor> anchoritemsaac = std::make_unique<Wt::WAnchor>(linkitemsaac,"Request"); */
auto anchoritemsaac = std::make_unique<Wt::WPushButton>("Request");
anchoritemsaac->setStyleClass("btn btn-lg btn-outline-info btn-rounded");
auto ptraac = anchoritemsaac.get();

auto anchoritems3aac = std::make_unique<Wt::WPushButton>("Cancel Request");
anchoritems3aac->setStyleClass("btn btn-lg btn-outline-danger btn-rounded");
auto ptr3aac = anchoritems3aac.get();
anchoritems3aac->hide();

if(loggeduserdue==userdue) {

anchoritemsaac->hide();
anchoritems3aac->hide();

}


else {

 anchoritemsaac->show();


 }

anchoritemsaac->clicked().connect([=] {

if(loggeduserdue.isEmpty()) {

 if(appvieweric_->isVisible()) {

   if(accregister_)
        accregister_->hide();
     if(accretrieve_)
           accretrieve_->hide();
      if(accsetup_)
           accsetup_->show();

              appvieweric_->hide();


        }

else  {




        }


    }

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaac.toUTF8())) {

ptr3aac->show();
ptraac->hide();
//  qDebug() << "btnscope3_ clicked()!" << Qt::endl;
if(timercarousel->isActive()) {

 //timercarousel->stop(); //eric ruin


 }




QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

auto size = is.tellg();
char * str;
std::string content = mypayload->fromIstream(is.read(str,size));
idsid = QString::fromStdString(content);
// construct string to stream size
//qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
is.close();

}



if(ordersmap.count("itemsid")<1) {

isnew = true;
ordersmap.clear();

}




if(isnew) {

ordersmap.insert("ids_id",businessuuid);
ordersmap.insert("recipient",idsid);
ordersmap.insert("deliverer",businessuuid);
ordersmap.insert("locationto",reqlocationto);
ordersmap.insert("locationfrom",reqlocationfrom);
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaac.toUTF8()));
//  qDebug() << "isnew is: " << isnew << Qt::endl;



}

else {

//  qDebug() << "isnew is: " << isnew << Qt::endl;

}

QList<QVariant> values = ordersmap.values("itemsid");
for (int i = 0; i < values.size(); ++i) {

if(values.at(i).toString()==QString::fromStdString(dataitemuuidaac.toUTF8())) {


    inmap = true;

}

else {


    inmap = false;

    }

}


if(!inmap) {

//  qDebug()  << "Adding itemuuid in DS!" << Qt::endl;
ordersmap.insert("itemsid",QString::fromStdString(dataitemuuidaac.toUTF8()));


}

else {

    //   qDebug()  << "itemuuid already in DS!" << Qt::endl;

}


QList<QVariant> ids_idvalues = ordersmap.values("ids_id");
for (int i = 0; i < ids_idvalues.size(); ++i) {

    //  qDebug()  << "ids_id QList<QVariant> is: " << ids_idvalues.at(i).toString()  << Qt::endl;
    reqidids = ids_idvalues.at(i).toString();


}

QList<QVariant> recipientvalues = ordersmap.values("recipient");
for (int i = 0; i < recipientvalues.size(); ++i) {

    // qDebug()  << "recipient QList<QVariant> is: " << recipientvalues.at(i).toString()  << Qt::endl;
    reqrecipient = recipientvalues.at(i).toString();


}


QList<QVariant> deliverervalues = ordersmap.values("deliverer");
for (int i = 0; i < deliverervalues.size(); ++i) {

// qDebug()  << "deliverer QList<QVariant> is: " << deliverervalues.at(i).toString()  << Qt::endl;
reqdeliverer = deliverervalues.at(i).toString();


}


QList<QVariant> locationtovalues = ordersmap.values("locationto");
for (int i = 0; i < locationtovalues.size(); ++i) {

    //  qDebug()  << "locationto QList<QVariant> is: " << locationtovalues.at(i).toString()  << Qt::endl;
    reqlocationto = locationtovalues.at(i).toString();

}


QList<QVariant> locationfromvalues = ordersmap.values("locationfrom");
for (int i = 0; i < locationfromvalues.size(); ++i) {

    //   qDebug()  << "locationfrom QList<QVariant> is: " << locationfromvalues.at(i).toString()  << Qt::endl;
    reqlocationfrom = locationfromvalues.at(i).toString();


}

QList<QVariant> actualvalues = ordersmap.values("itemsid");
for (int i = 0; i < actualvalues.size(); ++i) {

    //  qDebug()  << "itemsid QList<QVariant> is: " << actualvalues.at(i).toString()  << Qt::endl;


    }



QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
while (j != ordersmap.end() && j.key() == "itemsid") {
    // qDebug() << "itemsid   is: " << j.value().toString() << Qt::endl;
    isnew = false;
    ++j;

    }








    }

 else {


    customToast("appview-view20","Please sign in and ensure that you are not requesting items from your own business!");
    timerdeletewidget->start();


    }





});

anchoritems3aac->clicked().connect([=] {


if(loggeduserdue.isEmpty()) {

    if(appvieweric_->isVisible()) {

        if(accregister_)
            accregister_->hide();
        if(accretrieve_)
            accretrieve_->hide();
        if(accsetup_)
           accsetup_->show();

          appvieweric_->hide();


    }

else  {




    }


}

else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausernameaac.toUTF8())) {

//  qDebug() << "btnscope3a_ clicked()!" << Qt::endl;
ptr3aac->hide();
ptraac->show();

QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

         auto size = is.tellg();
         char * str;
         std::string content = mypayload->fromIstream(is.read(str,size));
         idsid = QString::fromStdString(content);
          // construct string to stream size
        //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
          is.close();

}


if(ordersmap.count("itemsid") == 1) {

   isnew = true;
   ordersmap.remove("ids_id",idsid);
   ordersmap.remove("recipient",idsid);
   ordersmap.remove("deliverer",businessuuid);
   ordersmap.remove("locationto",reqlocationto);
   ordersmap.remove("locationfrom",reqlocationfrom);
   ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaac.toUTF8()));


}

else {

  ordersmap.remove("itemsid",QString::fromStdString(dataitemuuidaac.toUTF8()));



}


   /*  QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
     while (j != ordersmap.end() && j.key() == "itemsid") {
     qDebug() << "itemsid after remove is: " << j.value().toString() << Qt::endl;
      ++j;

        } */










}

else {


customToast("appview-view20","Only the owner of the business can do that!");
timerdeletewidget->start();


}




});




if(delitem3) {

allitems_->removeWidget(delitem3);
allitems_->bindEmpty("itemlinkaac");
delitem3 = allitems_->bindWidget("itemlinkaac", std::move(anchoritemsaac));

}

else {


delitem3 = allitems_->bindWidget("itemlinkaac", std::move(anchoritemsaac));


}

if(delitem3c) {

allitems_->removeWidget(delitem3c);
allitems_->bindEmpty("itemlink3aac");
delitem3c = allitems_->bindWidget("itemlink3aac", std::move(anchoritems3aac));

}

else {


delitem3c = allitems_->bindWidget("itemlink3aac", std::move(anchoritems3aac));


}



 std::string match1 = "https://www.youtube.com/embed";
 std::string match2 = "https://www.youtube.com/v";
 std::string spattern1 = "("+match1+")(.*)";
 std::string spattern2 = "("+match2+")(.*)";
 std::string myinputtxt1aaa = datastandardizedmediaaaa.toUTF8();


 auto appviewordinaryimage1aaa_ = std::make_unique<Wt::WTemplate>();
 auto strviewordinaryimage1aaa = Wt::WString::tr("ordinaryimagesaaa");
 appviewordinaryimage1aaa_->setTemplateText(strviewordinaryimage1aaa);
 appviewordinaryimage1aaa_->bindString("imgsrcaaa",mysrcaaa,Wt::TextFormat::Plain);


 auto appviewordinaryimage2aaa_ = std::make_unique<Wt::WTemplate>();
 auto strviewordinaryimage2aaa = Wt::WString::tr("ordinaryimagesaaa");
 appviewordinaryimage2aaa_->setTemplateText(strviewordinaryimage2aaa);
 appviewordinaryimage2aaa_->bindString("imgsrcaaa",mysrcaaa,Wt::TextFormat::Plain);
 auto appviewordinaryimage2aaa = appviewordinaryimage2aaa_.get();



 auto appviewembedyoutube1aaa_ = std::make_unique<Wt::WTemplate>();
 auto embedyoutube1aaa = Wt::WString::tr("youtubeembed1aaa");
 appviewembedyoutube1aaa_->setTemplateText(embedyoutube1aaa);
 appviewembedyoutube1aaa_->bindString("standardizedmediaaaa",datastandardizedmediaaaa.toUTF8(),Wt::TextFormat::Plain);
 auto appviewembedyoutube1aaa = appviewembedyoutube1aaa_.get();


 auto appviewembedyoutube2aaa_ = std::make_unique<Wt::WTemplate>();
 auto embedyoutube2aaa = Wt::WString::tr("youtubeembed2aaa");
 appviewembedyoutube2aaa_->setTemplateText(embedyoutube2aaa);
 appviewembedyoutube2aaa_->bindWidget("ordinaryimagesaaa", std::move(appviewordinaryimage2aaa_));
 appviewembedyoutube2aaa_->bindWidget("youtubeembed1aaa", std::move(appviewembedyoutube1aaa_));





 appviewordinaryimage2aaa->clicked().connect([=] {

 if((std::regex_match(myinputtxt1aaa, std::regex(spattern1)))) {

    // appviewembedyoutube1aaa->show();
    // appviewordinaryimage2aaa->hide();

   }

 else {



 }



 });



 auto flashcontaineraaa = std::make_unique<Wt::WContainerWidget>();
 auto flashaaa = flashcontaineraaa->addNew<Wt::WFlashObject>(datastandardizedmediaaaa.toUTF8());
 flashaaa->setFlashParameter("allowFullScreen", "true");
 // flashaaa->setFlashParameter("allowFullScreen", "yes");
 auto myposteraaa = std::make_unique<Wt::WImage>(Wt::WLink(mysrcaaa));
 myposteraaa->setStyleClass("img-fluid card-img-top");
 myposteraaa->setAttributeValue("width" ,"100%");
 myposteraaa->setAttributeValue("height" ,"100%");
 flashaaa->setAlternativeContent(std::move(myposteraaa));


 auto containervideoaaa = std::make_unique<Wt::WContainerWidget>();
 containervideoaaa->setStyleClass("card");
 auto videoplyaaa = containervideoaaa->addNew<Wt::WVideo>();
 videoplyaaa->addSource(Wt::WLink(mysrcaaa));
 auto myposter2aaa = std::make_unique<Wt::WImage>(Wt::WLink(datastandardizedmediaaaa.toUTF8()));
 myposter2aaa->setStyleClass("img-fluid card-img-top");
 myposter2aaa->setAttributeValue("width" ,"100%");
 myposter2aaa->setAttributeValue("height" ,"100%");
 videoplyaaa->setAlternativeContent(std::move(myposter2aaa));





 if(datastandardizedmediaaaa.toUTF8()=="later" || datastandardizedmediaaaa.toUTF8()=="null" || datastandardizedmediaaaa.toUTF8()=="nill" || datastandardizedmediaaaa.toUTF8()=="empty" || datastandardizedmediaaaa.toUTF8()=="N/A") {

  allitems_->bindWidget("mediasrcaaa", std::move(appviewordinaryimage1aaa_));

 }

 else if((std::regex_match(myinputtxt1aaa, std::regex(spattern1)))) {

 //  appviewembedyoutube1aaa->hide();
   appviewordinaryimage2aaa->hide();
 allitems_->bindWidget("mediasrcaaa", std::move(appviewembedyoutube2aaa_));



 }

 else if((std::regex_match(myinputtxt1aaa, std::regex(spattern2)))) {


  allitems_->bindWidget("mediasrcaaa", std::move(flashcontaineraaa));



 }

 else if(datastandardizedmediaaaa.toUTF8()=="localvideo" ) {

   allitems_->bindWidget("mediasrcaaa", std::move(containervideoaaa));

  }

 else {

  allitems_->bindWidget("mediasrcaaa", std::move(containervideoaaa));


 }



 std::string myinputtxt1aab = datastandardizedmediaaab.toUTF8();


 auto appviewordinaryimage1aab_ = std::make_unique<Wt::WTemplate>();
 auto strviewordinaryimage1aab = Wt::WString::tr("ordinaryimagesaab");
 appviewordinaryimage1aab_->setTemplateText(strviewordinaryimage1aab);
 appviewordinaryimage1aab_->bindString("imgsrcaab",mysrcaab,Wt::TextFormat::Plain);

 auto appviewordinaryimage2aab_ = std::make_unique<Wt::WTemplate>();
 auto strviewordinaryimage2aab = Wt::WString::tr("ordinaryimagesaab");
 appviewordinaryimage2aab_->setTemplateText(strviewordinaryimage2aab);
 appviewordinaryimage2aab_->bindString("imgsrcaab",mysrcaab,Wt::TextFormat::Plain);
 auto appviewordinaryimage2aab = appviewordinaryimage2aab_.get();



 auto appviewembedyoutube1aab_ = std::make_unique<Wt::WTemplate>();
 auto embedyoutube1aab = Wt::WString::tr("youtubeembed1aab");
 appviewembedyoutube1aab_->setTemplateText(embedyoutube1aab);
 appviewembedyoutube1aab_->bindString("standardizedmediaaab",datastandardizedmediaaab.toUTF8(),Wt::TextFormat::Plain);
 auto appviewembedyoutube1aab = appviewembedyoutube1aab_.get();


 auto appviewembedyoutube2aab_ = std::make_unique<Wt::WTemplate>();
 auto embedyoutube2aab = Wt::WString::tr("youtubeembed2aab");
 appviewembedyoutube2aab_->setTemplateText(embedyoutube2aab);
 appviewembedyoutube2aab_->bindWidget("ordinaryimagesaab", std::move(appviewordinaryimage2aab_));
 appviewembedyoutube2aab_->bindWidget("youtubeembed1aab", std::move(appviewembedyoutube1aab_));





 appviewordinaryimage2aab->clicked().connect([=] {

 if((std::regex_match(myinputtxt1aab, std::regex(spattern1)))) {

       // appviewembedyoutube1aab->show();
       // appviewordinaryimage2aab->hide();

      }

    else {



    }



 });


 auto flashcontaineraab = std::make_unique<Wt::WContainerWidget>();
 auto flashaab = flashcontaineraab->addNew<Wt::WFlashObject>(datastandardizedmediaaab.toUTF8());
 flashaab->setFlashParameter("allowFullScreen", "true");
 // flashaab->setFlashParameter("allowFullScreen", "yes");
 auto myposteraab = std::make_unique<Wt::WImage>(Wt::WLink(mysrcaab));
 myposteraab->setStyleClass("img-fluid card-img-top");
 myposteraab->setAttributeValue("width" ,"100%");
 myposteraab->setAttributeValue("height" ,"100%");
 flashaab->setAlternativeContent(std::move(myposteraab));



 auto containervideoaab = std::make_unique<Wt::WContainerWidget>();
 containervideoaab->setStyleClass("card");
 auto videoplyaab = containervideoaab->addNew<Wt::WVideo>();
 videoplyaab->addSource(Wt::WLink(mysrcaab));
 auto myposter2aab = std::make_unique<Wt::WImage>(Wt::WLink(datastandardizedmediaaab.toUTF8()));
 myposter2aab->setStyleClass("img-fluid card-img-top");
 myposter2aab->setAttributeValue("width" ,"100%");
 myposter2aab->setAttributeValue("height" ,"100%");
 videoplyaab->setAlternativeContent(std::move(myposter2aab));





 if(datastandardizedmediaaab.toUTF8()=="later" || datastandardizedmediaaab.toUTF8()=="null" || datastandardizedmediaaab.toUTF8()=="nill" || datastandardizedmediaaab.toUTF8()=="empty" || datastandardizedmediaaab.toUTF8()=="N/A") {

 allitems_->bindWidget("mediasrcaab", std::move(appviewordinaryimage1aab_));

 }

 else if((std::regex_match(myinputtxt1aab, std::regex(spattern1)))) {

 //  appviewembedyoutube1aab->hide();
 appviewordinaryimage2aab->hide();
 allitems_->bindWidget("mediasrcaab", std::move(appviewembedyoutube2aab_));



 }

 else if((std::regex_match(myinputtxt1aab, std::regex(spattern2)))) {


 allitems_->bindWidget("mediasrcaab", std::move(flashcontaineraab));


 }

 else if(datastandardizedmediaaab.toUTF8()=="localvideo" ) {

   allitems_->bindWidget("mediasrcaab", std::move(containervideoaab));

  }

 else {

  allitems_->bindWidget("mediasrcaab", std::move(containervideoaab));


 }



 std::string myinputtxt1aac = datastandardizedmediaaac.toUTF8();


 auto appviewordinaryimage1aac_ = std::make_unique<Wt::WTemplate>();
 auto strviewordinaryimage1aac = Wt::WString::tr("ordinaryimagesaac");
 appviewordinaryimage1aac_->setTemplateText(strviewordinaryimage1aac);
 appviewordinaryimage1aac_->bindString("imgsrcaac",mysrcaac,Wt::TextFormat::Plain);

 auto appviewordinaryimage2aac_ = std::make_unique<Wt::WTemplate>();
 auto strviewordinaryimage2aac = Wt::WString::tr("ordinaryimagesaac");
 appviewordinaryimage2aac_->setTemplateText(strviewordinaryimage2aac);
 appviewordinaryimage2aac_->bindString("imgsrcaac",mysrcaac,Wt::TextFormat::Plain);
 auto appviewordinaryimage2aac = appviewordinaryimage2aac_.get();



 auto appviewembedyoutube1aac_ = std::make_unique<Wt::WTemplate>();
 auto embedyoutube1aac = Wt::WString::tr("youtubeembed1aac");
 appviewembedyoutube1aac_->setTemplateText(embedyoutube1aac);
 appviewembedyoutube1aac_->bindString("standardizedmediaaac",datastandardizedmediaaac.toUTF8(),Wt::TextFormat::Plain);
 auto appviewembedyoutube1aac = appviewembedyoutube1aac_.get();


 auto appviewembedyoutube2aac_ = std::make_unique<Wt::WTemplate>();
 auto embedyoutube2aac = Wt::WString::tr("youtubeembed2aac");
 appviewembedyoutube2aac_->setTemplateText(embedyoutube2aac);
 appviewembedyoutube2aac_->bindWidget("ordinaryimagesaac", std::move(appviewordinaryimage2aac_));
 appviewembedyoutube2aac_->bindWidget("youtubeembed1aac", std::move(appviewembedyoutube1aac_));





 appviewordinaryimage2aac->clicked().connect([=] {

 if((std::regex_match(myinputtxt1aac, std::regex(spattern1)))) {

       // appviewembedyoutube1aac->show();
       // appviewordinaryimage2aac->hide();

      }

    else {



    }


 });


 auto flashcontaineraac = std::make_unique<Wt::WContainerWidget>();
 auto flashaac = flashcontaineraac->addNew<Wt::WFlashObject>(datastandardizedmediaaac.toUTF8());
 flashaac->setFlashParameter("allowFullScreen", "true");
 // flashaac->setFlashParameter("allowFullScreen", "yes");
 auto myposteraac = std::make_unique<Wt::WImage>(Wt::WLink(mysrcaac));
 myposteraac->setStyleClass("img-fluid card-img-top");
 myposteraac->setAttributeValue("width" ,"100%");
 myposteraac->setAttributeValue("height" ,"100%");
 flashaac->setAlternativeContent(std::move(myposteraac));


 auto containervideoaac = std::make_unique<Wt::WContainerWidget>();
 containervideoaac->setStyleClass("card");
 auto videoplyaac = containervideoaac->addNew<Wt::WVideo>();
 videoplyaac->addSource(Wt::WLink(mysrcaac));
 auto myposter2aac = std::make_unique<Wt::WImage>(Wt::WLink(datastandardizedmediaaac.toUTF8()));
 myposter2aac->setStyleClass("img-fluid card-img-top");
 myposter2aac->setAttributeValue("width" ,"100%");
 myposter2aac->setAttributeValue("height" ,"100%");
 videoplyaac->setAlternativeContent(std::move(myposter2aac));





 if(datastandardizedmediaaac.toUTF8()=="later" || datastandardizedmediaaac.toUTF8()=="null" || datastandardizedmediaaac.toUTF8()=="nill" || datastandardizedmediaaac.toUTF8()=="empty" || datastandardizedmediaaac.toUTF8()=="N/A") {

 allitems_->bindWidget("mediasrcaac", std::move(appviewordinaryimage1aac_));

 }

 else if((std::regex_match(myinputtxt1aac, std::regex(spattern1)))) {

 //  appviewembedyoutube1aac->hide();
 appviewordinaryimage2aac->hide();
 allitems_->bindWidget("mediasrcaac", std::move(appviewembedyoutube2aac_));



 }

 else if((std::regex_match(myinputtxt1aac, std::regex(spattern2)))) {


 allitems_->bindWidget("mediasrcaac", std::move(flashcontaineraac));


 }

 else if(datastandardizedmediaaac.toUTF8()=="localvideo" ) {

   allitems_->bindWidget("mediasrcaac", std::move(containervideoaac));

  }

 else {

  allitems_->bindWidget("mediasrcaac", std::move(containervideoaac));


 }




   /* allitems_->bindString("fav1",mediasrcaaa,Wt::TextFormat::Plain);
    allitems_->bindString("fav2",mediasrcaab,Wt::TextFormat::Plain);
    allitems_->bindString("fav3",mediasrcaac,Wt::TextFormat::Plain); */







   carousel->addWidget(std::move(item1));
   carousel->addWidget(std::move(item2));
   carousel->addWidget(std::move(item3));
   allitems_->bindString("fav1",mediasrcaaa,Wt::TextFormat::Plain);
   allitems_->bindString("fav2",mediasrcaab,Wt::TextFormat::Plain);
   allitems_->bindString("fav3",mediasrcaac,Wt::TextFormat::Plain);
   allitems_->bindString("des1",mydesaaa,Wt::TextFormat::Plain);
   allitems_->bindString("des2",mydesaab,Wt::TextFormat::Plain);
   allitems_->bindString("des3",mydesaac,Wt::TextFormat::Plain);
   allitems_->bindString("name1",mynameaaa,Wt::TextFormat::Plain);
   allitems_->bindString("name2",mynameaab,Wt::TextFormat::Plain);
   allitems_->bindString("name3",mynameaac,Wt::TextFormat::Plain);
 //  allitems_->bindWidget("favourite",std::move(carousel));





}

else {

 //qDebug()<< "Model is empty!" << Qt::endl;
 allitems_->bindEmpty("itemlink1aaa");
 allitems_->bindEmpty("itemlink2aab");
 allitems_->bindEmpty("itemlink3aac");
 allitems_->bindEmpty("mediasrcaaa");
 allitems_->bindEmpty("mediasrcaab");
 allitems_->bindEmpty("mediasrcaac");
 allitems_->bindEmpty("fav1");
 allitems_->bindEmpty("fav2");
 allitems_->bindEmpty("fav3");
 allitems_->bindEmpty("des1");
 allitems_->bindEmpty("des2");
 allitems_->bindEmpty("des3");
 allitems_->bindEmpty("name1");
 allitems_->bindEmpty("name2");
 allitems_->bindEmpty("name3");


}


} // end try



catch(std::exception & e) {

  std::cout << "exception from mycarousel is: " << e.what() << std::endl;

}





}




void MapperSoftware::clearregister() {

edituserregscope->setText("");
editemailregscope->setText("");
edithintregscope->setText("");
editphoneregscope->setText("");
editpassregscope->setText("");
editpassreenterscope->setText("");

}

void MapperSoftware::clearlogin() {

editusersetupscope->setText("");
editpasssetupscope->setText("");

}

void MapperSoftware::clearretrieve() {

edituserretrievescope->setText("");
edithintretrievescope->setText("");


}






void MapperSoftware::loginuser() {

std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(editusersetupscope->text().toUTF8());
tokenfile.append("/reason.txt");

std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(editusersetupscope->text().toUTF8());
idsfile.append("/ids.txt");

std::string statusfile;
statusfile = mypayload->TOKEN_DIRIDS;
statusfile.append(editusersetupscope->text().toUTF8());
statusfile.append("/status.txt");



if(mysuccessmessage->isEmpty()) {

 showToast(Wt::WString::tr("appview-view16"));
 timerdeletewidget->start();
 //   customToast("appview-view19","Hello Toast");

/*

 QString * str = myerrormessage.get();
 QString data = *str;
 //qDebug() << " myerrormessage is: " << data << Qt::endl;

*/

}

else {

showToast(Wt::WString::tr("appview-view15"));
timerdeletewidget->start();
QString * str = mysuccessmessage.get();
QString data = *str;






//qDebug() << "mysuccessmessage is: " << data << Qt::endl;
QString tokenlogin = QString::fromStdWString(RandomLinks(LOGINTOKEN));
QString myreason = mypayload->extracttoken(data,QString::fromStdString(editusersetupscope->text().toUTF8())
   ,tokenlogin);

QDir appPath(QString::fromStdString(mypayload->TOKEN_DIRIDS));

loggeduserdue=QString::fromStdString(editusersetupscope->text().toUTF8());
//userdue = QString::fromStdString(editusersetupscope->text().toUTF8());

if(timercarousel->isActive()) {

   timercarousel->stop();

  }

timergetdatauser->start();
networkutils->getuseritems(userdue);





if(!appPath.exists(QString::fromStdString(editusersetupscope->text().toUTF8()))) {

appPath.mkdir(QString::fromStdString(editusersetupscope->text().toUTF8()));
std::fstream fs(tokenfile, std::ios::in);

if(!fs.is_open())
  {
     fs.clear();
     fs.open(tokenfile, std::ios::out);
   //qDebug() << "creating tokenfile" << Qt::endl;
     fs.close();


     fs.clear();
     fs.open(idsfile, std::ios::out);
     fs.close();

     fs.clear();
     fs.open(statusfile, std::ios::out);
     fs.close();




     std::ofstream fout(tokenfile,std::ios::trunc);

     if(fout) {

         fout << myreason.toStdString();
         fout.close();


     }

     std::ofstream fidsout(idsfile,std::ios::trunc);

     if(fidsout) {



         mypayload->processRequest(data);
         QJsonObject myjsobobj = mypayload->m_requestJson.toObject();
         QString myidsid = myjsobobj.value("ids_id").toString();
       //qDebug() << "idids is: " << myidsid << Qt::endl;
         fidsout << myidsid.toStdString();
         fidsout.close();
         accsetup_->hide();
         accregister_->hide();
         accretrieve_->hide();
         appvieweric_->show();




     }


     std::ofstream statusout(statusfile,std::ios::trunc);

     if(statusout) {

         statusout << "in";
         statusout.close();


     }








  }


else {

 //qDebug() << "tokenfile already exist now opening it for writing" << Qt::endl;

  std::ofstream fout(tokenfile,std::ios::trunc);

  if(fout) {


      fout << myreason.toStdString();
      fout.close();


  }

  std::ofstream fidsout(idsfile,std::ios::trunc);

  if(fidsout) {



      mypayload->processRequest(data);
      QJsonObject myjsobobj = mypayload->m_requestJson.toObject();
      QString myidsid = myjsobobj.value("ids_id").toString();
    //qDebug() << "idids is: " << myidsid << Qt::endl;
      fidsout << myidsid.toStdString();
      fidsout.close();
      accsetup_->hide();
      accregister_->hide();
      accretrieve_->hide();
      appvieweric_->show();




  }

  std::ofstream statusout(statusfile,std::ios::trunc);

  if(statusout) {

      statusout << "in";
      statusout.close();


  }







 }


}

else {

//qDebug() << "directory already exists! " << Qt::endl;
 std::fstream fs(tokenfile, std::ios::in);

 if(!fs.is_open())
   {
      fs.clear();
      fs.open(tokenfile, std::ios::out);
     // //qDebug() << "creating tokenfile" << Qt::endl;
      fs.close();

      fs.clear();
      fs.open(idsfile, std::ios::out);
      fs.close();


      fs.clear();
      fs.open(statusfile, std::ios::out);
      fs.close();






      std::ofstream fout(tokenfile,std::ios::trunc);

      if(fout) {

          fout << myreason.toStdString();
          fout.close();


      }

      std::ofstream fidsout(idsfile,std::ios::trunc);

      if(fidsout) {



          mypayload->processRequest(data);
          QJsonObject myjsobobj = mypayload->m_requestJson.toObject();
          QString myidsid = myjsobobj.value("ids_id").toString();
        //qDebug() << "idids is: " << myidsid << Qt::endl;
          fidsout << myidsid.toStdString();
          fidsout.close();
          accsetup_->hide();
          accregister_->hide();
          accretrieve_->hide();
          appvieweric_->show();



      }

      std::ofstream statusout(statusfile,std::ios::trunc);

      if(statusout) {

          statusout << "in";
          statusout.close();


      }








   }


 else {

 //qDebug() << "tokenfile already exist now opening it for writing" << Qt::endl;

   std::ofstream fout(tokenfile,std::ios::trunc);

   if(fout) {


       fout << myreason.toStdString();
       fout.close();


   }


   std::ofstream fidsout(idsfile,std::ios::trunc);

   if(fidsout) {



       mypayload->processRequest(data);
       QJsonObject myjsobobj = mypayload->m_requestJson.toObject();
       QString myidsid = myjsobobj.value("ids_id").toString();
     //qDebug() << "idids is: " << myidsid << Qt::endl;
       fidsout << myidsid.toStdString();
       fidsout.close();
       accsetup_->hide();
       accregister_->hide();
       accretrieve_->hide();
       appvieweric_->show();




   }

   std::ofstream statusout(statusfile,std::ios::trunc);

   if(statusout) {

       statusout << "in";
       statusout.close();


   }






  }

}



signouticonscope1_->show();
signiniconscope1_->hide();
clearregister();
clearlogin();
clearretrieve();
menu->itemAt(0)->menu()->select(0);
menu->itemAt(0)->menu()->itemAt(1)->show();
menu->itemAt(0)->menu()->itemAt(2)->hide();
menu->itemAt(0)->menu()->itemAt(3)->hide();





}


}


void MapperSoftware::registeruser() {

std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(edituserregscope->text().toUTF8());
tokenfile.append("/reason.txt");


std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(edituserregscope->text().toUTF8());
idsfile.append("/ids.txt");

std::string statusfile;
statusfile = mypayload->TOKEN_DIRIDS;
statusfile.append(edituserregscope->text().toUTF8());
statusfile.append("/status.txt");

// std::cout << "tokenfile is: " << tokenfile << std::endl;
// std::cout << "idsfile is: " << idsfile << std::endl;
// std::cout << "statusfile is: " << statusfile << std::endl;



if(mysuccessmessage->isEmpty()) {

showToast(Wt::WString::tr("appview-view18"));
timerdeletewidget->start();

/*

QString * str = myerrormessage.get();
QString data = *str;
//qDebug() << " myerrormessage is: " << data << Qt::endl;

*/

    }

else {

showToast(Wt::WString::tr("appview-view17"));
timerdeletewidget->start();
QString * str = mysuccessmessage.get();
QString data = *str;

qDebug() << "mysuccessmessage is: " << data << Qt::endl;
QString tokenlogin = QString::fromStdWString(RandomLinks(REGISTERTOKEN));
QString myreason = mypayload->extracttoken(data,QString::fromStdString(edituserregscope->text().toUTF8())
       ,tokenlogin);

QDir appPath(QString::fromStdString(mypayload->TOKEN_DIRIDS));

loggeduserdue = QString::fromStdString(edituserregscope->text().toUTF8());
//userdue = QString::fromStdString(edituserregscope->text().toUTF8());

if(timercarousel->isActive()) {

   timercarousel->stop();

  }

timergetdatauser->start();
networkutils->getuseritems(userdue);




if(!appPath.exists(QString::fromStdString(edituserregscope->text().toUTF8()))) {

appPath.mkdir(QString::fromStdString(edituserregscope->text().toUTF8()));
std::fstream fs(tokenfile, std::ios::in);

if(!fs.is_open()) {
   fs.clear();
   fs.open(tokenfile, std::ios::out);
   //qDebug() << "creating tokenfile" << Qt::endl;
    fs.close();


    fs.clear();
    fs.open(idsfile, std::ios::out);
    fs.close();

    fs.clear();
    fs.open(statusfile, std::ios::out);
    fs.close();






  std::ofstream fout(tokenfile,std::ios::trunc);

   if(fout) {

  fout << myreason.toStdString();
   fout.close();


    }

   std::ofstream fidsout(idsfile,std::ios::trunc);

   if(fidsout) {



       mypayload->processRequest(data);
       QJsonObject myjsobobj = mypayload->m_requestJson.toObject();
       QString myidsid = myjsobobj.value("ids_id").toString();
     //qDebug() << "idids is: " << myidsid << Qt::endl;
       fidsout << myidsid.toStdString();
       fidsout.close();
       accregister_->hide();
       accsetup_->hide();
       accretrieve_->hide();
       appvieweric_->show();





   }

   std::ofstream statusout(statusfile,std::ios::trunc);

   if(statusout) {

       statusout << "in";
       statusout.close();


   }






  }


 else {

     //qDebug() << "tokenfile already exist now opening it for writing" << Qt::endl;

 std::ofstream fout(tokenfile,std::ios::trunc);

 if(fout) {


  fout << myreason.toStdString();
  fout.close();


    }

 std::ofstream fidsout(idsfile,std::ios::trunc);

 if(fidsout) {



     mypayload->processRequest(data);
     QJsonObject myjsobobj = mypayload->m_requestJson.toObject();
     QString myidsid = myjsobobj.value("ids_id").toString();
   //qDebug() << "idids is: " << myidsid << Qt::endl;
     fidsout << myidsid.toStdString();
     fidsout.close();
     accregister_->hide();
     accsetup_->hide();
     accretrieve_->hide();
     appvieweric_->show();



 }


 std::ofstream statusout(statusfile,std::ios::trunc);

 if(statusout) {

     statusout << "in";
     statusout.close();


 }








  }


    }

 else {

 //qDebug() << "directory already exists! " << Qt::endl;
 std::fstream fs(tokenfile, std::ios::in);

 if(!fs.is_open()) {
   fs.clear();
   fs.open(tokenfile, std::ios::out);
   //qDebug() << "creating tokenfile" << Qt::endl;
   fs.close();


   fs.clear();
   fs.open(idsfile, std::ios::out);
   fs.close();


   fs.clear();
   fs.open(statusfile, std::ios::out);
   fs.close();





 std::ofstream fout(tokenfile,std::ios::trunc);

  if(fout) {

  fout << myreason.toStdString();
  fout.close();


        }


  std::ofstream fidsout(idsfile,std::ios::trunc);

  if(fidsout) {



      mypayload->processRequest(data);
      QJsonObject myjsobobj = mypayload->m_requestJson.toObject();
      QString myidsid = myjsobobj.value("ids_id").toString();
    //qDebug() << "idids is: " << myidsid << Qt::endl;
      fidsout << myidsid.toStdString();
      fidsout.close();
      accregister_->hide();
      accsetup_->hide();
      accretrieve_->hide();
      appvieweric_->show();




  }

  std::ofstream statusout(statusfile,std::ios::trunc);

  if(statusout) {

      statusout << "in";
      statusout.close();


  }







       }


  else {

 //qDebug() << "tokenfile already exist now opening it for writing" << Qt::endl;

 std::ofstream fout(tokenfile,std::ios::trunc);

 if(fout) {


 fout << myreason.toStdString();
 fout.close();


    }

 std::ofstream fidsout(idsfile,std::ios::trunc);

 if(fidsout) {



     mypayload->processRequest(data);
     QJsonObject myjsobobj = mypayload->m_requestJson.toObject();
     QString myidsid = myjsobobj.value("ids_id").toString();
   //qDebug() << "idids is: " << myidsid << Qt::endl;
     fidsout << myidsid.toStdString();
     fidsout.close();
     accregister_->hide();
     accsetup_->hide();
     accretrieve_->hide();
     appvieweric_->show();



 }


 std::ofstream statusout(statusfile,std::ios::trunc);

 if(statusout) {

     statusout << "in";
     statusout.close();


 }









      }

    }


  signouticonscope1_->show();
  signiniconscope1_->hide();
  clearregister();
  clearlogin();
  clearretrieve();
  menu->itemAt(0)->menu()->select(0);
  menu->itemAt(0)->menu()->itemAt(1)->show();
  menu->itemAt(0)->menu()->itemAt(2)->hide();
  menu->itemAt(0)->menu()->itemAt(3)->hide();



   }

}


void MapperSoftware::retrievepassword() {

 if(mysuccessmessage->isEmpty()) {

  customToast("appview-view20","Incorrect credentials!");
  timerdeletewidget->start();


 }

 else {


 QString * str = mysuccessmessage.get();
 QString data = *str;
/* if(!timercarousel->isActive()) {

 timercarousel->start();

 timergetdatauser->start();
 networkutils->getuseritems(userdue);


 } */

 timergetdatauser->start();
 networkutils->getuseritems(userdue);

 mypayload->processRequest(data);
 QJsonObject myjsobobj = mypayload->m_requestJson.toObject();
 QString mypass = myjsobobj.value("password").toString();
 customToast("appview-view23","Your pin is "+mypass.toStdString());
 accretrieve_->hide();
 accsetup_->hide();
 accregister_->hide();
 appvieweric_->show();



   }



}

void MapperSoftware::thedataall() { // start thedataall()


}  // end thedataall()

void MapperSoftware::thedatauser() { // start thedatauser()


    if(mysuccessmessage->isEmpty()) {

    customToast("appview-view20","Slow network detected,please refresh the page!");
    timerdeletewidget->start();


     }

    else {


    QString * str = mysuccessmessage.get();
    QString data = *str;

   // qDebug() << "The data user is: " << data << Qt::endl;

   /* if(!(timercarousel->isActive())) {

       timercarousel->start();
    } */

    mycarousel();

    try {




    QVariantList mylist = myjsonutils->toVariantList(data);
    QJsonArray arr= QJsonArray::fromVariantList(mylist);
    QJsonDocument idsdoc = QJsonDocument();
    QJsonDocument catsdoc = QJsonDocument();
    QJsonDocument itemsdoc = QJsonDocument();
    QJsonDocument ordersdoc = QJsonDocument();
    QJsonDocument logisticsdoc = QJsonDocument();
    QJsonDocument deliverydoc = QJsonDocument();
    QJsonDocument metricsdoc = QJsonDocument();
    QJsonDocument locationsdoc = QJsonDocument();
    QJsonArray idsarr;
    QJsonArray catsarr;
    QJsonArray itemsarr;
    QJsonArray ordersarr;
    QJsonArray logisticsarr;
    QJsonArray deliveryarr;
    QJsonArray metricsarr;
    QJsonArray locationsarr;


    for (auto i = 0; i < arr.size(); ++i) {
     if (arr[i].isObject()) {

       auto dataObject = arr[i].toObject();
       idsdoc.setObject(dataObject);
     //  mypayload->modifyJsonValue(mydoc, "topkey["+QString::number(i)+"].somevalue");
       mypayload->modifyJsonValue(idsdoc, "list_catids");
       mypayload->modifyJsonValue(idsdoc, "list_deliveryids");
       mypayload->modifyJsonValue(idsdoc, "list_itemsids");
       mypayload->modifyJsonValue(idsdoc, "list_locationsids");
       mypayload->modifyJsonValue(idsdoc, "list_logisticsids");
       mypayload->modifyJsonValue(idsdoc, "list_metricsids");
       mypayload->modifyJsonValue(idsdoc, "list_ordersids");
       idsarr << idsdoc.object();





     }

    }

    QJsonDocument newdoc1(idsarr);
    QString mynewjson1 = newdoc1.toJson(QJsonDocument::JsonFormat::Indented);
    //   //qDebug() << "mynewjson1 is: " << qPrintable(mynewjson1) << Qt::endl;
    jsoncons::ojson j1 = jsoncons::ojson::parse(mynewjson1.toStdString());
    std::string output1;
    jsoncons::csv::csv_options ioptions1;
    ioptions1.quote_style(jsoncons::csv::quote_style_kind::nonnumeric);
    jsoncons::csv::encode_csv(j1, output1, ioptions1);
  //  std::cout << "ids csv output is: " << output1 << "\n\n";
    idsstandardmodel = csvModel(output1);

    for (auto i = 0; i < arr.size(); ++i) {  // start cats
      if (arr[i].isObject()) {

        auto dataObject = arr[i].toObject();
        QJsonValue myvalue = dataObject.value("list_catids");
        QJsonArray myarr = myvalue.toArray();
        for (auto i = 0; i < myarr.size(); ++i) {
          if (myarr[i].isObject()) {

          auto dataObjectmyarr = myarr[i].toObject();
          catsdoc.setObject(dataObjectmyarr);
          mypayload->modifyJsonValue(catsdoc, "ids_id");
          catsarr << catsdoc.object();



          }

        }





      }

    }  // end cats




    QJsonDocument newdoc2(catsarr);
    QString mynewjson2 = newdoc2.toJson(QJsonDocument::JsonFormat::Indented);
       //qDebug() << "mynewjson2 is: " << qPrintable(mynewjson2) << Qt::endl;
    jsoncons::ojson j2 = jsoncons::ojson::parse(mynewjson2.toStdString());
    std::string output2;
    jsoncons::csv::csv_options ioptions2;
    ioptions2.quote_style(jsoncons::csv::quote_style_kind::nonnumeric);
    jsoncons::csv::encode_csv(j2, output2, ioptions2);
    //std::cout << "csv output2 is: " << output2 << "\n\n";
    catsstandardmodel = csvModel(output2);




    for (auto i = 0; i < arr.size(); ++i) {  // start items
      if (arr[i].isObject()) {

        auto dataObject = arr[i].toObject();
        QJsonValue myvalue = dataObject.value("list_itemsids");
        QJsonArray myarr = myvalue.toArray();
        for (auto i = 0; i < myarr.size(); ++i) {
          if (myarr[i].isObject()) {

          auto dataObjectmyarr = myarr[i].toObject();
          itemsdoc.setObject(dataObjectmyarr);
          mypayload->modifyJsonValue(itemsdoc, "ids_id");
          itemsarr << itemsdoc.object();





          }

        }





      }

    }  // end items



    QJsonDocument newdoc3(itemsarr);
    QString mynewjson3 = newdoc3.toJson(QJsonDocument::JsonFormat::Indented);
      //qDebug() << "mynewjson3 is: " << qPrintable(mynewjson3) << Qt::endl;
    jsoncons::ojson j3 = jsoncons::ojson::parse(mynewjson3.toStdString());
    std::string output3;
    jsoncons::csv::csv_options ioptions3;
    ioptions3.quote_style(jsoncons::csv::quote_style_kind::nonnumeric);
    jsoncons::csv::encode_csv(j3, output3, ioptions3);
    // std::cout << "csv output3 is: " << output3 << "\n\n";
    itemsstandardmodel = csvModel(output3);




    for (auto i = 0; i < arr.size(); ++i) {  // start orders
      if (arr[i].isObject()) {

        auto dataObject = arr[i].toObject();
        QJsonValue myvalue = dataObject.value("list_ordersids");
        QJsonArray myarr = myvalue.toArray();
        for (auto i = 0; i < myarr.size(); ++i) {
          if (myarr[i].isObject()) {

          auto dataObjectmyarr = myarr[i].toObject();
          ordersdoc.setObject(dataObjectmyarr);
          mypayload->modifyJsonValue(ordersdoc, "ids_id");
          ordersarr << ordersdoc.object();





          }

        }





      }

    }  // end orders

    QJsonDocument newdoc4(ordersarr);
    QString mynewjson4 = newdoc4.toJson(QJsonDocument::JsonFormat::Indented);
    //qDebug() << "mynewjson4 is: " << qPrintable(mynewjson4) << Qt::endl;
    jsoncons::ojson j4 = jsoncons::ojson::parse(mynewjson4.toStdString());
    std::string output4;
    jsoncons::csv::csv_options ioptions4;
    ioptions4.quote_style(jsoncons::csv::quote_style_kind::nonnumeric);
    jsoncons::csv::encode_csv(j4, output4, ioptions4);
    //  std::cout << "csv output4 is: " << output4 << "\n\n";
    ordersstandardmodel = csvModel(output4);


    for (auto i = 0; i < arr.size(); ++i) {  // start logistics
      if (arr[i].isObject()) {

        auto dataObject = arr[i].toObject();
        QJsonValue myvalue = dataObject.value("list_logisticsids");
        QJsonArray myarr = myvalue.toArray();
        for (auto i = 0; i < myarr.size(); ++i) {
          if (myarr[i].isObject()) {

          auto dataObjectmyarr = myarr[i].toObject();
          logisticsdoc.setObject(dataObjectmyarr);
          mypayload->modifyJsonValue(logisticsdoc, "ids_id");
          logisticsarr << logisticsdoc.object();




          }

        }





      }

    }  // end logistics


    QJsonDocument newdoclogi(logisticsarr);
    QString mynewjsonlogi = newdoclogi.toJson(QJsonDocument::JsonFormat::Indented);
    //qDebug() << "mynewjsonlogi is: " << qPrintable(mynewjsonlogi) << Qt::endl;
    jsoncons::ojson jlogi = jsoncons::ojson::parse(mynewjsonlogi.toStdString());
    std::string outputlogi;
    jsoncons::csv::csv_options ioptionslogi;
    ioptionslogi.quote_style(jsoncons::csv::quote_style_kind::nonnumeric);
    jsoncons::csv::encode_csv(jlogi, outputlogi, ioptionslogi);
    //  std::cout << "csv outputlogi is: " << outputlogi << "\n\n";
    logisticsstandardmodel = csvModel(outputlogi);



    for (auto i = 0; i < arr.size(); ++i) {  // start delivery
      if (arr[i].isObject()) {

        auto dataObject = arr[i].toObject();
        QJsonValue myvalue = dataObject.value("list_deliveryids");
        QJsonArray myarr = myvalue.toArray();
        for (auto i = 0; i < myarr.size(); ++i) {
          if (myarr[i].isObject()) {

          auto dataObjectmyarr = myarr[i].toObject();
          deliverydoc.setObject(dataObjectmyarr);
          mypayload->modifyJsonValue(deliverydoc, "ids_id");
          deliveryarr << deliverydoc.object();




          }

        }





      }

    }  // end delivery


    QJsonDocument newdoc5(deliveryarr);
    QString mynewjson5 = newdoc5.toJson(QJsonDocument::JsonFormat::Indented);
     //qDebug() << "mynewjson5 is: " << qPrintable(mynewjson5) << Qt::endl;
    jsoncons::ojson j5 = jsoncons::ojson::parse(mynewjson5.toStdString());
    std::string output5;
    jsoncons::csv::csv_options ioptions5;
    ioptions5.quote_style(jsoncons::csv::quote_style_kind::nonnumeric);
    jsoncons::csv::encode_csv(j5, output5, ioptions5);
    //   std::cout << "csv output5 is: " << output5 << "\n\n";
    deliverystandardmodel = csvModel(output5);


    for (auto i = 0; i < arr.size(); ++i) {  // start metrics
      if (arr[i].isObject()) {

        auto dataObject = arr[i].toObject();
        QJsonValue myvalue = dataObject.value("list_metricsids");
        QJsonArray myarr = myvalue.toArray();
        for (auto i = 0; i < myarr.size(); ++i) {
          if (myarr[i].isObject()) {

          auto dataObjectmyarr = myarr[i].toObject();
          metricsdoc.setObject(dataObjectmyarr);
          mypayload->modifyJsonValue(metricsdoc, "ids_id");
          metricsarr << metricsdoc.object();




          }

        }





      }

    }  // end metrics


    QJsonDocument newdoc6(metricsarr);
    QString mynewjson6 = newdoc6.toJson(QJsonDocument::JsonFormat::Indented);
    //qDebug() << "mynewjson6 is: " << qPrintable(mynewjson6) << Qt::endl;
    jsoncons::ojson j6 = jsoncons::ojson::parse(mynewjson6.toStdString());
    std::string output6;
    jsoncons::csv::csv_options ioptions6;
    ioptions6.quote_style(jsoncons::csv::quote_style_kind::nonnumeric);
    jsoncons::csv::encode_csv(j6, output6, ioptions6);
    //   std::cout << "csv output6 is: " << output6 << "\n\n";
    metricsstandardmodel = csvModel(output6);




    for (auto i = 0; i < arr.size(); ++i) {  // start locations
      if (arr[i].isObject()) {

        auto dataObject = arr[i].toObject();
        QJsonValue myvalue = dataObject.value("list_locationsids");
        QJsonArray myarr = myvalue.toArray();
        for (auto i = 0; i < myarr.size(); ++i) {
          if (myarr[i].isObject()) {

          auto dataObjectmyarr = myarr[i].toObject();
          locationsdoc.setObject(dataObjectmyarr);
          mypayload->modifyJsonValue(locationsdoc, "ids_id");
          locationsarr << locationsdoc.object();




          }

        }





      }

    }  // end locations



    QJsonDocument newdoc8(locationsarr);
    QString mynewjson8 = newdoc8.toJson(QJsonDocument::JsonFormat::Indented);
    //qDebug() << "mynewjson8 is: " << qPrintable(mynewjson8) << Qt::endl;
    jsoncons::ojson j8 = jsoncons::ojson::parse(mynewjson8.toStdString());
    std::string output8;
    jsoncons::csv::csv_options ioptions8;
    ioptions8.quote_style(jsoncons::csv::quote_style_kind::nonnumeric);
    jsoncons::csv::encode_csv(j8, output8, ioptions8);
    //   std::cout << "csv output8 is: " << output8 << "\n\n";
    locationsstandardmodel = csvModel(output8);
    ordersfilter->setSourceModel(ordersstandardmodel);
    ordersfilter->setDynamicSortFilter(true);
    //  std::regex myreg1 {R"(\w{2}\s∗\d{5}(−\d{4})?)"};
    //  std::string itempattern = R"(^item.$)";
   // std::string myinputtxt = "2022-10-30";
   // std::string spattern = R"(()" + myinputtxt + R"()(.*))";
   // ordersfilter->setFilterRegExp(std::make_unique<std::regex>(spattern));
    // ordersfilter->setFilterRegExp(std::make_unique<std::regex>("(eri)(.*)"));
    ordersfilter->setFilterKeyColumn(0);
    ordersfilter->setFilterRole(Wt::ItemDataRole::Display);

    idsfilter->setSourceModel(idsstandardmodel);
    idsfilter->setDynamicSortFilter(true);
    //  std::regex myreg1 {R"(\w{2}\s∗\d{5}(−\d{4})?)"};
    //  std::string itempattern = R"(^item.$)";
    //std::string myinputtxt = "ericm";
    //std::string spattern = R"(()" + myinputtxt + R"()(.*))";
    //idsfilter->setFilterRegExp(std::make_unique<std::regex>(spattern));
    // idsfilter->setFilterRegExp(std::make_unique<std::regex>("(eri)(.*)"));
    idsfilter->setFilterKeyColumn(20);
    idsfilter->setFilterRole(Wt::ItemDataRole::Display);

    int catrows = catsstandardmodel->rowCount();
    int idsrow = idsfilter->rowCount();
    int ordersrow = ordersfilter->rowCount();

     if((idsstandardmodel->columnCount() && idsstandardmodel->rowCount())==0) {

      allitems_->bindEmpty("bizname");
      allitems_->bindEmpty("bizphone");



     }

     else {


         for (int data = 0; data < idsrow; ++data) {

          Wt::cpp17::any mydataidids = idsfilter->data(data,12);
          Wt::WString dataidids = Wt::cpp17::any_cast<Wt::WString>(mydataidids );
          Wt::cpp17::any mydataidphone = idsfilter->data(data,15);
          Wt::WString dataidphone = Wt::cpp17::any_cast<Wt::WString>(mydataidphone );
          Wt::cpp17::any mydataiddate = idsfilter->data(data,7);
          Wt::WString dataiddate = Wt::cpp17::any_cast<Wt::WString>(mydataiddate );
          Wt::cpp17::any mydataidsub = idsfilter->data(data,19);
          Wt::WString dataidsub = Wt::cpp17::any_cast<Wt::WString>(mydataidsub);
          Wt::cpp17::any mydataidblacklist = idsfilter->data(data,5);
          Wt::WString dataidblacklist = Wt::cpp17::any_cast<Wt::WString>(mydataidblacklist);
          Wt::cpp17::any mydataidbusiness = idsfilter->data(data,6);
          Wt::WString dataidbusiness = Wt::cpp17::any_cast<Wt::WString>(mydataidbusiness);
          Wt::cpp17::any mydataidresponsibiities = idsfilter->data(data,17);
          Wt::WString dataidresponsibiities = Wt::cpp17::any_cast<Wt::WString>(mydataidresponsibiities);
          Wt::cpp17::any mydataidauthority = idsfilter->data(data,3);
          Wt::WString dataidauthority = Wt::cpp17::any_cast<Wt::WString>(mydataidauthority);
          Wt::cpp17::any mydataidusername = idsfilter->data(data,20);
          Wt::WString dataidusername = Wt::cpp17::any_cast<Wt::WString>(mydataidusername);
          Wt::cpp17::any mydataidverification = idsfilter->data(data,21);
          Wt::WString dataidverification = Wt::cpp17::any_cast<Wt::WString>(mydataidverification);
          Wt::cpp17::any mydataidpolicy = idsfilter->data(data,16);
          Wt::WString dataidpolicy = Wt::cpp17::any_cast<Wt::WString>(mydataidpolicy);
          Wt::cpp17::any mydataidadname = idsfilter->data(data,2);
          Wt::WString dataidadname = Wt::cpp17::any_cast<Wt::WString>(mydataidadname);
          Wt::cpp17::any mydataidavatarname = idsfilter->data(data,4);
          Wt::WString dataidavatarname = Wt::cpp17::any_cast<Wt::WString>(mydataidavatarname);
          Wt::cpp17::any mydataididcardfront = idsfilter->data(data,11);
          Wt::WString dataididcardfront = Wt::cpp17::any_cast<Wt::WString>(mydataididcardfront);
          Wt::cpp17::any mydataididcardback = idsfilter->data(data,10);
          Wt::WString dataididcardback = Wt::cpp17::any_cast<Wt::WString>(mydataididcardback);
          Wt::cpp17::any mydataidemail = idsfilter->data(data,8);
          Wt::WString dataidemail = Wt::cpp17::any_cast<Wt::WString>(mydataidemail);
        /*  qDebug() << "dataidids is: " << QString::fromStdString(dataidids.toUTF8()) << Qt::endl;
          qDebug() << "dataidphone is: " << QString::fromStdString(dataidphone.toUTF8()) << Qt::endl;
          qDebug() << "dataiddate is: " << QString::fromStdString(dataiddate.toUTF8()) << Qt::endl;
          qDebug() << "dataidsub is: " << QString::fromStdString(dataidsub.toUTF8()) << Qt::endl;
          qDebug() << "dataidblacklist is: " << QString::fromStdString(dataidblacklist.toUTF8()) << Qt::endl;
          qDebug() << "dataidbusiness is: " << QString::fromStdString(dataidbusiness.toUTF8()) << Qt::endl;
          qDebug() << "dataidresponsibiities is: " << QString::fromStdString(dataidresponsibiities.toUTF8()) << Qt::endl;
          qDebug() << "dataidauthority is: " << QString::fromStdString(dataidauthority.toUTF8()) << Qt::endl;
          qDebug() << "dataidverification is: " << QString::fromStdString(dataidverification.toUTF8()) << Qt::endl;
          qDebug() << "dataidpolicy  is: " << QString::fromStdString(dataidpolicy.toUTF8()) << Qt::endl;
          qDebug() << "dataidadname  is: " << QString::fromStdString(dataidadname.toUTF8()) << Qt::endl;
          qDebug() << "dataidavatarname  is: " << QString::fromStdString(dataidavatarname.toUTF8()) << Qt::endl;
          qDebug() << "dataididcardfront  is: " << QString::fromStdString(dataididcardfront.toUTF8()) << Qt::endl;
          qDebug() << "dataididcardback  is: " << QString::fromStdString(dataididcardback.toUTF8()) << Qt::endl;
          qDebug() << "dataidemail  is: " << QString::fromStdString(dataidemail.toUTF8()) << Qt::endl;
          qDebug() << "dataidusername  is: " << QString::fromStdString(dataidusername.toUTF8()) << Qt::endl;
          */




          businessusername = QString::fromStdString(dataidusername.toUTF8());
          bizusername = QString::fromStdString(dataidusername.toUTF8());
          businessuuid = QString::fromStdString(dataidids.toUTF8());
          businessphonenumber = QString::fromStdString(dataidphone.toUTF8());
          businesssubscription = QString::fromStdString(dataidsub.toUTF8()).toInt();
          businessblacklist = QString::fromStdString(dataidblacklist.toUTF8()).toInt();
          businessresponsibilities = QString::fromStdString(dataidresponsibiities.toUTF8()).toInt();
          businesspolicy = QString::fromStdString(dataidpolicy.toUTF8()).toInt();
          businessavatar = QString::fromStdString(dataidavatarname.toUTF8());
          businessemail = QString::fromStdString(dataidemail.toUTF8());
          businessverification = QString::fromStdString(dataidverification.toUTF8()).toInt();
          businesswhichbiz = QString::fromStdString(dataidbusiness.toUTF8());

        /*  qDebug() << "businesssubscription is: " << businesssubscription << Qt::endl;
          qDebug() << "businessblacklist is: " << businessblacklist << Qt::endl;
          qDebug() << "businessresponsibilities is: " << businessresponsibilities << Qt::endl;
          qDebug() << "businesspolicy is: " << businesspolicy << Qt::endl;
          qDebug() << "businessverification is: " << businessverification << Qt::endl; */


         }


      //   businessusername.remove(QChar('_'), Qt::CaseInsensitive);
         allitems_->bindString("bizname",bizusername.replace("_"," ").toStdString(),Wt::TextFormat::Plain);
         allsettings_->bindString("bizname",bizusername.replace("_"," ").toStdString(),Wt::TextFormat::Plain);
         allitems_->bindString("bizphone",businessphonenumber.toStdString(),Wt::TextFormat::Plain);




     }


    if(catrows>0 && !(businesssubscription > 1 || businessblacklist > 0 || businessverification > 3) && (userdue==loggeduserdue)) {


        auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
        auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
        mycategories.clear();

        for (int data = 0; data < catrows; ++data) {

         Wt::cpp17::any catnameany = catsstandardmodel->data(data,2);
         Wt::WString catname = Wt::cpp17::any_cast<Wt::WString>(catnameany );
         Wt::cpp17::any catidany = catsstandardmodel->data(data,0);
         Wt::WString catid = Wt::cpp17::any_cast<Wt::WString>(catidany );
       //qDebug() << "catname is: " << QString::fromStdString(catname.toUTF8()) << Qt::endl;

     //    mycategories[catname.toUTF8()] = catname.toUTF8();

         std::vector<std::string> becats;
         becats.push_back(catname.toUTF8());

         mycategories[catname.toUTF8()] = becats;

         auto catview1_ = std::make_unique<Wt::WTemplate>();
         auto strcatview1 = Wt::WString::tr("uiux1-view5");
         catview1_->setTemplateText(strcatview1);
         auto deleteiconme = std::make_unique<Wt::WText>(deleteicon);
         deleteiconme->setStyleClass("ericcolors2");
         deleteiconme->clicked().connect([=] {

       //  qDebug() << "deleteiconme clicked()" << Qt::endl;
     //   qDebug() << "Category is: " << QString::fromStdString(catname.toUTF8()) << Qt::endl;
     //   qDebug() << "Category id is: " << QString::fromStdString(catid.toUTF8()) << Qt::endl;

         QVariantMap info;
         info.insert("user",loggeduserdue);
         info.insert("categoriesid",QString::fromStdString(catid.toUTF8()));


         timerdeletecategory->start();
         networkutils->deletecategory(info);




           });

         catview1_->bindString("categorydata",catname.toUTF8(),Wt::TextFormat::Plain);
         catview1_->bindWidget("actiondelete", std::move(deleteiconme));
         divinner1_->addWidget(std::move(catview1_));







        }

         divouter1_->addWidget(std::move(divinner1_));
         allsettings_->bindWidget("mycategoriesaction",std::move(divouter1_));






       {

        initModelnew();
        auto categoryCB = std::make_unique<Wt::WComboBox>();
        categoryCB->setModel(categoryModelnew());
        mytemplateformview_->updateViewValue(myformmodelnew_.get(), CategoryField,categoryCB.get());
        mytemplateformview_->updateModel(myformmodelnew_.get());
        mytemplateformview_->updateView(myformmodelnew_.get());


        }

        {

         initModelupdate();
         auto categoryCB = std::make_unique<Wt::WComboBox>();
         categoryCB->setModel(categoryModelupdate());
         updatetemplateformview_->updateViewValue(myformmodelupdate_.get(), CategoryField,categoryCB.get());
         updatetemplateformview_->updateModel(myformmodelupdate_.get());
         updatetemplateformview_->updateView(myformmodelupdate_.get());


         }










    }

    else {

        auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
        auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
        auto catview1_ = std::make_unique<Wt::WTemplate>();
        auto strcatview1 = Wt::WString::tr("uiux1-view5");
        catview1_->setTemplateText(strcatview1);
        catview1_->bindEmpty("categorydata");
        catview1_->bindEmpty("actiondelete");
        divinner1_->addWidget(std::move(catview1_));
        divouter1_->addWidget(std::move(divinner1_));
       // allsettings_->bindWidget("mycategoriesaction",std::move(divouter1_));
        allsettings_->bindEmpty("mycategoriesaction");
        mycategories.clear();


        {

         initModelnew();
         auto categoryCB = std::make_unique<Wt::WComboBox>();
         categoryCB->setModel(categoryModelnew());
         mytemplateformview_->updateViewValue(myformmodelnew_.get(), CategoryField,categoryCB.get());
         mytemplateformview_->updateModel(myformmodelnew_.get());
         mytemplateformview_->updateView(myformmodelnew_.get());


         }

         {

          initModelupdate();
          auto categoryCB = std::make_unique<Wt::WComboBox>();
          categoryCB->setModel(categoryModelupdate());
          updatetemplateformview_->updateViewValue(myformmodelupdate_.get(), CategoryField,categoryCB.get());
          updatetemplateformview_->updateModel(myformmodelupdate_.get());
          updatetemplateformview_->updateView(myformmodelupdate_.get());


          }





    }


if(ordersrow>0 && !(businesssubscription > 1 || businessblacklist > 0 || businessverification > 3)) {


auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
//requestsmap = QMultiMap<QString, QVariant>();
QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    idsid = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

for (int data = 0; data < ordersrow; ++data) {

Wt::cpp17::any mydatadeliverertest = ordersfilter->data(data,1);
Wt::WString datadeliverertest = Wt::cpp17::any_cast<Wt::WString>(mydatadeliverertest);

Wt::cpp17::any mydatarecipienttest = ordersfilter->data(data,6);
Wt::WString datarecipienttest = Wt::cpp17::any_cast<Wt::WString>(mydatarecipienttest);

if(datadeliverertest.toUTF8()==businessuuid.toStdString() && (loggeduserdue==businessusername)) {

Wt::cpp17::any mydatadate = ordersfilter->data(data,0);
Wt::WString datadate = Wt::cpp17::any_cast<Wt::WString>(mydatadate);
Wt::cpp17::any mydatadeliverer = ordersfilter->data(data,1);
Wt::WString datadeliverer = Wt::cpp17::any_cast<Wt::WString>(mydatadeliverer);
Wt::cpp17::any mydataitem = ordersfilter->data(data,2);
Wt::WString dataitem = Wt::cpp17::any_cast<Wt::WString>(mydataitem);
Wt::cpp17::any mydatalocationfrom = ordersfilter->data(data,3);
Wt::WString datalocationfrom = Wt::cpp17::any_cast<Wt::WString>(mydatalocationfrom);
Wt::cpp17::any mydatalocationto = ordersfilter->data(data,4);
Wt::WString datalocationto = Wt::cpp17::any_cast<Wt::WString>(mydatalocationto);
Wt::cpp17::any mydataordersid = ordersfilter->data(data,5);
Wt::WString dataordersid = Wt::cpp17::any_cast<Wt::WString>(mydataordersid);
Wt::cpp17::any mydatarecipient = ordersfilter->data(data,6);
Wt::WString datarecipient = Wt::cpp17::any_cast<Wt::WString>(mydatarecipient);
//qDebug() << "datadate orders is: " << QString::fromStdString(datadate.toUTF8()) << Qt::endl;

auto reqview1_ = std::make_unique<Wt::WTemplate>();
auto strreqview1 = Wt::WString::tr("uiux1-view7");
reqview1_->setTemplateText(strreqview1);
auto deleteiconme = std::make_unique<Wt::WText>(deleteicon);
deleteiconme->setStyleClass("ericcolors2");
deleteiconme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("orderid",QString::fromStdString(dataordersid.toUTF8()));
info.insert("recipientid",QString::fromStdString(datarecipient.toUTF8()));
info.insert("delivererid",QString::fromStdString(datadeliverer.toUTF8()));
timerdeleteorder->start();
networkutils->deleterequest(info);





});


if(loggeduserdue==userdue) {

 deleteiconme->hide();

}

else {

 deleteiconme->hide();

}

auto infoitemiconme = std::make_unique<Wt::WText>(infoiconc);
infoitemiconme->setStyleClass("ericcolors2");
infoitemiconme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("itemid",QString::fromStdString(dataitem.toUTF8()));
timergeneric2->start();
networkutils->queryitems(info);






});

auto infousericonme = std::make_unique<Wt::WText>(usericonc);
infousericonme->setStyleClass("ericcolors2");
infousericonme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("ids_id",QString::fromStdString(datarecipient.toUTF8()));
timergeneric1->start();
networkutils->userbyid(info);




});


QString ltme = mypayload->mylocaltime(QString::fromStdString(datadate.toUTF8()), "yyyy-MM-ddTHH:mm:ss.zzzZ");


reqview1_->bindString("reqid",dataordersid.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("custid",datarecipient.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindWidget("custinfo", std::move(infousericonme));
reqview1_->bindString("itemid",dataitem.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindWidget("iteminfo", std::move(infoitemiconme));
reqview1_->bindString("locationinfo",datalocationto.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("businessid",datadeliverer.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("datereq",ltme.toStdString(),Wt::TextFormat::Plain);
reqview1_->bindWidget("deleteaction", std::move(deleteiconme));
divinner1_->addWidget(std::move(reqview1_));






}

else if(QString::fromStdString(datarecipienttest.toUTF8())==idsid &&!(datadeliverertest.toUTF8()==businessuuid.toStdString() && (loggeduserdue==businessusername))) {


Wt::cpp17::any mydatadate = ordersfilter->data(data,0);
Wt::WString datadate = Wt::cpp17::any_cast<Wt::WString>(mydatadate);
Wt::cpp17::any mydatadeliverer = ordersfilter->data(data,1);
Wt::WString datadeliverer = Wt::cpp17::any_cast<Wt::WString>(mydatadeliverer);
Wt::cpp17::any mydataitem = ordersfilter->data(data,2);
Wt::WString dataitem = Wt::cpp17::any_cast<Wt::WString>(mydataitem);
Wt::cpp17::any mydatalocationfrom = ordersfilter->data(data,3);
Wt::WString datalocationfrom = Wt::cpp17::any_cast<Wt::WString>(mydatalocationfrom);
Wt::cpp17::any mydatalocationto = ordersfilter->data(data,4);
Wt::WString datalocationto = Wt::cpp17::any_cast<Wt::WString>(mydatalocationto);
Wt::cpp17::any mydataordersid = ordersfilter->data(data,5);
Wt::WString dataordersid = Wt::cpp17::any_cast<Wt::WString>(mydataordersid);
Wt::cpp17::any mydatarecipient = ordersfilter->data(data,6);
Wt::WString datarecipient = Wt::cpp17::any_cast<Wt::WString>(mydatarecipient);
/*requestsmap.insert("datadate",QString::fromStdString(datadate.toUTF8()));
requestsmap.insert("datadeliverer",QString::fromStdString(datadeliverer.toUTF8()));
requestsmap.insert("dataitem",QString::fromStdString(dataitem.toUTF8()));
requestsmap.insert("datalocationfrom",QString::fromStdString(datalocationfrom.toUTF8()));
requestsmap.insert("datalocationto",QString::fromStdString(datalocationto.toUTF8()));
requestsmap.insert("dataordersid",QString::fromStdString(dataordersid.toUTF8()));
requestsmap.insert("datarecipient",QString::fromStdString(datarecipient.toUTF8())); */

//qDebug() << "datadate orders is: " << QString::fromStdString(datadate.toUTF8()) << Qt::endl;

auto reqview1_ = std::make_unique<Wt::WTemplate>();
auto strreqview1 = Wt::WString::tr("uiux1-view7");
reqview1_->setTemplateText(strreqview1);
auto deleteiconme = std::make_unique<Wt::WText>(deleteicon);
deleteiconme->setStyleClass("ericcolors2");
deleteiconme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("orderid",QString::fromStdString(dataordersid.toUTF8()));
info.insert("recipientid",QString::fromStdString(datarecipient.toUTF8()));
info.insert("delivererid",QString::fromStdString(datadeliverer.toUTF8()));
timerdeleteorder->start();
networkutils->deleterequest(info);





});


if(loggeduserdue==userdue) {

 deleteiconme->hide();

}

else {

 //deleteiconme->hide();

}

auto infoitemiconme = std::make_unique<Wt::WText>(infoiconc);
infoitemiconme->setStyleClass("ericcolors2");
infoitemiconme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("itemid",QString::fromStdString(dataitem.toUTF8()));
timergeneric2->start();
networkutils->queryitems(info);






});

auto infousericonme = std::make_unique<Wt::WText>(usericonc);
infousericonme->setStyleClass("ericcolors2");
infousericonme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("ids_id",QString::fromStdString(datarecipient.toUTF8()));
timergeneric1->start();
networkutils->userbyid(info);




});


QString ltme = mypayload->mylocaltime(QString::fromStdString(datadate.toUTF8()), "yyyy-MM-ddTHH:mm:ss.zzzZ");


reqview1_->bindString("reqid",dataordersid.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("custid",datarecipient.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindWidget("custinfo", std::move(infousericonme));
reqview1_->bindString("itemid",dataitem.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("locationinfo",datalocationto.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindWidget("iteminfo", std::move(infoitemiconme));
reqview1_->bindString("businessid",datadeliverer.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("datereq",ltme.toStdString(),Wt::TextFormat::Plain);
reqview1_->bindWidget("deleteaction", std::move(deleteiconme));
divinner1_->addWidget(std::move(reqview1_));





}

else {


auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
auto reqview1_ = std::make_unique<Wt::WTemplate>();
auto strreqview1 = Wt::WString::tr("uiux1-view7");
reqview1_->setTemplateText(strreqview1);
reqview1_->bindEmpty("reqid");
reqview1_->bindEmpty("custid");
reqview1_->bindEmpty("custinfo");
reqview1_->bindEmpty("itemid");
reqview1_->bindEmpty("iteminfo");
reqview1_->bindEmpty("locationinfo");
reqview1_->bindEmpty("businessid");
reqview1_->bindEmpty("datereq");
reqview1_->bindEmpty("deleteaction");
divinner1_->addWidget(std::move(reqview1_));
divouter1_->addWidget(std::move(divinner1_));
allrequests_->bindEmpty("myrequestsaction");





}





  }

divouter1_->addWidget(std::move(divinner1_));
allrequests_->bindWidget("myrequestsaction",std::move(divouter1_));


    }

else {

auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
auto reqview1_ = std::make_unique<Wt::WTemplate>();
auto strreqview1 = Wt::WString::tr("uiux1-view7");
reqview1_->setTemplateText(strreqview1);
reqview1_->bindEmpty("reqid");
reqview1_->bindEmpty("custid");
reqview1_->bindEmpty("custinfo");
reqview1_->bindEmpty("itemid");
reqview1_->bindEmpty("iteminfo");
reqview1_->bindEmpty("locationinfo");
reqview1_->bindEmpty("businessid");
reqview1_->bindEmpty("datereq");
reqview1_->bindEmpty("deleteaction");
divinner1_->addWidget(std::move(reqview1_));
divouter1_->addWidget(std::move(divinner1_));
allrequests_->bindEmpty("myrequestsaction");






}




   if((itemsstandardmodel->rowCount())==0 && !(businesssubscription > 1 || businessblacklist > 0 || businessverification > 3)) {



   // qDebug() << "itemsstandardmodel is empty! " << Qt::endl;
    auto container = std::make_unique<Wt::WContainerWidget>();
    container->setStyleClass("album py-5 bg-light");
    auto div2_ = std::make_unique<Wt::WContainerWidget>();
    div2_->setStyleClass("container");
    auto div3_ = std::make_unique<Wt::WContainerWidget>();
    div3_->setStyleClass("row row-cols-1 row-cols-sm-2 row-cols-md-3 g-3");
    auto appview8_ = std::make_unique<Wt::WTemplate>();
    auto strview1 = Wt::WString::tr("appview-view8");
    appview8_->setTemplateText(strview1);
    appview8_->bindEmpty("smalltxt");
    appview8_->bindEmpty("para");
    appview8_->bindEmpty("imgsrc");
    appview8_->bindEmpty("standardized");
    appview8_->bindEmpty("quantity");
    appview8_->bindEmpty("category");
    appview8_->bindEmpty("standardizedmedia");
    appview8_->bindEmpty("itemname");
    appview8_->bindEmpty("username");
    appview8_->bindEmpty("mediasrc");
    appview8_->bindEmpty("manufacturer");
    appview8_ ->bindEmpty("myparagraph");
    appview8_ ->bindEmpty("mysmalltext");
    appview8_ ->bindEmpty("myimagetemplate");
    appview8_ ->bindEmpty("btn1");
    appview8_ ->bindEmpty("btn2");
    appview8_ ->bindEmpty("btn3");
    div3_->addWidget(std::move(appview8_));
    div2_->addWidget(std::move(div3_));
    container->addWidget(std::move(div2_));

    if(!(loggeduserdue=="")) {



   std::string myhelplink1 = "/helppage/loggedin";






    Wt::WLink linkhelpaaa1 = Wt::WLink(Wt::LinkType::InternalPath,  myhelplink1);
   // linkhelpaaa1.setTarget(Wt::LinkTarget::ThisWindow);
    helppageanchor1_ = nullptr;
    helppageanchor1_ = new Wt::WAnchor(linkhelpaaa1,"Visit the helppage");
    helppageanchor1_->clicked().connect([=] {

   //qDebug() << "helppageanchor1_ clicked()" << Qt::endl;




      });


    std::unique_ptr<Wt::WAnchor> helppageanchor1(helppageanchor1_);
    allitems_->bindWidget("myhelp1", std::move(helppageanchor1));



    if(loggeduserdue==userdue) {

        categoriesbtn_->show();
        execbtn->hide();
        locinfotemplateformview_->hide();


      }

    else {

        categoriesbtn_->hide();
        execbtn->show();
        locinfotemplateformview_->show();

     }

    }

   else {


  std::string myhelplink1 = "/helppage/loggedout";

  Wt::WLink linkhelpaaa1 = Wt::WLink(Wt::LinkType::InternalPath,  myhelplink1);
//  linkhelpaaa1.setTarget(Wt::LinkTarget::ThisWindow);
  helppageanchor1_ = nullptr;
  helppageanchor1_ = new Wt::WAnchor(linkhelpaaa1,"Visit the helppage");
  helppageanchor1_->clicked().connect([=] {

 // qDebug() << "helppageanchor1_ clicked()" << Qt::endl;




     });


  std::unique_ptr<Wt::WAnchor> helppageanchor1(helppageanchor1_);
  allitems_->bindWidget("myhelp1", std::move(helppageanchor1));

   if(loggeduserdue==userdue) {

       categoriesbtn_->show();
       execbtn->hide();
       locinfotemplateformview_->hide();

     }

   else {

       categoriesbtn_->hide();
       execbtn->show();
       locinfotemplateformview_->show();

    }

    }


    allitems_->bindEmpty("items");
    allitems_->bindEmpty("itemlinkaaa");
    allitems_->bindEmpty("itemlinkaab");
    allitems_->bindEmpty("itemlinkaac");
    allitems_->bindEmpty("fav1");
    allitems_->bindEmpty("fav2");
    allitems_->bindEmpty("fav3");
    allitems_->bindEmpty("des1");
    allitems_->bindEmpty("des2");
    allitems_->bindEmpty("des3");
    allitems_->bindEmpty("name1");
    allitems_->bindEmpty("name2");
    allitems_->bindEmpty("name3");
    removetpl(allitems_,"myspinner1");
    showCustomWidgetT("spinner1","myspinner1",allitems_);





     }
     else { // start items



         itemsfilter->setSourceModel(itemsstandardmodel);
         itemsfilter->setDynamicSortFilter(true);
         itemsfilter->setFilterKeyColumn(6);
        // itemsfilter->sort(1);
       //  std::regex myreg1 {R"(\w{2}\s∗\d{5}(−\d{4})?)"};
       //  std::string itempattern = R"(^item.$)";
       //  std::string myinputtxt = "ericm";
        // std::string spattern = R"(()" + myinputtxt + R"()(.*))";
        // itemsfilter->setFilterRegExp(std::make_unique<std::regex>(spattern));
         // itemsfilter->setFilterRegExp(std::make_unique<std::regex>("(eri)(.*)"));
         itemsfilter->setFilterRole(Wt::ItemDataRole::Display);

        //qDebug() << "column count is: " << itemsfilter->columnCount() << Qt::endl;
         //qDebug() << "row count is: " << itemsfilter->rowCount() << Qt::endl;
     //   Wt::WModelIndex  myindex = itemsfilter->index(0,5);



         auto container = std::make_unique<Wt::WContainerWidget>();
         container->setStyleClass("album py-5 bg-light");
         auto div2_ = std::make_unique<Wt::WContainerWidget>();
         div2_->setStyleClass("container");
         auto div3_ = std::make_unique<Wt::WContainerWidget>();
         div3_->setStyleClass("row row-cols-1 row-cols-sm-2 row-cols-md-3 g-3");




         int allrows = itemsfilter->rowCount();

         for (int card = 0; card < allrows; ++card) {

          auto appview8_ = std::make_unique<Wt::WTemplate>();
          auto strview1 = Wt::WString::tr("appview-view8");
          appview8_->setTemplateText(strview1);
          auto paragraphtext_ = std::make_unique<Wt::WTemplate>();
          auto str4 = Wt::WString::tr("appview-view4");
          paragraphtext_->setTemplateText(str4);
          Wt::cpp17::any mydatades = itemsfilter->data(card,2);
          Wt::WString datades = Wt::cpp17::any_cast<Wt::WString>(mydatades);
          Wt::cpp17::any mydataitemname = itemsfilter->data(card,6);
          Wt::WString dataitemname = Wt::cpp17::any_cast<Wt::WString>(mydataitemname);
          Wt::cpp17::any mydataprice = itemsfilter->data(card,7);
          Wt::WString dataprice = Wt::cpp17::any_cast<Wt::WString>(mydataprice);
          Wt::cpp17::any mydatasize = itemsfilter->data(card,8);
          Wt::WString datasize = Wt::cpp17::any_cast<Wt::WString>(mydatasize);
          Wt::cpp17::any mydatastandardized = itemsfilter->data(card,9);
          Wt::WString datastandardized = Wt::cpp17::any_cast<Wt::WString>(mydatastandardized);
          Wt::cpp17::any mydataquantity = itemsfilter->data(card,8);
          Wt::WString dataquantity = Wt::cpp17::any_cast<Wt::WString>(mydataquantity);
          Wt::cpp17::any mydatacategory = itemsfilter->data(card,0);
          Wt::WString datacategory = Wt::cpp17::any_cast<Wt::WString>(mydatacategory);
          Wt::cpp17::any mydatastandardizedmedia  = itemsfilter->data(card,10);
          Wt::WString datastandardizedmedia = Wt::cpp17::any_cast<Wt::WString>(mydatastandardizedmedia);
          Wt::cpp17::any mydatauuid  = itemsfilter->data(card,12);
          Wt::WString datauuid = Wt::cpp17::any_cast<Wt::WString>(mydatauuid);
          std::string paragraphtext = datades.toUTF8();
          Wt::cpp17::any mydatausername = itemsfilter->data(card,11);
          Wt::WString datausername = Wt::cpp17::any_cast<Wt::WString>(mydatausername);
       //   std::cout << "paragraphtext is: " << paragraphtext << std::endl;
          paragraphtext_->bindString("para",paragraphtext,Wt::TextFormat::Plain);
          auto smalltext_ = std::make_unique<Wt::WTemplate>();
          auto str5 = Wt::WString::tr("appview-view5");
          smalltext_->setTemplateText(str5);
          std::string smalltext = dataprice.toUTF8();
          auto imagetemplate_ = std::make_unique<Wt::WTemplate>();
          auto str6 = Wt::WString::tr("appview-view6");
          imagetemplate_->setTemplateText(str6);
          Wt::cpp17::any mydataitemuuid = itemsfilter->data(card,3);
          Wt::WString dataitemuuid = Wt::cpp17::any_cast<Wt::WString>(mydataitemuuid);
          Wt::cpp17::any mydatamanufacturer = itemsfilter->data(card,4);
          Wt::WString datamanufacturer = Wt::cpp17::any_cast<Wt::WString>(mydatamanufacturer);
          Wt::cpp17::any mydatamedia = itemsfilter->data(card,5);
          Wt::WString datamedia = Wt::cpp17::any_cast<Wt::WString>(mydatamedia);
          std::string mysrc = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+datamedia.toUTF8();
       //   std::cout << "mysrc is: " << mysrc << std::endl;
         // qInfo() << "media src: " << QString::fromStdString(mysrc) << Qt::endl;
          auto btnscope1_ = std::make_unique<Wt::WPushButton>("Update");
          btnscope1_->setStyleClass("btn btn-sm btn-outline-secondary");
          btnscope1_->clicked().connect([=] {

              //qDebug() << "mysrc is: " << QString::fromStdString(mysrc) << Qt::endl;
              //qDebug() << "card is: " << card << Qt::endl;

              if(loggeduserdue.isEmpty()) {

                  if(appvieweric_->isVisible()) {

                      if(accregister_)
                          accregister_->hide();
                      if(accretrieve_)
                          accretrieve_->hide();
                      if(accsetup_)
                         accsetup_->show();

                        appvieweric_->hide();


                  }

                  else {



                  }


              }

              else if(!loggeduserdue.isEmpty() && loggeduserdue.toStdString()==datausername.toUTF8()) {


                 //  qDebug() << "Proceed to Update an item... "  << Qt::endl;


                   doupdateitem = true;
                   retainmedianame = "";
                   retainmedianame = datamedia;
                   retainuniqueid = "";
                   retainuniqueid = dataitemuuid;
                   int index = combocategoryupdate->findText(datacategory);
                   combocategoryupdate->setCurrentIndex(index);
                   updateitemname->setText(dataitemname);
                   updatequantity->setText(dataquantity);
                   updateprice->setText(dataprice);
                   updateitemdescription->setText(datades);
                   updatestandardized->setText(datastandardized);
                   updatestandardizedmedia->setText(datastandardizedmedia);
                   updateitemuuid->setText(datauuid);
                   updatemanufacturer->setText(datamanufacturer);
                   //mytemplateformview_->updateModel(myformmodelnew_.get());
                  // mytemplateformview_->updateView(myformmodelnew_.get());
                   updatetemplateformview_->updateModel(myformmodelupdate_.get());
                   updatetemplateformview_->updateView(myformmodelupdate_.get());

                   menu->itemAt(0)->menu()->itemAt(1)->hide();
                   menu->itemAt(0)->menu()->itemAt(2)->show();
                   menu->itemAt(0)->menu()->itemAt(3)->hide();
                   menu->itemAt(0)->menu()->select(2);





              }


              else {


            customToast("appview-view20","Only the owner of the business can do that!");
            timerdeletewidget->start();


              }


          });

          auto btnscope2_ = std::make_unique<Wt::WPushButton>("Delete");
          btnscope2_->setStyleClass("btn btn-sm btn-outline-secondary");
          btnscope2_->clicked().connect([=] {





             if(loggeduserdue.isEmpty()) {

                 if(appvieweric_->isVisible()) {

                     if(accregister_)
                         accregister_->hide();
                     if(accretrieve_)
                         accretrieve_->hide();
                     if(accsetup_)
                        accsetup_->show();

                       appvieweric_->hide();


                 }

            else  {




                 }


             }

             else if(!loggeduserdue.isEmpty() && loggeduserdue.toStdString()==datausername.toUTF8()) {

              //   qDebug() << "Delete Step!" << Qt::endl;

                   deletemap = QVariantMap();
                   QVariantMap info;
                   info.insert("user",loggeduserdue);
                   info.insert("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));
                   info.insert("name",QString::fromStdString(dataitemname.toUTF8()));
                   info.insert("category",QString::fromStdString(datacategory.toUTF8()));

                   QString idsid = "";
                   std::string idsfile;
                   idsfile = mypayload->TOKEN_DIRIDS;
                   idsfile.append(loggeduserdue.toStdString());
                   idsfile.append("/ids.txt");



                   if(std::ifstream is{idsfile, std::ios_base::in}) {

                       auto size = is.tellg();
                       char * str;
                       std::string content = mypayload->fromIstream(is.read(str,size));
                       idsid = QString::fromStdString(content);
                        // construct string to stream size
                      //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
                        is.close();

                     }

                   info.insert("ids_id",idsid);
                   deletemap = info;


                 timerdeleteitem->start();
                 networkutils->deleteitem(deletemap);




             }

         else {


        customToast("appview-view20","Only the owner of the business can do that!");
        timerdeletewidget->start();


             }



          });






          auto btnscope3_ = std::make_unique<Wt::WPushButton>("Request Item");
          btnscope3_->setStyleClass("btn btn-outline-info btn-rounded");
          auto scope3_ = btnscope3_.get();
          auto btnscope3a_ = std::make_unique<Wt::WPushButton>("Cancel Request");
          btnscope3a_->setStyleClass("btn btn-outline-danger btn-rounded");
          btnscope3a_->hide();
          auto scope3a_ =  btnscope3a_.get();


          btnscope3_->clicked().connect([=] {




              if(loggeduserdue.isEmpty()) {

                  if(appvieweric_->isVisible()) {

                      if(accregister_)
                          accregister_->hide();
                      if(accretrieve_)
                          accretrieve_->hide();
                      if(accsetup_)
                         accsetup_->show();

                        appvieweric_->hide();


                  }

             else  {




                  }


              }

              else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausername.toUTF8())) {

                   scope3a_->show();
                   scope3_->hide();
                 //  qDebug() << "btnscope3_ clicked()!" << Qt::endl;
                   if(timercarousel->isActive()) {

                   // timercarousel->stop(); // eric ruin

                   }




                    QString idsid = "";
                    std::string idsfile;
                    idsfile = mypayload->TOKEN_DIRIDS;
                    idsfile.append(loggeduserdue.toStdString());
                    idsfile.append("/ids.txt");



                    if(std::ifstream is{idsfile, std::ios_base::in}) {

                        auto size = is.tellg();
                        char * str;
                        std::string content = mypayload->fromIstream(is.read(str,size));
                        idsid = QString::fromStdString(content);
                         // construct string to stream size
                       //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
                         is.close();

                      }



      if(ordersmap.count("itemsid")<1) {

      isnew = true;
      ordersmap.clear();

      }




         if(isnew) {

         ordersmap.insert("ids_id",businessuuid);
         ordersmap.insert("recipient",idsid);
         ordersmap.insert("deliverer",businessuuid);
         ordersmap.insert("locationto",reqlocationto);
         ordersmap.insert("locationfrom",reqlocationfrom);
         ordersmap.insert("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));
       //  qDebug() << "isnew is: " << isnew << Qt::endl;



              }

           else {

          //  qDebug() << "isnew is: " << isnew << Qt::endl;

               }

         QList<QVariant> values = ordersmap.values("itemsid");
         for (int i = 0; i < values.size(); ++i) {

          if(values.at(i).toString()==QString::fromStdString(dataitemuuid.toUTF8())) {


            inmap = true;

          }

          else {


          inmap = false;

          }

          }


       if(!inmap) {

      //  qDebug()  << "Adding itemuuid in DS!" << Qt::endl;
        ordersmap.insert("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));



         }

       else {

    //   qDebug()  << "itemuuid already in DS!" << Qt::endl;

       }


       QList<QVariant> ids_idvalues = ordersmap.values("ids_id");
       for (int i = 0; i < ids_idvalues.size(); ++i) {

      //  qDebug()  << "ids_id QList<QVariant> is: " << ids_idvalues.at(i).toString()  << Qt::endl;
        reqidids = ids_idvalues.at(i).toString();


        }

       QList<QVariant> recipientvalues = ordersmap.values("recipient");
       for (int i = 0; i < recipientvalues.size(); ++i) {

       // qDebug()  << "recipient QList<QVariant> is: " << recipientvalues.at(i).toString()  << Qt::endl;
        reqrecipient = recipientvalues.at(i).toString();


        }


       QList<QVariant> deliverervalues = ordersmap.values("deliverer");
       for (int i = 0; i < deliverervalues.size(); ++i) {

       // qDebug()  << "deliverer QList<QVariant> is: " << deliverervalues.at(i).toString()  << Qt::endl;
       reqdeliverer = deliverervalues.at(i).toString();


        }


       QList<QVariant> locationtovalues = ordersmap.values("locationto");
       for (int i = 0; i < locationtovalues.size(); ++i) {

      //  qDebug()  << "locationto QList<QVariant> is: " << locationtovalues.at(i).toString()  << Qt::endl;
        reqlocationto = locationtovalues.at(i).toString();

        }


       QList<QVariant> locationfromvalues = ordersmap.values("locationfrom");
       for (int i = 0; i < locationfromvalues.size(); ++i) {

     //   qDebug()  << "locationfrom QList<QVariant> is: " << locationfromvalues.at(i).toString()  << Qt::endl;
        reqlocationfrom = locationfromvalues.at(i).toString();


        }

       QList<QVariant> actualvalues = ordersmap.values("itemsid");
       for (int i = 0; i < actualvalues.size(); ++i) {

      //  qDebug()  << "itemsid QList<QVariant> is: " << actualvalues.at(i).toString()  << Qt::endl;


        }




       QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
       while (j != ordersmap.end() && j.key() == "itemsid") {
      // qDebug() << "itemsid   is: " << j.value().toString() << Qt::endl;
       isnew = false;
        ++j;

          }








              }

          else {


         customToast("appview-view20","Please sign in and ensure that you are not requesting items from your own business!");
         timerdeletewidget->start();


              }



          });




          btnscope3a_->clicked().connect([=] {




              if(loggeduserdue.isEmpty()) {

                  if(appvieweric_->isVisible()) {

                      if(accregister_)
                          accregister_->hide();
                      if(accretrieve_)
                          accretrieve_->hide();
                      if(accsetup_)
                         accsetup_->show();

                        appvieweric_->hide();


                  }

             else  {




                  }


              }

              else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausername.toUTF8())) {

                 //  qDebug() << "btnscope3a_ clicked()!" << Qt::endl;
                   scope3a_->hide();
                   scope3_->show();

                   QString idsid = "";
                   std::string idsfile;
                   idsfile = mypayload->TOKEN_DIRIDS;
                   idsfile.append(loggeduserdue.toStdString());
                   idsfile.append("/ids.txt");



                   if(std::ifstream is{idsfile, std::ios_base::in}) {

                       auto size = is.tellg();
                       char * str;
                       std::string content = mypayload->fromIstream(is.read(str,size));
                       idsid = QString::fromStdString(content);
                        // construct string to stream size
                      //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
                        is.close();

                     }


                   if(ordersmap.count("itemsid") == 1) {

                   isnew = true;
                   ordersmap.remove("ids_id",idsid);
                   ordersmap.remove("recipient",idsid);
                   ordersmap.remove("deliverer",businessuuid);
                   ordersmap.remove("locationto",reqlocationto);
                   ordersmap.remove("locationfrom",reqlocationfrom);
                   ordersmap.remove("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));


                   }

                   else {

                  ordersmap.remove("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));



                   }


                 /*  QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
                   while (j != ordersmap.end() && j.key() == "itemsid") {
                   qDebug() << "itemsid after remove is: " << j.value().toString() << Qt::endl;
                    ++j;

                      } */










              }

          else {


         customToast("appview-view20","Only the owner of the business can do that!");
         timerdeletewidget->start();


              }



          });


          if(loggeduserdue==userdue) {

              btnscope1_->show();
              btnscope2_->show();
              btnscope3_->hide();
              btnscope3a_->hide();


          }


          else {

              btnscope1_->hide();
              btnscope2_->hide();
              btnscope3_->show();

           }


          //  https://www.youtube.com/embed/rRoy6I4gKWU?autoplay=1
          // https://www.youtube.com/v/rRoy6I4gKWU?autoplay=1



          std::string match1 = "https://www.youtube.com/embed";
          std::string match2 = "https://www.youtube.com/v";
          std::string myinputtxt1 = datastandardizedmedia.toUTF8();
          std::string spattern1 = "("+match1+")(.*)";
          std::string spattern2 = "("+match2+")(.*)";


          auto appviewordinaryimage1_ = std::make_unique<Wt::WTemplate>();
          auto strviewordinaryimage1 = Wt::WString::tr("ordinaryimages");
          appviewordinaryimage1_->setTemplateText(strviewordinaryimage1);
          appviewordinaryimage1_->bindString("imgsrc",mysrc,Wt::TextFormat::Plain);


          auto appviewordinaryimage2_ = std::make_unique<Wt::WTemplate>();
          auto strviewordinaryimage2 = Wt::WString::tr("ordinaryimages");
          appviewordinaryimage2_->setTemplateText(strviewordinaryimage2);
          appviewordinaryimage2_->bindString("imgsrc",mysrc,Wt::TextFormat::Plain);
          auto appviewordinaryimage2 = appviewordinaryimage2_.get();



          auto appviewembedyoutube1_ = std::make_unique<Wt::WTemplate>();
          auto embedyoutube1 = Wt::WString::tr("youtubeembed1");
          appviewembedyoutube1_->setTemplateText(embedyoutube1);
          appviewembedyoutube1_->bindString("standardizedmedia",datastandardizedmedia.toUTF8(),Wt::TextFormat::Plain);
          auto appviewembedyoutube1 = appviewembedyoutube1_.get();


          auto appviewembedyoutube2_ = std::make_unique<Wt::WTemplate>();
          auto embedyoutube2 = Wt::WString::tr("youtubeembed2");
          appviewembedyoutube2_->setTemplateText(embedyoutube2);
          appviewembedyoutube2_->bindWidget("ordinaryimages", std::move(appviewordinaryimage2_));
          appviewembedyoutube2_->bindWidget("youtubeembed1", std::move(appviewembedyoutube1_));





          appviewordinaryimage2->clicked().connect([=] {

          if((std::regex_match(myinputtxt1, std::regex(spattern1)))) {

             // appviewembedyoutube1->show();
             // appviewordinaryimage2->hide();

            }

          else {



          }



          });



          auto flashcontainer = std::make_unique<Wt::WContainerWidget>();
          auto flash = flashcontainer->addNew<Wt::WFlashObject>(datastandardizedmedia.toUTF8());
          flash->setFlashParameter("allowFullScreen", "true");
         // flash->setFlashParameter("allowFullScreen", "yes");
          auto myposter1 = std::make_unique<Wt::WImage>(Wt::WLink(mysrc));
          myposter1->setStyleClass("img-fluid card-img-top");
          myposter1->setAttributeValue("width" ,"100%");
          myposter1->setAttributeValue("height" ,"100%");
          flash->setAlternativeContent(std::move(myposter1));


    /*      auto containerplayer = std::make_unique<Wt::WContainerWidget>();
         // containerplayer->setStyleClass("container card");
          Wt::WMediaPlayer *player1 = containerplayer->addNew<Wt::WMediaPlayer>(Wt::MediaType::Video);
          player1->setStyleClass("card");
        //  player1->setVideoSize(240,135);
          player1->addSource(Wt::MediaEncoding::M4V, Wt::WLink(mysrc));
        // player1->setControlsWidget(0); */



          auto containervideo = std::make_unique<Wt::WContainerWidget>();
          containervideo->setStyleClass("card");
          auto videoply = containervideo->addNew<Wt::WVideo>();
          videoply->addSource(Wt::WLink(mysrc));
          auto myposter2 = std::make_unique<Wt::WImage>(Wt::WLink(datastandardizedmedia.toUTF8()));
          myposter2->setStyleClass("img-fluid card-img-top");
          myposter2->setAttributeValue("width" ,"100%");
          myposter2->setAttributeValue("height" ,"100%");
          videoply->setAlternativeContent(std::move(myposter2));





          if(datastandardizedmedia.toUTF8()=="later" || datastandardizedmedia.toUTF8()=="null" || datastandardizedmedia.toUTF8()=="nill" || datastandardizedmedia.toUTF8()=="empty" || datastandardizedmedia.toUTF8()=="N/A") {

           appview8_->bindWidget("mediasrc", std::move(appviewordinaryimage1_));

          }

          else if((std::regex_match(myinputtxt1, std::regex(spattern1)))) {

        //  appviewembedyoutube1->hide();
            appviewordinaryimage2->hide();
          appview8_->bindWidget("mediasrc", std::move(appviewembedyoutube2_));



          }

          else if((std::regex_match(myinputtxt1, std::regex(spattern2)))) {


           appview8_->bindWidget("mediasrc", std::move(flashcontainer));



          }


          else if(datastandardizedmedia.toUTF8()=="localvideo") {

            //  appview8_->bindWidget("mediasrc", std::move(containerplayer));
               appview8_->bindWidget("mediasrc", std::move(containervideo));

             }



          else {

         appview8_->bindWidget("mediasrc", std::move(containervideo));

          }




          appview8_->bindString("smalltxt",smalltext,Wt::TextFormat::Plain);
          appview8_->bindString("para",paragraphtext,Wt::TextFormat::Plain);
          appview8_->bindString("standardized",datastandardized.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("quantity",dataquantity.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("category",datacategory.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("itemname",dataitemname.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("username",datausername.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("manufacturer",datamanufacturer.toUTF8(),Wt::TextFormat::Plain);
          appview8_ ->bindWidget("myparagraph", std::move(paragraphtext_));
          appview8_ ->bindWidget("mysmalltext", std::move(smalltext_));
          appview8_ ->bindWidget("myimagetemplate", std::move(imagetemplate_));
          appview8_ ->bindWidget("btn1", std::move(btnscope1_));
          appview8_ ->bindWidget("btn2", std::move(btnscope2_));
          appview8_ ->bindWidget("btn3", std::move(btnscope3_));
          appview8_ ->bindWidget("btn4", std::move(btnscope3a_));
          div3_->addWidget(std::move(appview8_));




             }



           div2_->addWidget(std::move(div3_));
           container->addWidget(std::move(div2_));



          allitems_->bindWidget("items",std::move(container));
          removetpl(allitems_,"myspinner1");


         if(!(loggeduserdue=="")) {



        std::string myhelplink1 = "/helppage/loggedin";






         Wt::WLink linkhelpaaa1 = Wt::WLink(Wt::LinkType::InternalPath,  myhelplink1);
        // linkhelpaaa1.setTarget(Wt::LinkTarget::ThisWindow);
         helppageanchor1_ = nullptr;
         helppageanchor1_ = new Wt::WAnchor(linkhelpaaa1,"Visit the helppage");
         helppageanchor1_->clicked().connect([=] {

        //qDebug() << "helppageanchor1_ clicked()" << Qt::endl;




           });


         std::unique_ptr<Wt::WAnchor> helppageanchor1(helppageanchor1_);
         allitems_->bindWidget("myhelp1", std::move(helppageanchor1));



         if(loggeduserdue==userdue) {

             categoriesbtn_->show();
             execbtn->hide();
             locinfotemplateformview_->hide();

           }

         else {

             categoriesbtn_->hide();
             execbtn->show();
             locinfotemplateformview_->show();

          }

         }

        else {


       std::string myhelplink1 = "/helppage/loggedout";

       Wt::WLink linkhelpaaa1 = Wt::WLink(Wt::LinkType::InternalPath,  myhelplink1);
     //  linkhelpaaa1.setTarget(Wt::LinkTarget::ThisWindow);
       helppageanchor1_ = nullptr;
       helppageanchor1_ = new Wt::WAnchor(linkhelpaaa1,"Visit the helppage");
       helppageanchor1_->clicked().connect([=] {

      // qDebug() << "helppageanchor1_ clicked()" << Qt::endl;




          });


       std::unique_ptr<Wt::WAnchor> helppageanchor1(helppageanchor1_);
       allitems_->bindWidget("myhelp1", std::move(helppageanchor1));

        if(loggeduserdue==userdue) {

            categoriesbtn_->show();
            execbtn->hide();
            locinfotemplateformview_->hide();

          }

        else {

            categoriesbtn_->hide();
            execbtn->show();
            locinfotemplateformview_->show();

         }

         }





          isstartup = false;






        }  // end items






    }

    catch(std::exception & e) {

      std::cout << "exception is: " << e.what() << std::endl;

    }










    }




}  // end thedatauser()


void MapperSoftware::thedatauserftr() { // start thedatauserftr()




    mycarouselftr();

    try {




    int catrows = catsstandardmodel->rowCount();
    int idsrow = idsfilter->rowCount();
    int ordersrow = ordersfilter->rowCount();

     if((idsstandardmodel->columnCount() && idsstandardmodel->rowCount())==0) {

      allitems_->bindEmpty("bizname");
      allitems_->bindEmpty("bizphone");



     }

     else {


         for (int data = 0; data < idsrow; ++data) {

          Wt::cpp17::any mydataidids = idsfilter->data(data,12);
          Wt::WString dataidids = Wt::cpp17::any_cast<Wt::WString>(mydataidids );
          Wt::cpp17::any mydataidphone = idsfilter->data(data,15);
          Wt::WString dataidphone = Wt::cpp17::any_cast<Wt::WString>(mydataidphone );
          Wt::cpp17::any mydataiddate = idsfilter->data(data,7);
          Wt::WString dataiddate = Wt::cpp17::any_cast<Wt::WString>(mydataiddate );
          Wt::cpp17::any mydataidsub = idsfilter->data(data,19);
          Wt::WString dataidsub = Wt::cpp17::any_cast<Wt::WString>(mydataidsub);
          Wt::cpp17::any mydataidblacklist = idsfilter->data(data,5);
          Wt::WString dataidblacklist = Wt::cpp17::any_cast<Wt::WString>(mydataidblacklist);
          Wt::cpp17::any mydataidbusiness = idsfilter->data(data,6);
          Wt::WString dataidbusiness = Wt::cpp17::any_cast<Wt::WString>(mydataidbusiness);
          Wt::cpp17::any mydataidresponsibiities = idsfilter->data(data,17);
          Wt::WString dataidresponsibiities = Wt::cpp17::any_cast<Wt::WString>(mydataidresponsibiities);
          Wt::cpp17::any mydataidauthority = idsfilter->data(data,3);
          Wt::WString dataidauthority = Wt::cpp17::any_cast<Wt::WString>(mydataidauthority);
          Wt::cpp17::any mydataidusername = idsfilter->data(data,20);
          Wt::WString dataidusername = Wt::cpp17::any_cast<Wt::WString>(mydataidusername);
          Wt::cpp17::any mydataidverification = idsfilter->data(data,21);
          Wt::WString dataidverification = Wt::cpp17::any_cast<Wt::WString>(mydataidverification);
          Wt::cpp17::any mydataidpolicy = idsfilter->data(data,16);
          Wt::WString dataidpolicy = Wt::cpp17::any_cast<Wt::WString>(mydataidpolicy);
          Wt::cpp17::any mydataidadname = idsfilter->data(data,2);
          Wt::WString dataidadname = Wt::cpp17::any_cast<Wt::WString>(mydataidadname);
          Wt::cpp17::any mydataidavatarname = idsfilter->data(data,4);
          Wt::WString dataidavatarname = Wt::cpp17::any_cast<Wt::WString>(mydataidavatarname);
          Wt::cpp17::any mydataididcardfront = idsfilter->data(data,11);
          Wt::WString dataididcardfront = Wt::cpp17::any_cast<Wt::WString>(mydataididcardfront);
          Wt::cpp17::any mydataididcardback = idsfilter->data(data,10);
          Wt::WString dataididcardback = Wt::cpp17::any_cast<Wt::WString>(mydataididcardback);
          Wt::cpp17::any mydataidemail = idsfilter->data(data,8);
          Wt::WString dataidemail = Wt::cpp17::any_cast<Wt::WString>(mydataidemail);
        /*  qDebug() << "dataidids is: " << QString::fromStdString(dataidids.toUTF8()) << Qt::endl;
          qDebug() << "dataidphone is: " << QString::fromStdString(dataidphone.toUTF8()) << Qt::endl;
          qDebug() << "dataiddate is: " << QString::fromStdString(dataiddate.toUTF8()) << Qt::endl;
          qDebug() << "dataidsub is: " << QString::fromStdString(dataidsub.toUTF8()) << Qt::endl;
          qDebug() << "dataidblacklist is: " << QString::fromStdString(dataidblacklist.toUTF8()) << Qt::endl;
          qDebug() << "dataidbusiness is: " << QString::fromStdString(dataidbusiness.toUTF8()) << Qt::endl;
          qDebug() << "dataidresponsibiities is: " << QString::fromStdString(dataidresponsibiities.toUTF8()) << Qt::endl;
          qDebug() << "dataidauthority is: " << QString::fromStdString(dataidauthority.toUTF8()) << Qt::endl;
          qDebug() << "dataidverification is: " << QString::fromStdString(dataidverification.toUTF8()) << Qt::endl;
          qDebug() << "dataidpolicy  is: " << QString::fromStdString(dataidpolicy.toUTF8()) << Qt::endl;
          qDebug() << "dataidadname  is: " << QString::fromStdString(dataidadname.toUTF8()) << Qt::endl;
          qDebug() << "dataidavatarname  is: " << QString::fromStdString(dataidavatarname.toUTF8()) << Qt::endl;
          qDebug() << "dataididcardfront  is: " << QString::fromStdString(dataididcardfront.toUTF8()) << Qt::endl;
          qDebug() << "dataididcardback  is: " << QString::fromStdString(dataididcardback.toUTF8()) << Qt::endl;
          qDebug() << "dataidemail  is: " << QString::fromStdString(dataidemail.toUTF8()) << Qt::endl;
          qDebug() << "dataidusername  is: " << QString::fromStdString(dataidusername.toUTF8()) << Qt::endl;
          */



          businessusername = QString::fromStdString(dataidusername.toUTF8());
          businessuuid = QString::fromStdString(dataidids.toUTF8());
          businessphonenumber = QString::fromStdString(dataidphone.toUTF8());
          businesssubscription = QString::fromStdString(dataidsub.toUTF8()).toInt();
          businessblacklist = QString::fromStdString(dataidblacklist.toUTF8()).toInt();
          businessresponsibilities = QString::fromStdString(dataidresponsibiities.toUTF8()).toInt();
          businesspolicy = QString::fromStdString(dataidpolicy.toUTF8()).toInt();
          businessavatar = QString::fromStdString(dataidavatarname.toUTF8());
          businessemail = QString::fromStdString(dataidemail.toUTF8());
          businessverification = QString::fromStdString(dataidverification.toUTF8()).toInt();
          businesswhichbiz = QString::fromStdString(dataidbusiness.toUTF8());

        /*  qDebug() << "businesssubscription is: " << businesssubscription << Qt::endl;
          qDebug() << "businessblacklist is: " << businessblacklist << Qt::endl;
          qDebug() << "businessresponsibilities is: " << businessresponsibilities << Qt::endl;
          qDebug() << "businesspolicy is: " << businesspolicy << Qt::endl;
          qDebug() << "businessverification is: " << businessverification << Qt::endl; */


         }


      //   businessusername.remove(QChar('_'), Qt::CaseInsensitive);
         allitems_->bindString("bizname",businessusername.replace("_"," ").toStdString(),Wt::TextFormat::Plain);
         allsettings_->bindString("bizname",businessusername.replace("_"," ").toStdString(),Wt::TextFormat::Plain);
         allitems_->bindString("bizphone",businessphonenumber.toStdString(),Wt::TextFormat::Plain);



     }


    if(catrows>0 && !(businesssubscription > 1 || businessblacklist > 0 || businessverification > 3) && (userdue==loggeduserdue)) {


        auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
        auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
        mycategories.clear();

        for (int data = 0; data < catrows; ++data) {

         Wt::cpp17::any catnameany = catsstandardmodel->data(data,2);
         Wt::WString catname = Wt::cpp17::any_cast<Wt::WString>(catnameany );
         Wt::cpp17::any catidany = catsstandardmodel->data(data,0);
         Wt::WString catid = Wt::cpp17::any_cast<Wt::WString>(catidany );
       //qDebug() << "catname is: " << QString::fromStdString(catname.toUTF8()) << Qt::endl;

     //    mycategories[catname.toUTF8()] = catname.toUTF8();

         std::vector<std::string> becats;
         becats.push_back(catname.toUTF8());

         mycategories[catname.toUTF8()] = becats;

         auto catview1_ = std::make_unique<Wt::WTemplate>();
         auto strcatview1 = Wt::WString::tr("uiux1-view5");
         catview1_->setTemplateText(strcatview1);
         auto deleteiconme = std::make_unique<Wt::WText>(deleteicon);
         deleteiconme->setStyleClass("ericcolors2");
         deleteiconme->clicked().connect([=] {

       //  qDebug() << "deleteiconme clicked()" << Qt::endl;
     //   qDebug() << "Category is: " << QString::fromStdString(catname.toUTF8()) << Qt::endl;
     //   qDebug() << "Category id is: " << QString::fromStdString(catid.toUTF8()) << Qt::endl;

         QVariantMap info;
         info.insert("user",loggeduserdue);
         info.insert("categoriesid",QString::fromStdString(catid.toUTF8()));


         timerdeletecategory->start();
         networkutils->deletecategory(info);




           });

         catview1_->bindString("categorydata",catname.toUTF8(),Wt::TextFormat::Plain);
         catview1_->bindWidget("actiondelete", std::move(deleteiconme));
         divinner1_->addWidget(std::move(catview1_));







        }

         divouter1_->addWidget(std::move(divinner1_));
         allsettings_->bindWidget("mycategoriesaction",std::move(divouter1_));






       {

        initModelnew();
        auto categoryCB = std::make_unique<Wt::WComboBox>();
        categoryCB->setModel(categoryModelnew());
        mytemplateformview_->updateViewValue(myformmodelnew_.get(), CategoryField,categoryCB.get());
        mytemplateformview_->updateModel(myformmodelnew_.get());
        mytemplateformview_->updateView(myformmodelnew_.get());


        }

        {

         initModelupdate();
         auto categoryCB = std::make_unique<Wt::WComboBox>();
         categoryCB->setModel(categoryModelupdate());
         updatetemplateformview_->updateViewValue(myformmodelupdate_.get(), CategoryField,categoryCB.get());
         updatetemplateformview_->updateModel(myformmodelupdate_.get());
         updatetemplateformview_->updateView(myformmodelupdate_.get());


         }










    }

    else {

        auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
        auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
        auto catview1_ = std::make_unique<Wt::WTemplate>();
        auto strcatview1 = Wt::WString::tr("uiux1-view5");
        catview1_->setTemplateText(strcatview1);
        catview1_->bindEmpty("categorydata");
        catview1_->bindEmpty("actiondelete");
        divinner1_->addWidget(std::move(catview1_));
        divouter1_->addWidget(std::move(divinner1_));
       // allsettings_->bindWidget("mycategoriesaction",std::move(divouter1_));
        allsettings_->bindEmpty("mycategoriesaction");
        mycategories.clear();


        {

         initModelnew();
         auto categoryCB = std::make_unique<Wt::WComboBox>();
         categoryCB->setModel(categoryModelnew());
         mytemplateformview_->updateViewValue(myformmodelnew_.get(), CategoryField,categoryCB.get());
         mytemplateformview_->updateModel(myformmodelnew_.get());
         mytemplateformview_->updateView(myformmodelnew_.get());


         }

         {

          initModelupdate();
          auto categoryCB = std::make_unique<Wt::WComboBox>();
          categoryCB->setModel(categoryModelupdate());
          updatetemplateformview_->updateViewValue(myformmodelupdate_.get(), CategoryField,categoryCB.get());
          updatetemplateformview_->updateModel(myformmodelupdate_.get());
          updatetemplateformview_->updateView(myformmodelupdate_.get());


          }





    }


if(ordersrow>0 && !(businesssubscription > 1 || businessblacklist > 0 || businessverification > 3)) {


auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
//requestsmap = QMultiMap<QString, QVariant>();
QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    idsid = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

for (int data = 0; data < ordersrow; ++data) {

Wt::cpp17::any mydatadeliverertest = ordersfilter->data(data,1);
Wt::WString datadeliverertest = Wt::cpp17::any_cast<Wt::WString>(mydatadeliverertest);

Wt::cpp17::any mydatarecipienttest = ordersfilter->data(data,6);
Wt::WString datarecipienttest = Wt::cpp17::any_cast<Wt::WString>(mydatarecipienttest);

if(datadeliverertest.toUTF8()==businessuuid.toStdString() && (loggeduserdue==businessusername)) {

Wt::cpp17::any mydatadate = ordersfilter->data(data,0);
Wt::WString datadate = Wt::cpp17::any_cast<Wt::WString>(mydatadate);
Wt::cpp17::any mydatadeliverer = ordersfilter->data(data,1);
Wt::WString datadeliverer = Wt::cpp17::any_cast<Wt::WString>(mydatadeliverer);
Wt::cpp17::any mydataitem = ordersfilter->data(data,2);
Wt::WString dataitem = Wt::cpp17::any_cast<Wt::WString>(mydataitem);
Wt::cpp17::any mydatalocationfrom = ordersfilter->data(data,3);
Wt::WString datalocationfrom = Wt::cpp17::any_cast<Wt::WString>(mydatalocationfrom);
Wt::cpp17::any mydatalocationto = ordersfilter->data(data,4);
Wt::WString datalocationto = Wt::cpp17::any_cast<Wt::WString>(mydatalocationto);
Wt::cpp17::any mydataordersid = ordersfilter->data(data,5);
Wt::WString dataordersid = Wt::cpp17::any_cast<Wt::WString>(mydataordersid);
Wt::cpp17::any mydatarecipient = ordersfilter->data(data,6);
Wt::WString datarecipient = Wt::cpp17::any_cast<Wt::WString>(mydatarecipient);
//qDebug() << "datadate orders is: " << QString::fromStdString(datadate.toUTF8()) << Qt::endl;

auto reqview1_ = std::make_unique<Wt::WTemplate>();
auto strreqview1 = Wt::WString::tr("uiux1-view7");
reqview1_->setTemplateText(strreqview1);
auto deleteiconme = std::make_unique<Wt::WText>(deleteicon);
deleteiconme->setStyleClass("ericcolors2");
deleteiconme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("orderid",QString::fromStdString(dataordersid.toUTF8()));
info.insert("recipientid",QString::fromStdString(datarecipient.toUTF8()));
info.insert("delivererid",QString::fromStdString(datadeliverer.toUTF8()));
timerdeleteorder->start();
networkutils->deleterequest(info);





});

auto infoitemiconme = std::make_unique<Wt::WText>(infoiconc);
infoitemiconme->setStyleClass("ericcolors2");
infoitemiconme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("itemid",QString::fromStdString(dataitem.toUTF8()));
timergeneric2->start();
networkutils->queryitems(info);






});

auto infousericonme = std::make_unique<Wt::WText>(usericonc);
infousericonme->setStyleClass("ericcolors2");
infousericonme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("ids_id",QString::fromStdString(datarecipient.toUTF8()));
timergeneric1->start();
networkutils->userbyid(info);




});


QString ltme = mypayload->mylocaltime(QString::fromStdString(datadate.toUTF8()), "yyyy-MM-ddTHH:mm:ss.zzzZ");


reqview1_->bindString("reqid",dataordersid.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("custid",datarecipient.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindWidget("custinfo", std::move(infousericonme));
reqview1_->bindString("itemid",dataitem.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindWidget("iteminfo", std::move(infoitemiconme));
reqview1_->bindString("locationinfo",datalocationto.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("businessid",datadeliverer.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("datereq",ltme.toStdString(),Wt::TextFormat::Plain);
reqview1_->bindWidget("deleteaction", std::move(deleteiconme));
divinner1_->addWidget(std::move(reqview1_));






}

else if(QString::fromStdString(datarecipienttest.toUTF8())==idsid &&!(datadeliverertest.toUTF8()==businessuuid.toStdString() && (loggeduserdue==businessusername))) {


Wt::cpp17::any mydatadate = ordersfilter->data(data,0);
Wt::WString datadate = Wt::cpp17::any_cast<Wt::WString>(mydatadate);
Wt::cpp17::any mydatadeliverer = ordersfilter->data(data,1);
Wt::WString datadeliverer = Wt::cpp17::any_cast<Wt::WString>(mydatadeliverer);
Wt::cpp17::any mydataitem = ordersfilter->data(data,2);
Wt::WString dataitem = Wt::cpp17::any_cast<Wt::WString>(mydataitem);
Wt::cpp17::any mydatalocationfrom = ordersfilter->data(data,3);
Wt::WString datalocationfrom = Wt::cpp17::any_cast<Wt::WString>(mydatalocationfrom);
Wt::cpp17::any mydatalocationto = ordersfilter->data(data,4);
Wt::WString datalocationto = Wt::cpp17::any_cast<Wt::WString>(mydatalocationto);
Wt::cpp17::any mydataordersid = ordersfilter->data(data,5);
Wt::WString dataordersid = Wt::cpp17::any_cast<Wt::WString>(mydataordersid);
Wt::cpp17::any mydatarecipient = ordersfilter->data(data,6);
Wt::WString datarecipient = Wt::cpp17::any_cast<Wt::WString>(mydatarecipient);
/*requestsmap.insert("datadate",QString::fromStdString(datadate.toUTF8()));
requestsmap.insert("datadeliverer",QString::fromStdString(datadeliverer.toUTF8()));
requestsmap.insert("dataitem",QString::fromStdString(dataitem.toUTF8()));
requestsmap.insert("datalocationfrom",QString::fromStdString(datalocationfrom.toUTF8()));
requestsmap.insert("datalocationto",QString::fromStdString(datalocationto.toUTF8()));
requestsmap.insert("dataordersid",QString::fromStdString(dataordersid.toUTF8()));
requestsmap.insert("datarecipient",QString::fromStdString(datarecipient.toUTF8())); */

//qDebug() << "datadate orders is: " << QString::fromStdString(datadate.toUTF8()) << Qt::endl;

auto reqview1_ = std::make_unique<Wt::WTemplate>();
auto strreqview1 = Wt::WString::tr("uiux1-view7");
reqview1_->setTemplateText(strreqview1);
auto deleteiconme = std::make_unique<Wt::WText>(deleteicon);
deleteiconme->setStyleClass("ericcolors2");
deleteiconme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("orderid",QString::fromStdString(dataordersid.toUTF8()));
info.insert("recipientid",QString::fromStdString(datarecipient.toUTF8()));
info.insert("delivererid",QString::fromStdString(datadeliverer.toUTF8()));
timerdeleteorder->start();
networkutils->deleterequest(info);





});

auto infoitemiconme = std::make_unique<Wt::WText>(infoiconc);
infoitemiconme->setStyleClass("ericcolors2");
infoitemiconme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("itemid",QString::fromStdString(dataitem.toUTF8()));
timergeneric2->start();
networkutils->queryitems(info);






});

auto infousericonme = std::make_unique<Wt::WText>(usericonc);
infousericonme->setStyleClass("ericcolors2");
infousericonme->clicked().connect([=] {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("ids_id",QString::fromStdString(datarecipient.toUTF8()));
timergeneric1->start();
networkutils->userbyid(info);




});


QString ltme = mypayload->mylocaltime(QString::fromStdString(datadate.toUTF8()), "yyyy-MM-ddTHH:mm:ss.zzzZ");


reqview1_->bindString("reqid",dataordersid.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("custid",datarecipient.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindWidget("custinfo", std::move(infousericonme));
reqview1_->bindString("itemid",dataitem.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindWidget("iteminfo", std::move(infoitemiconme));
reqview1_->bindString("locationinfo",datalocationto.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("businessid",datadeliverer.toUTF8(),Wt::TextFormat::Plain);
reqview1_->bindString("datereq",ltme.toStdString(),Wt::TextFormat::Plain);
reqview1_->bindWidget("deleteaction", std::move(deleteiconme));
divinner1_->addWidget(std::move(reqview1_));





}

else {


auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
auto reqview1_ = std::make_unique<Wt::WTemplate>();
auto strreqview1 = Wt::WString::tr("uiux1-view7");
reqview1_->setTemplateText(strreqview1);
reqview1_->bindEmpty("reqid");
reqview1_->bindEmpty("custid");
reqview1_->bindEmpty("custinfo");
reqview1_->bindEmpty("itemid");
reqview1_->bindEmpty("iteminfo");
reqview1_->bindEmpty("locationinfo");
reqview1_->bindEmpty("businessid");
reqview1_->bindEmpty("datereq");
reqview1_->bindEmpty("deleteaction");
divinner1_->addWidget(std::move(reqview1_));
divouter1_->addWidget(std::move(divinner1_));
allrequests_->bindEmpty("myrequestsaction");





}





  }

divouter1_->addWidget(std::move(divinner1_));
allrequests_->bindWidget("myrequestsaction",std::move(divouter1_));


    }

else {

auto divouter1_ = std::make_unique<Wt::WContainerWidget>();
auto divinner1_ = std::make_unique<Wt::WContainerWidget>();
auto reqview1_ = std::make_unique<Wt::WTemplate>();
auto strreqview1 = Wt::WString::tr("uiux1-view7");
reqview1_->setTemplateText(strreqview1);
reqview1_->bindEmpty("reqid");
reqview1_->bindEmpty("custid");
reqview1_->bindEmpty("custinfo");
reqview1_->bindEmpty("itemid");
reqview1_->bindEmpty("iteminfo");
reqview1_->bindEmpty("locationinfo");
reqview1_->bindEmpty("businessid");
reqview1_->bindEmpty("datereq");
reqview1_->bindEmpty("deleteaction");
divinner1_->addWidget(std::move(reqview1_));
divouter1_->addWidget(std::move(divinner1_));
allrequests_->bindEmpty("myrequestsaction");






}




   if((itemsstandardmodel->rowCount())==0 && !(businesssubscription > 1 || businessblacklist > 0 || businessverification > 3)) {



   // qDebug() << "itemsstandardmodel is empty! " << Qt::endl;
    auto container = std::make_unique<Wt::WContainerWidget>();
    container->setStyleClass("album py-5 bg-light");
    auto div2_ = std::make_unique<Wt::WContainerWidget>();
    div2_->setStyleClass("container");
    auto div3_ = std::make_unique<Wt::WContainerWidget>();
    div3_->setStyleClass("row row-cols-1 row-cols-sm-2 row-cols-md-3 g-3");
    auto appview8_ = std::make_unique<Wt::WTemplate>();
    auto strview1 = Wt::WString::tr("appview-view8");
    appview8_->setTemplateText(strview1);
    appview8_->bindEmpty("smalltxt");
    appview8_->bindEmpty("para");
    appview8_->bindEmpty("imgsrc");
    appview8_->bindEmpty("standardized");
    appview8_->bindEmpty("mediasrc");
    appview8_->bindEmpty("quantity");
    appview8_->bindEmpty("category");
    appview8_->bindEmpty("standardizedmedia");
    appview8_->bindEmpty("itemname");
    appview8_->bindEmpty("username");
    appview8_->bindEmpty("manufacturer");
    appview8_ ->bindEmpty("myparagraph");
    appview8_ ->bindEmpty("mysmalltext");
    appview8_ ->bindEmpty("myimagetemplate");
    appview8_ ->bindEmpty("btn1");
    appview8_ ->bindEmpty("btn2");
    appview8_ ->bindEmpty("btn3");
    div3_->addWidget(std::move(appview8_));
    div2_->addWidget(std::move(div3_));
    container->addWidget(std::move(div2_));
    allitems_->bindEmpty("items");
    allitems_->bindEmpty("itemlinkaaa");
    allitems_->bindEmpty("itemlinkaab");
    allitems_->bindEmpty("itemlinkaac");
    allitems_->bindEmpty("fav1");
    allitems_->bindEmpty("fav2");
    allitems_->bindEmpty("fav3");
    allitems_->bindEmpty("des1");
    allitems_->bindEmpty("des2");
    allitems_->bindEmpty("des3");
    allitems_->bindEmpty("name1");
    allitems_->bindEmpty("name2");
    allitems_->bindEmpty("name3");
    removetpl(allitems_,"myspinner1");
    showCustomWidgetT("spinner1","myspinner1",allitems_);





     }
     else { // start items






         auto container = std::make_unique<Wt::WContainerWidget>();
         container->setStyleClass("album py-5 bg-light");
         auto div2_ = std::make_unique<Wt::WContainerWidget>();
         div2_->setStyleClass("container");
         auto div3_ = std::make_unique<Wt::WContainerWidget>();
         div3_->setStyleClass("row row-cols-1 row-cols-sm-2 row-cols-md-3 g-3");




         int allrows = itemsfilter->rowCount();

         for (int card = 0; card < allrows; ++card) {

          auto appview8_ = std::make_unique<Wt::WTemplate>();
          auto strview1 = Wt::WString::tr("appview-view8");
          appview8_->setTemplateText(strview1);
          auto paragraphtext_ = std::make_unique<Wt::WTemplate>();
          auto str4 = Wt::WString::tr("appview-view4");
          paragraphtext_->setTemplateText(str4);
          Wt::cpp17::any mydatades = itemsfilter->data(card,2);
          Wt::WString datades = Wt::cpp17::any_cast<Wt::WString>(mydatades);
          Wt::cpp17::any mydataitemname = itemsfilter->data(card,6);
          Wt::WString dataitemname = Wt::cpp17::any_cast<Wt::WString>(mydataitemname);
          Wt::cpp17::any mydataprice = itemsfilter->data(card,7);
          Wt::WString dataprice = Wt::cpp17::any_cast<Wt::WString>(mydataprice);
          Wt::cpp17::any mydatasize = itemsfilter->data(card,8);
          Wt::WString datasize = Wt::cpp17::any_cast<Wt::WString>(mydatasize);
          Wt::cpp17::any mydatastandardized = itemsfilter->data(card,9);
          Wt::WString datastandardized = Wt::cpp17::any_cast<Wt::WString>(mydatastandardized);
          Wt::cpp17::any mydataquantity = itemsfilter->data(card,8);
          Wt::WString dataquantity = Wt::cpp17::any_cast<Wt::WString>(mydataquantity);
          Wt::cpp17::any mydatacategory = itemsfilter->data(card,0);
          Wt::WString datacategory = Wt::cpp17::any_cast<Wt::WString>(mydatacategory);
          Wt::cpp17::any mydatastandardizedmedia  = itemsfilter->data(card,10);
          Wt::WString datastandardizedmedia = Wt::cpp17::any_cast<Wt::WString>(mydatastandardizedmedia);
          Wt::cpp17::any mydatauuid  = itemsfilter->data(card,12);
          Wt::WString datauuid = Wt::cpp17::any_cast<Wt::WString>(mydatauuid);
          std::string paragraphtext = datades.toUTF8();
          Wt::cpp17::any mydatausername = itemsfilter->data(card,11);
          Wt::WString datausername = Wt::cpp17::any_cast<Wt::WString>(mydatausername);
       //   std::cout << "paragraphtext is: " << paragraphtext << std::endl;
          paragraphtext_->bindString("para",paragraphtext,Wt::TextFormat::Plain);
          auto smalltext_ = std::make_unique<Wt::WTemplate>();
          auto str5 = Wt::WString::tr("appview-view5");
          smalltext_->setTemplateText(str5);
          std::string smalltext = dataprice.toUTF8();
          auto imagetemplate_ = std::make_unique<Wt::WTemplate>();
          auto str6 = Wt::WString::tr("appview-view6");
          imagetemplate_->setTemplateText(str6);
          Wt::cpp17::any mydataitemuuid = itemsfilter->data(card,3);
          Wt::WString dataitemuuid = Wt::cpp17::any_cast<Wt::WString>(mydataitemuuid);
          Wt::cpp17::any mydatamanufacturer = itemsfilter->data(card,4);
          Wt::WString datamanufacturer = Wt::cpp17::any_cast<Wt::WString>(mydatamanufacturer);
          Wt::cpp17::any mydatamedia = itemsfilter->data(card,5);
          Wt::WString datamedia = Wt::cpp17::any_cast<Wt::WString>(mydatamedia);
          std::string mysrc = QString::fromStdWString(networkutils->myurldue(MEDIAURL)).toStdString()+datamedia.toUTF8();
       //   std::cout << "mysrc is: " << mysrc << std::endl;
          auto btnscope1_ = std::make_unique<Wt::WPushButton>("Update");
          btnscope1_->setStyleClass("btn btn-sm btn-outline-secondary");
          btnscope1_->clicked().connect([=] {

              //qDebug() << "mysrc is: " << QString::fromStdString(mysrc) << Qt::endl;
              //qDebug() << "card is: " << card << Qt::endl;

              if(loggeduserdue.isEmpty()) {

                  if(appvieweric_->isVisible()) {

                      if(accregister_)
                          accregister_->hide();
                      if(accretrieve_)
                          accretrieve_->hide();
                      if(accsetup_)
                         accsetup_->show();

                        appvieweric_->hide();


                  }

                  else {



                  }


              }

              else if(!loggeduserdue.isEmpty() && loggeduserdue.toStdString()==datausername.toUTF8()) {


                 //  qDebug() << "Proceed to Update an item... "  << Qt::endl;


                   doupdateitem = true;
                   retainmedianame = "";
                   retainmedianame = datamedia;
                   retainuniqueid = "";
                   retainuniqueid = dataitemuuid;
                   int index = combocategoryupdate->findText(datacategory);
                   combocategoryupdate->setCurrentIndex(index);
                   updateitemname->setText(dataitemname);
                   updatequantity->setText(dataquantity);
                   updateprice->setText(dataprice);
                   updateitemdescription->setText(datades);
                   updatestandardized->setText(datastandardized);
                   updatestandardizedmedia->setText(datastandardizedmedia);
                   updateitemuuid->setText(datauuid);
                   updatemanufacturer->setText(datamanufacturer);
                   //mytemplateformview_->updateModel(myformmodelnew_.get());
                  // mytemplateformview_->updateView(myformmodelnew_.get());
                   updatetemplateformview_->updateModel(myformmodelupdate_.get());
                   updatetemplateformview_->updateView(myformmodelupdate_.get());

                   menu->itemAt(0)->menu()->itemAt(1)->hide();
                   menu->itemAt(0)->menu()->itemAt(2)->show();
                   menu->itemAt(0)->menu()->itemAt(3)->hide();
                   menu->itemAt(0)->menu()->select(2);





              }


              else {


            customToast("appview-view20","Only the owner of the business can do that!");
            timerdeletewidget->start();


              }


          });

          auto btnscope2_ = std::make_unique<Wt::WPushButton>("Delete");
          btnscope2_->setStyleClass("btn btn-sm btn-outline-secondary");
          btnscope2_->clicked().connect([=] {





             if(loggeduserdue.isEmpty()) {

                 if(appvieweric_->isVisible()) {

                     if(accregister_)
                         accregister_->hide();
                     if(accretrieve_)
                         accretrieve_->hide();
                     if(accsetup_)
                        accsetup_->show();

                       appvieweric_->hide();


                 }

            else  {




                 }


             }

             else if(!loggeduserdue.isEmpty() && loggeduserdue.toStdString()==datausername.toUTF8()) {

              //   qDebug() << "Delete Step!" << Qt::endl;

                   deletemap = QVariantMap();
                   QVariantMap info;
                   info.insert("user",loggeduserdue);
                   info.insert("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));
                   info.insert("name",QString::fromStdString(dataitemname.toUTF8()));
                   info.insert("category",QString::fromStdString(datacategory.toUTF8()));
                   info.insert("avatarfield",QString::fromStdString(datamedia.toUTF8()));

                   QString idsid = "";
                   std::string idsfile;
                   idsfile = mypayload->TOKEN_DIRIDS;
                   idsfile.append(loggeduserdue.toStdString());
                   idsfile.append("/ids.txt");



                   if(std::ifstream is{idsfile, std::ios_base::in}) {

                       auto size = is.tellg();
                       char * str;
                       std::string content = mypayload->fromIstream(is.read(str,size));
                       idsid = QString::fromStdString(content);
                        // construct string to stream size
                      //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
                        is.close();

                     }

                   info.insert("ids_id",idsid);
                   deletemap = info;


                 timerdeleteitem->start();
                 networkutils->deleteitem(deletemap);




             }

         else {


        customToast("appview-view20","Only the owner of the business can do that!");
        timerdeletewidget->start();


             }



          });



          auto btnscope3_ = std::make_unique<Wt::WPushButton>("Request Item");
          btnscope3_->setStyleClass("btn btn-outline-info btn-rounded");
          auto scope3_ = btnscope3_.get();
          auto btnscope3a_ = std::make_unique<Wt::WPushButton>("Cancel Request");
          btnscope3a_->setStyleClass("btn btn-outline-danger btn-rounded");
          btnscope3a_->hide();
          auto scope3a_ =  btnscope3a_.get();


          btnscope3_->clicked().connect([=] {




              if(loggeduserdue.isEmpty()) {

                  if(appvieweric_->isVisible()) {

                      if(accregister_)
                          accregister_->hide();
                      if(accretrieve_)
                          accretrieve_->hide();
                      if(accsetup_)
                         accsetup_->show();

                        appvieweric_->hide();


                  }

             else  {




                  }


              }

              else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausername.toUTF8())) {

                   scope3a_->show();
                   scope3_->hide();
                 //  qDebug() << "btnscope3_ clicked()!" << Qt::endl;
                   if(timercarousel->isActive()) {

                   // timercarousel->stop(); // eric ruin

                   }




                    QString idsid = "";
                    std::string idsfile;
                    idsfile = mypayload->TOKEN_DIRIDS;
                    idsfile.append(loggeduserdue.toStdString());
                    idsfile.append("/ids.txt");



                    if(std::ifstream is{idsfile, std::ios_base::in}) {

                        auto size = is.tellg();
                        char * str;
                        std::string content = mypayload->fromIstream(is.read(str,size));
                        idsid = QString::fromStdString(content);
                         // construct string to stream size
                       //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
                         is.close();

                      }



      if(ordersmap.count("itemsid")<1) {

      isnew = true;
      ordersmap.clear();

      }




         if(isnew) {

         ordersmap.insert("ids_id",businessuuid);
         ordersmap.insert("recipient",idsid);
         ordersmap.insert("deliverer",businessuuid);
         ordersmap.insert("locationto",reqlocationto);
         ordersmap.insert("locationfrom",reqlocationfrom);
         ordersmap.insert("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));
       //  qDebug() << "isnew is: " << isnew << Qt::endl;



              }

           else {

          //  qDebug() << "isnew is: " << isnew << Qt::endl;

               }

         QList<QVariant> values = ordersmap.values("itemsid");
         for (int i = 0; i < values.size(); ++i) {

          if(values.at(i).toString()==QString::fromStdString(dataitemuuid.toUTF8())) {


            inmap = true;

          }

          else {


          inmap = false;

          }

          }


       if(!inmap) {

      //  qDebug()  << "Adding itemuuid in DS!" << Qt::endl;
        ordersmap.insert("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));



         }

       else {

    //   qDebug()  << "itemuuid already in DS!" << Qt::endl;

       }


       QList<QVariant> ids_idvalues = ordersmap.values("ids_id");
       for (int i = 0; i < ids_idvalues.size(); ++i) {

      //  qDebug()  << "ids_id QList<QVariant> is: " << ids_idvalues.at(i).toString()  << Qt::endl;
        reqidids = ids_idvalues.at(i).toString();


        }

       QList<QVariant> recipientvalues = ordersmap.values("recipient");
       for (int i = 0; i < recipientvalues.size(); ++i) {

       // qDebug()  << "recipient QList<QVariant> is: " << recipientvalues.at(i).toString()  << Qt::endl;
        reqrecipient = recipientvalues.at(i).toString();


        }


       QList<QVariant> deliverervalues = ordersmap.values("deliverer");
       for (int i = 0; i < deliverervalues.size(); ++i) {

       // qDebug()  << "deliverer QList<QVariant> is: " << deliverervalues.at(i).toString()  << Qt::endl;
       reqdeliverer = deliverervalues.at(i).toString();


        }


       QList<QVariant> locationtovalues = ordersmap.values("locationto");
       for (int i = 0; i < locationtovalues.size(); ++i) {

      //  qDebug()  << "locationto QList<QVariant> is: " << locationtovalues.at(i).toString()  << Qt::endl;
        reqlocationto = locationtovalues.at(i).toString();

        }


       QList<QVariant> locationfromvalues = ordersmap.values("locationfrom");
       for (int i = 0; i < locationfromvalues.size(); ++i) {

     //   qDebug()  << "locationfrom QList<QVariant> is: " << locationfromvalues.at(i).toString()  << Qt::endl;
        reqlocationfrom = locationfromvalues.at(i).toString();


        }

       QList<QVariant> actualvalues = ordersmap.values("itemsid");
       for (int i = 0; i < actualvalues.size(); ++i) {

      //  qDebug()  << "itemsid QList<QVariant> is: " << actualvalues.at(i).toString()  << Qt::endl;


        }




       QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
       while (j != ordersmap.end() && j.key() == "itemsid") {
      // qDebug() << "itemsid   is: " << j.value().toString() << Qt::endl;
       isnew = false;
        ++j;

          }








              }

          else {


         customToast("appview-view20","Please sign in and ensure that you are not requesting items from your own business!");
         timerdeletewidget->start();


              }



          });




          btnscope3a_->clicked().connect([=] {




              if(loggeduserdue.isEmpty()) {

                  if(appvieweric_->isVisible()) {

                      if(accregister_)
                          accregister_->hide();
                      if(accretrieve_)
                          accretrieve_->hide();
                      if(accsetup_)
                         accsetup_->show();

                        appvieweric_->hide();


                  }

             else  {




                  }


              }

              else if(!loggeduserdue.isEmpty() && !(loggeduserdue.toStdString()==datausername.toUTF8())) {

                 //  qDebug() << "btnscope3a_ clicked()!" << Qt::endl;
                   scope3a_->hide();
                   scope3_->show();

                   QString idsid = "";
                   std::string idsfile;
                   idsfile = mypayload->TOKEN_DIRIDS;
                   idsfile.append(loggeduserdue.toStdString());
                   idsfile.append("/ids.txt");



                   if(std::ifstream is{idsfile, std::ios_base::in}) {

                       auto size = is.tellg();
                       char * str;
                       std::string content = mypayload->fromIstream(is.read(str,size));
                       idsid = QString::fromStdString(content);
                        // construct string to stream size
                      //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
                        is.close();

                     }


                   if(ordersmap.count("itemsid") == 1) {

                   isnew = true;
                   ordersmap.remove("ids_id",idsid);
                   ordersmap.remove("recipient",idsid);
                   ordersmap.remove("deliverer",businessuuid);
                   ordersmap.remove("locationto",reqlocationto);
                   ordersmap.remove("locationfrom",reqlocationfrom);
                   ordersmap.remove("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));


                   }

                   else {

                  ordersmap.remove("itemsid",QString::fromStdString(dataitemuuid.toUTF8()));



                   }


                 /*  QMultiMap<QString, QVariant>::iterator j = ordersmap.find("itemsid");
                   while (j != ordersmap.end() && j.key() == "itemsid") {
                   qDebug() << "itemsid after remove is: " << j.value().toString() << Qt::endl;
                    ++j;

                      } */










              }

          else {


         customToast("appview-view20","Only the owner of the business can do that!");
         timerdeletewidget->start();


              }



          });


          if(loggeduserdue==userdue) {

              btnscope1_->show();
              btnscope2_->show();
              btnscope3_->hide();
              btnscope3a_->hide();

          }


          else {

              btnscope1_->hide();
              btnscope2_->hide();
              btnscope3_->show();

           }


          //  https://www.youtube.com/embed/rRoy6I4gKWU?autoplay=1
          // https://www.youtube.com/v/rRoy6I4gKWU?autoplay=1



          std::string match1 = "https://www.youtube.com/embed";
          std::string match2 = "https://www.youtube.com/v";
          std::string myinputtxt1 = datastandardizedmedia.toUTF8();
          std::string spattern1 = "("+match1+")(.*)";
          std::string spattern2 = "("+match2+")(.*)";


          auto appviewordinaryimage1_ = std::make_unique<Wt::WTemplate>();
          auto strviewordinaryimage1 = Wt::WString::tr("ordinaryimages");
          appviewordinaryimage1_->setTemplateText(strviewordinaryimage1);
          appviewordinaryimage1_->bindString("imgsrc",mysrc,Wt::TextFormat::Plain);


          auto appviewordinaryimage2_ = std::make_unique<Wt::WTemplate>();
          auto strviewordinaryimage2 = Wt::WString::tr("ordinaryimages");
          appviewordinaryimage2_->setTemplateText(strviewordinaryimage2);
          appviewordinaryimage2_->bindString("imgsrc",mysrc,Wt::TextFormat::Plain);
          auto appviewordinaryimage2 = appviewordinaryimage2_.get();



          auto appviewembedyoutube1_ = std::make_unique<Wt::WTemplate>();
          auto embedyoutube1 = Wt::WString::tr("youtubeembed1");
          appviewembedyoutube1_->setTemplateText(embedyoutube1);
          appviewembedyoutube1_->bindString("standardizedmedia",datastandardizedmedia.toUTF8(),Wt::TextFormat::Plain);
          auto appviewembedyoutube1 = appviewembedyoutube1_.get();


          auto appviewembedyoutube2_ = std::make_unique<Wt::WTemplate>();
          auto embedyoutube2 = Wt::WString::tr("youtubeembed2");
          appviewembedyoutube2_->setTemplateText(embedyoutube2);
          appviewembedyoutube2_->bindWidget("ordinaryimages", std::move(appviewordinaryimage2_));
          appviewembedyoutube2_->bindWidget("youtubeembed1", std::move(appviewembedyoutube1_));





          appviewordinaryimage2->clicked().connect([=] {

          if((std::regex_match(myinputtxt1, std::regex(spattern1)))) {

             // appviewembedyoutube1->show();
             // appviewordinaryimage2->hide();

            }

          else {



          }



          });



          auto flashcontainer = std::make_unique<Wt::WContainerWidget>();
          auto flash = flashcontainer->addNew<Wt::WFlashObject>(datastandardizedmedia.toUTF8());
          flash->setFlashParameter("allowFullScreen", "true");
         // flash->setFlashParameter("allowFullScreen", "yes");
          auto myposter = std::make_unique<Wt::WImage>(Wt::WLink(mysrc));
          myposter->setStyleClass("img-fluid card-img-top");
          myposter->setAttributeValue("width" ,"100%");
          myposter->setAttributeValue("height" ,"100%");
          flash->setAlternativeContent(std::move(myposter));



          auto containervideo = std::make_unique<Wt::WContainerWidget>();
          containervideo->setStyleClass("card");
          auto videoply = containervideo->addNew<Wt::WVideo>();
          videoply->addSource(Wt::WLink(mysrc));
          auto myposter2 = std::make_unique<Wt::WImage>(Wt::WLink(datastandardizedmedia.toUTF8()));
          myposter2->setStyleClass("img-fluid card-img-top");
          myposter2->setAttributeValue("width" ,"100%");
          myposter2->setAttributeValue("height" ,"100%");
          videoply->setAlternativeContent(std::move(myposter2));





          if(datastandardizedmedia.toUTF8()=="later" || datastandardizedmedia.toUTF8()=="null" || datastandardizedmedia.toUTF8()=="nill" || datastandardizedmedia.toUTF8()=="empty" || datastandardizedmedia.toUTF8()=="N/A") {

           appview8_->bindWidget("mediasrc", std::move(appviewordinaryimage1_));

          }

          else if((std::regex_match(myinputtxt1, std::regex(spattern1)))) {

        //  appviewembedyoutube1->hide();
            appviewordinaryimage2->hide();
          appview8_->bindWidget("mediasrc", std::move(appviewembedyoutube2_));



          }

          else if((std::regex_match(myinputtxt1, std::regex(spattern2)))) {


           appview8_->bindWidget("mediasrc", std::move(flashcontainer));



          }


          else if(datastandardizedmedia.toUTF8()=="localvideo") {

            //  appview8_->bindWidget("mediasrc", std::move(containerplayer));
               appview8_->bindWidget("mediasrc", std::move(containervideo));

             }



          else {

         appview8_->bindWidget("mediasrc", std::move(containervideo));

          }






          appview8_->bindString("smalltxt",smalltext,Wt::TextFormat::Plain);
          appview8_->bindString("para",paragraphtext,Wt::TextFormat::Plain);
          appview8_->bindString("standardized",datastandardized.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("quantity",dataquantity.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("category",datacategory.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("itemname",dataitemname.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("username",datausername.toUTF8(),Wt::TextFormat::Plain);
          appview8_->bindString("manufacturer",datamanufacturer.toUTF8(),Wt::TextFormat::Plain);
          appview8_ ->bindWidget("myparagraph", std::move(paragraphtext_));
          appview8_ ->bindWidget("mysmalltext", std::move(smalltext_));
          appview8_ ->bindWidget("myimagetemplate", std::move(imagetemplate_));
          appview8_ ->bindWidget("btn1", std::move(btnscope1_));
          appview8_ ->bindWidget("btn2", std::move(btnscope2_));
          appview8_ ->bindWidget("btn3", std::move(btnscope3_));
          appview8_ ->bindWidget("btn4", std::move(btnscope3a_));
          div3_->addWidget(std::move(appview8_));




             }



           div2_->addWidget(std::move(div3_));
           container->addWidget(std::move(div2_));



          allitems_->bindWidget("items",std::move(container));
          removetpl(allitems_,"myspinner1");



          isstartup = false;






        }  // end items






    }

    catch(std::exception & e) {

      std::cout << "exception is: " << e.what() << std::endl;

    }













}  // end thedatauserftr()


void MapperSoftware::theitemnew() {


if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();
doupdateitem = false;
douploadnewitem=false;

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {


QString * str = mysuccessmessage.get();
QString data = *str;
if(!douploadnewitem) {

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
//QString name =  mydearobj.value("name").toString();
//qDebug() << "items_id is: " << results << Qt::endl;
//qDebug() << "name is: " << name << Qt::endl;
//qDebug() << "data theitemnew is: " << data << Qt::endl;

if(results=="" && !doupdateitem) {

fu->upload();
uploadButton->disable();
timeritemnew->stop();
douploadnewitem=true;

}
else if(doupdateitem && results=="") {

 customToast("appview-view20","You cannot update non-existing item!");
 timerdeletewidget->start();
 timerdeletewidget->start();
 doupdateitem = false;
 douploadnewitem=false;

}

else if(doupdateitem && !(results=="")) {

fu->upload();
uploadButton->disable();
timeritemnew->stop();
douploadnewitem=true;

}


else {

customToast("appview-view20","Item already exist!");
timerdeletewidget->start();
timerdeletewidget->start();
doupdateitem = false;
douploadnewitem=false;

 }

}

else {

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;
if(!results.isEmpty() && !doupdateitem) {

customToast("appview-view19","Item was uploaded successful!");
timerdeletewidget->start();
uploadButton->enable();
if(!(results=="")) {
if(timercarousel->isActive()){

  timercarousel->stop();


}

networkutils->getuseritems(userdue);
timergetdatauser->start();

}


}

else if(!results.isEmpty() && doupdateitem) {

customToast("appview-view19","Item was updated successfully!");
timerdeletewidget->start();
uploadButton->enable();
doupdateitem = false;
if(timercarousel->isActive()){

timercarousel->stop();


}

networkutils->getuseritems(userdue);
timergetdatauser->start();
doupdateitem = false;
douploadnewitem=false;

    }

else {

customToast("appview-view20","Item was not uploaded,check your internet connection!");
timerdeletewidget->start();
doupdateitem = false;
douploadnewitem=false;
//timercarousel->start();

}

douploadnewitem=false;
doupdateitem = false;


}




}



}



void MapperSoftware::thedeleteitem() {


if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","An error occured while deleting item!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;




mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
qDebug() << "data is: " << data << Qt::endl;
qDebug() << "items_id is: " << results << Qt::endl;

if(!(results=="")) {
if(timercarousel->isActive()){

  timercarousel->stop();


}

networkutils->getuseritems(userdue);
timergetdatauser->start();

}






}


}


void MapperSoftware::theupdateuser() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
 qDebug() << "data is: " << data << Qt::endl;


}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;


}



 }

}

void MapperSoftware::theupdatepassword() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;




mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("ids_id").toString();
Q_UNUSED(results)
//qDebug() << "data updatepassword is: " << data << Qt::endl;
//qDebug() << "idsid 1 updatepassword is: " << results << Qt::endl;
networkutils->getuseritems(userdue);
timergetdatauser->start();




 }

}

void MapperSoftware::theupdatephonenumber() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("ids_id").toString();
Q_UNUSED(results)
//qDebug() << "idsid 2 updatephonenumber is: " << results << Qt::endl;
networkutils->getuseritems(userdue);
timergetdatauser->start();





 }



}


void MapperSoftware::thelogout() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Failed to sign out!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

//qInfo() << "new token after logout is: " << data << Qt::endl;

std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(loggeduserdue.toStdString());
tokenfile.append("/reason.txt");

std::ofstream fout(tokenfile,std::ios::trunc);

if(fout) {

 fout << data.toStdString();
 fout.close();


}


signiniconscope1_->show();
signouticonscope1_->hide();
customToast("appview-view19","Signed out,please sign in again for advanced operations.");
timerdeletewidget->start();
clearregister();
clearlogin();
clearretrieve();
myformmodelnew_->reset();
myformmodelupdate_.get()->reset();
mycategoryformmodel_.get()->reset();
passupdateformmodel_.get()->reset();
phoneupdateformmodel_.get()->reset();
locinfoformmodel_.get()->reset();
mytemplateformview_->updateView(myformmodelnew_.get());
updatetemplateformview_->updateView(myformmodelupdate_.get());
categorytemplateformview_->updateView(mycategoryformmodel_.get());
passupdatetemplateformview_->updateView(passupdateformmodel_.get());
phoneupdatetemplateformview_->updateView(phoneupdateformmodel_.get());
locinfotemplateformview_->updateView(locinfoformmodel_.get());
menu->itemAt(0)->menu()->select(0);
menu->itemAt(0)->menu()->itemAt(1)->hide();
menu->itemAt(0)->menu()->itemAt(2)->hide();
menu->itemAt(0)->menu()->itemAt(3)->hide();
loggeduserdue ="";


networkutils->getuseritems(userdue);
timergetdatauser->start();






 }

}

void MapperSoftware::thecategorynew() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "data is: " << data << Qt::endl;


}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("categories_id").toString();
//Q_UNUSED(results)
//qDebug() << "categories id is: " << results << Qt::endl;
if(!(results=="")) {
if(timercarousel->isActive()){

  timercarousel->stop();


}

networkutils->getuseritems(userdue);
timergetdatauser->start();

}
//qDebug() << "data is: " << data << Qt::endl;


}



     }

}

void MapperSoftware::thedeletecategory() {


if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("categories_id").toString();
Q_UNUSED(results)
//qDebug() << "data is: " << data << Qt::endl;


}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("categories_id").toString();
Q_UNUSED(results)
if(!(results=="")) {
if(timercarousel->isActive()){

  timercarousel->stop();


}

networkutils->getuseritems(userdue);
timergetdatauser->start();

}

}


}

}

void MapperSoftware::thelocationnew() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;


}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
qDebug() << "data is: " << data << Qt::endl;

}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;
}



     }

}

void MapperSoftware::thedeletelocation() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;


}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
qDebug() << "data is: " << data << Qt::endl;


}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;


}



     }

}

void MapperSoftware::theordernew() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;


}

else {

QString * str = mysuccessmessage.get();
QString data = *str;



mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("orders_id").toString();
Q_UNUSED(results)
qDebug() << "order id is: " << results << Qt::endl;
if(!results.isEmpty()) {

timergetdatauser->start();
locinfoformmodel_.get()->reset();
locinfotemplateformview_->updateView(locinfoformmodel_.get());
networkutils->getuseritems(userdue);



}

else {

customToast("appview-view20","Failed to make a request!");
timerdeletewidget->start();

}





     }

}



void MapperSoftware::thedeleteorder() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
bool results =  mydearobj.value("deleted").toBool();
Q_UNUSED(results)
//qDebug() << "deleted is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;

if(results) {

customToast("appview-view19","Request deleted!");
timerdeletewidget->start();
timergetdatauser->start();
networkutils->getuseritems(userdue);

}

else {

customToast("appview-view20","Failed to delete request!");
timerdeletewidget->start();


}






     }

}


void  MapperSoftware::thegenericdata1() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;



mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("phone").toString();
Q_UNUSED(results)
//qDebug() << "data is: " << data << Qt::endl;
//qDebug() << "Phone number is: " << results << Qt::endl;
QVariantMap info;
info.insert("username",mydearobj.value("username").toString());
info.insert("phone",mydearobj.value("phone").toString());
info.insert("email",mydearobj.value("email").toString());
info.insert("idsid",mydearobj.value("ids_id").toString());
customToastUD("appview-view21",info);









 }


}



void  MapperSoftware::thegenericdata2() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("price").toString();
Q_UNUSED(results)
//qDebug() << "Price is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;
QVariantMap info;
info.insert("name",mydearobj.value("name").toString());
info.insert("price",mydearobj.value("price").toString());
info.insert("quantity",mydearobj.value("sizeorquantity").toString());
info.insert("category",mydearobj.value("category").toString());
info.insert("itemid",mydearobj.value("items_id").toString());
customToastID("appview-view22",info);







    }


}

void MapperSoftware::thelogisticsnew() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
qDebug() << "data is: " << data << Qt::endl;


}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;

}



     }


}

void MapperSoftware::thedeletelogistics() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
qDebug() << "data is: " << data << Qt::endl;

}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;

}



}

}

void MapperSoftware::thedeliverynew() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
qDebug() << "data is: " << data << Qt::endl;


}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;


}



 }


}

void MapperSoftware::thedeletedelivery() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;


}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
qDebug() << "data is: " << data << Qt::endl;


}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;


}



  }

}

void MapperSoftware::themetricsnew() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
qDebug() << "data is: " << data << Qt::endl;

}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;

}



     }

}

void MapperSoftware::thedeletemetrics() {

if(mysuccessmessage->isEmpty()) {

customToast("appview-view20","Server error!");
timerdeletewidget->start();

//QString * str = myerrormessage.get();
//QString data = *str;
//qDebug() << "error message is: " << qPrintable(data) << Qt::endl;



}

else {

QString * str = mysuccessmessage.get();
QString data = *str;

{


mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonArray mydeararr = myjsonvalue.toArray();
QJsonValue dataFirstarr = mydeararr.first();
QJsonObject mydearobj = dataFirstarr.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
qDebug() << "data is: " << data << Qt::endl;

}

{

mypayload->processRequest(data);
QJsonObject mydearpayloadresults = mypayload->m_requestJson.toObject();
QJsonValue myjsonvalue = mydearpayloadresults.value("data");
QJsonObject mydearobj = myjsonvalue.toObject();
QString results =  mydearobj.value("items_id").toString();
Q_UNUSED(results)
//qDebug() << "Item id is: " << results << Qt::endl;
//qDebug() << "data is: " << data << Qt::endl;

}



     }


}





const int MapperSoftware::MAX_LENGTH = 100;
const int MapperSoftware::MAX_LONGLENGTH = 100000000;

const Wt::WFormModel::Field MapperSoftware::CategoryField = "category";
const Wt::WFormModel::Field MapperSoftware::ItemNameField = "item-name";
const Wt::WFormModel::Field MapperSoftware::QuantityField = "quantity";
const Wt::WFormModel::Field MapperSoftware::PriceField = "price";
const Wt::WFormModel::Field MapperSoftware::DescriptionField = "description";
const Wt::WFormModel::Field MapperSoftware::MediaField = "media";
const Wt::WFormModel::Field MapperSoftware::StandardizedField = "standardized";
const Wt::WFormModel::Field MapperSoftware::StandardizedMediaField = "standardized-media";
const Wt::WFormModel::Field MapperSoftware::ItemUuidField = "item-uuid";
const Wt::WFormModel::Field MapperSoftware::ManufacturerField = "manufacturer";
const Wt::WFormModel::Field MapperSoftware::PasswordField = "password";
const Wt::WFormModel::Field MapperSoftware::PhoneField = "phone-number";
const Wt::WFormModel::Field MapperSoftware::LocationField = "mylocation";



const MapperSoftware::CategoryMap  MapperSoftware::categories = {
  { "null", { "null" } },
  { "nill", { "nill" } }
};






void MapperSoftware::createmodelnew() {




    // initializeModels();

   //  myformmodelnew_->reset();
  //   myformmodelnew_ = nullptr;
  //   myformmodelnew_ = std::make_shared<Wt::WFormModel>();
     myformmodelnew_->addField(CategoryField);
     myformmodelnew_->addField(ItemNameField);
     myformmodelnew_->addField(QuantityField);
     myformmodelnew_->addField(PriceField);
     myformmodelnew_->addField(DescriptionField);
    // myformmodelnew_->addField(MediaField);
     myformmodelnew_->addField(StandardizedField);
     myformmodelnew_->addField(StandardizedMediaField);
     myformmodelnew_->addField(ItemUuidField);
     myformmodelnew_->addField(ManufacturerField);

     myformmodelnew_->setValidator(CategoryField, createBasicValidator());
     myformmodelnew_->setValidator(ItemNameField, createShortTextValidator(ItemNameField));
     myformmodelnew_->setValidator(QuantityField, createShortTextValidator(QuantityField));
     myformmodelnew_->setValidator(PriceField, createShortTextValidator(PriceField));
     myformmodelnew_->setValidator(DescriptionField, createLongTextValidator(DescriptionField));
  //   myformmodelnew_->setValidator(MediaField, createShortTextValidator(MediaField));
     myformmodelnew_->setValidator(StandardizedField, createShortNTextValidator(StandardizedField));
     myformmodelnew_->setValidator(StandardizedMediaField, createShortNTextValidator(StandardizedMediaField));
     myformmodelnew_->setValidator(ItemUuidField, createShortNTextValidator(ItemUuidField));
     myformmodelnew_->setValidator(ManufacturerField, createShortNTextValidator(ManufacturerField));

     // Here you could populate the model with initial data using
     // setValue() for each field.

     myformmodelnew_->setValue(CategoryField, std::string());

     return;
}


void MapperSoftware::createmodelupdate() {




    // initializeModels();

   //  myformmodelupdate_->reset();
  //   myformmodelupdate_ = nullptr;
  //   myformmodelupdate_ = std::make_shared<Wt::WFormModel>();
     myformmodelupdate_->addField(CategoryField);
     myformmodelupdate_->addField(ItemNameField);
     myformmodelupdate_->addField(QuantityField);
     myformmodelupdate_->addField(PriceField);
     myformmodelupdate_->addField(DescriptionField);
    // myformmodelupdate_->addField(MediaField);
     myformmodelupdate_->addField(StandardizedField);
     myformmodelupdate_->addField(StandardizedMediaField);
     myformmodelupdate_->addField(ItemUuidField);
     myformmodelupdate_->addField(ManufacturerField);

     myformmodelupdate_->setValidator(CategoryField, createBasicValidator());
     myformmodelupdate_->setValidator(ItemNameField, createShortTextValidator(ItemNameField));
     myformmodelupdate_->setValidator(QuantityField, createShortTextValidator(QuantityField));
     myformmodelupdate_->setValidator(PriceField, createShortTextValidator(PriceField));
     myformmodelupdate_->setValidator(DescriptionField, createLongTextValidator(DescriptionField));
  //   myformmodelupdate_->setValidator(MediaField, createShortTextValidator(MediaField));
     myformmodelupdate_->setValidator(StandardizedField, createShortNTextValidator(StandardizedField));
     myformmodelupdate_->setValidator(StandardizedMediaField, createShortNTextValidator(StandardizedMediaField));
     myformmodelupdate_->setValidator(ItemUuidField, createShortNTextValidator(ItemUuidField));
     myformmodelupdate_->setValidator(ManufacturerField, createShortNTextValidator(ManufacturerField));

     // Here you could populate the model with initial data using
     // setValue() for each field.

     myformmodelupdate_->setValue(CategoryField, std::string());

     return;
}


std::shared_ptr<Wt::WAbstractItemModel> MapperSoftware::categoryModelnew() {
    return categoryModelnew_;
}

std::shared_ptr<Wt::WAbstractItemModel> MapperSoftware::categoryModelupdate() {
    return categoryModelnew_;
}

int MapperSoftware::categoryModelnewRow(const std::string& data) {
    for (int i = 0; i < categoryModelnew_->rowCount(); ++i)
        if (categoryDatanew(i) == data)
            return i;

    return -1;
}


int MapperSoftware::categoryModelupdateRow(const std::string& data) {
    for (int i = 0; i < categoryModelupdate_->rowCount(); ++i)
        if (categoryDataupdate(i) == data)
            return i;

    return -1;
}

// Get the right code for the current index.
std::string MapperSoftware::categoryDatanew(int row) {
    return Wt::asString(categoryModelnew_->data(row, 0, Wt::ItemDataRole::User)).toUTF8();
}

std::string MapperSoftware::categoryDataupdate(int row) {
    return Wt::asString(categoryModelupdate_->data(row, 0, Wt::ItemDataRole::User)).toUTF8();
}


Wt::WString MapperSoftware::userDatanew() {
       return
       " category=" + Wt::asString(myformmodelnew_->value(CategoryField ))
       + ", item name=" + Wt::asString(myformmodelnew_->value(ItemNameField))
       + ", quantity=" + Wt::asString(myformmodelnew_->value(QuantityField))
       + ", price=" + Wt::asString(myformmodelnew_->value(PriceField))
       + ", description=" + Wt::asString(myformmodelnew_->value(DescriptionField))
     //  + ", media=" + Wt::asString(value(MediaField))
       + ", standardized=" + Wt::asString(myformmodelnew_->value(StandardizedField))
       + ", standardized media="+ Wt::asString(myformmodelnew_->value(StandardizedMediaField))
       + ", uuid=" + Wt::asString(myformmodelnew_->value(ItemUuidField))
       + ", manufacturer=" + Wt::asString(myformmodelnew_->value(ManufacturerField))
       + ".";
   }

Wt::WString MapperSoftware::userDataupdate() {
       return
       " category=" + Wt::asString(myformmodelupdate_->value(CategoryField ))
       + ", item name=" + Wt::asString(myformmodelupdate_->value(ItemNameField))
       + ", quantity=" + Wt::asString(myformmodelupdate_->value(QuantityField))
       + ", price=" + Wt::asString(myformmodelupdate_->value(PriceField))
       + ", description=" + Wt::asString(myformmodelupdate_->value(DescriptionField))
     //  + ", media=" + Wt::asString(value(MediaField))
       + ", standardized=" + Wt::asString(myformmodelupdate_->value(StandardizedField))
       + ", standardized media="+ Wt::asString(myformmodelupdate_->value(StandardizedMediaField))
       + ", uuid=" + Wt::asString(myformmodelupdate_->value(ItemUuidField))
       + ", manufacturer=" + Wt::asString(myformmodelupdate_->value(ManufacturerField))
       + ".";
   }

QVariantMap MapperSoftware::myuserDatanew(QString mediafile) {

 mytemplateformview_->updateModel(myformmodelnew_.get());

 if (myformmodelnew_->validate()) {

   //qDebug() << "Data to be uploaded item new is: " << QString::fromStdString(userDatanew().toUTF8()) << Qt::endl;

    }


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("name",QString::fromStdString(Wt::asString(myformmodelnew_->value(ItemNameField)).toUTF8()));
info.insert("category",QString::fromStdString(Wt::asString(myformmodelnew_->value(CategoryField )).toUTF8()));

QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    idsid = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

info.insert("ids_id",idsid);
info.insert("itemsid","empty");
info.insert("mediafile",mediafile);
info.insert("price",QString::fromStdString(Wt::asString(myformmodelnew_->value(PriceField)).toUTF8()));
info.insert("description",QString::fromStdString(Wt::asString(myformmodelnew_->value(DescriptionField)).toUTF8()));
info.insert("sizeorquantity",QString::fromStdString(Wt::asString(myformmodelnew_->value(QuantityField)).toUTF8()));

if(Wt::asString(myformmodelnew_->value(ItemUuidField))=="") {

info.insert("uuid",mypayload->newuuid());

}

else {

info.insert("uuid",QString::fromStdString(Wt::asString(myformmodelnew_->value(ItemUuidField)).toUTF8()));


}

if(Wt::asString(myformmodelnew_->value(StandardizedField))=="") {

info.insert("standardized","later");

}

else {

info.insert("standardized",QString::fromStdString(Wt::asString(myformmodelnew_->value(StandardizedField)).toUTF8()));

}

if(Wt::asString(myformmodelnew_->value(StandardizedMediaField))=="") {

info.insert("standardizedavatar","later");

}

else {

info.insert("standardizedavatar",QString::fromStdString(Wt::asString(myformmodelnew_->value(StandardizedMediaField)).toUTF8()));

}

if(Wt::asString(myformmodelnew_->value(ManufacturerField))=="") {

info.insert("manufacturer","later");

}

else {

info.insert("manufacturer",QString::fromStdString(Wt::asString(myformmodelnew_->value(ManufacturerField)).toUTF8()));

}

return info;



}

QVariantMap MapperSoftware::myuserDataupdate(QString mediafile) {


QVariantMap info;
info.insert("user",loggeduserdue);
info.insert("name",QString::fromStdString(Wt::asString(myformmodelupdate_->value(ItemNameField)).toUTF8()));
info.insert("category",QString::fromStdString(Wt::asString(myformmodelupdate_->value(CategoryField )).toUTF8()));

QString idsid = "";
std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(loggeduserdue.toStdString());
idsfile.append("/ids.txt");



if(std::ifstream is{idsfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    idsid = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

info.insert("ids_id",idsid);
info.insert("itemsid",QString::fromStdString(retainuniqueid.toUTF8()));
info.insert("mediafile",mediafile);
info.insert("price",QString::fromStdString(Wt::asString(myformmodelupdate_->value(PriceField)).toUTF8()));
info.insert("description",QString::fromStdString(Wt::asString(myformmodelupdate_->value(DescriptionField)).toUTF8()));
info.insert("sizeorquantity",QString::fromStdString(Wt::asString(myformmodelupdate_->value(QuantityField)).toUTF8()));

if(Wt::asString(myformmodelupdate_->value(ItemUuidField))=="") {

info.insert("uuid",mypayload->newuuid());

}

else {

info.insert("uuid",QString::fromStdString(Wt::asString(myformmodelupdate_->value(ItemUuidField)).toUTF8()));


}

if(Wt::asString(myformmodelupdate_->value(StandardizedField))=="") {

info.insert("standardized","later");

}

else {

info.insert("standardized",QString::fromStdString(Wt::asString(myformmodelupdate_->value(StandardizedField)).toUTF8()));

}

if(Wt::asString(myformmodelupdate_->value(StandardizedMediaField))=="") {

info.insert("standardizedavatar","later");

}

else {

info.insert("standardizedavatar",QString::fromStdString(Wt::asString(myformmodelupdate_->value(StandardizedMediaField)).toUTF8()));

}

if(Wt::asString(myformmodelupdate_->value(ManufacturerField))=="") {

info.insert("manufacturer","later");

}

else {

info.insert("manufacturer",QString::fromStdString(Wt::asString(myformmodelupdate_->value(ManufacturerField)).toUTF8()));

}

return info;



}



void MapperSoftware::initModelnew() {


    // Create a category model.
 //   unsigned categoryModelRows = mycategories.size() + 1;
  //  const unsigned categoryModelColumns = 1;
 //   categoryModelnew_ = std::make_shared<Wt::WStandardItemModel>(categoryModelRows, categoryModelColumns);
     categoryModelnew_->clear();
     categoryModelnew_->appendRow(std::make_unique<Wt::WStandardItem>());

    for (CategoryMap::const_iterator i = mycategories.begin(); i != mycategories.end(); ++i) {


        const std::vector<std::string>& thecats = i->second;


        for (unsigned j = 0; j < thecats.size(); ++j) {

        categoryModelnew_->appendRow(std::make_unique<Wt::WStandardItem>(thecats[j]));


        }

    }


    // The initial text shown in the category combo box should be an empty
    // string.
  /*  int row = 0;
    categoryModelnew_->setData(row, 0, std::string(" "), Wt::ItemDataRole::Display);
    categoryModelnew_->setData(row, 0, std::string(), Wt::ItemDataRole::User);

    // For each category, update the model based on the key (corresponding
    // to the uuid):
    // - set the uuid for the display role,
    // - set the category names for the user role.
    row = 1;
    for (CategoryMap::const_iterator i = mycategories.begin();
                                    i != mycategories.end(); ++i) {
        categoryModelnew_->setData(row, 0, i->second, Wt::ItemDataRole::Display);
        categoryModelnew_->setData(row++, 0, i->first, Wt::ItemDataRole::User);
    }
   */

    return;

}

void MapperSoftware::initModelupdate() {


    // Create a category model.
 //   unsigned categoryModelRows = mycategories.size() + 1;
  //  const unsigned categoryModelColumns = 1;
 //   categoryModelupdate_ = std::make_shared<Wt::WStandardItemModel>(categoryModelRows, categoryModelColumns);
     categoryModelupdate_->clear();
     categoryModelupdate_->appendRow(std::make_unique<Wt::WStandardItem>());

    for (CategoryMap::const_iterator i = mycategories.begin(); i != mycategories.end(); ++i) {


        const std::vector<std::string>& thecats = i->second;


        for (unsigned j = 0; j < thecats.size(); ++j) {

        categoryModelupdate_->appendRow(std::make_unique<Wt::WStandardItem>(thecats[j]));


        }

    }


    // The initial text shown in the category combo box should be an empty
    // string.
  /*  int row = 0;
    categoryModelupdate_->setData(row, 0, std::string(" "), Wt::ItemDataRole::Display);
    categoryModelupdate_->setData(row, 0, std::string(), Wt::ItemDataRole::User);

    // For each category, update the model based on the key (corresponding
    // to the uuid):
    // - set the uuid for the display role,
    // - set the category names for the user role.
    row = 1;
    for (CategoryMap::const_iterator i = mycategories.begin();
                                    i != mycategories.end(); ++i) {
        categoryModelupdate_->setData(row, 0, i->second, Wt::ItemDataRole::Display);
        categoryModelupdate_->setData(row++, 0, i->first, Wt::ItemDataRole::User);
    }
   */

    return;

}

void MapperSoftware::initializeModels() {
    // Create a category model.
  /*  unsigned categoryModelRows = categories.size() + 1;
    const unsigned categoryModelColumns = 1;
    categoryModel_ =
      std::make_shared<Wt::WStandardItemModel>(categoryModelRows, categoryModelColumns); */

    // The initial text shown in the category combo box should be an empty
    // string.
    int row = 0;
    categoryModelnew_->setData(row, 0, std::string(" "), Wt::ItemDataRole::Display);
    categoryModelnew_->setData(row, 0, std::string(), Wt::ItemDataRole::User);


    row = 1;
    for (CategoryMap::const_iterator i = categories.begin();
                                    i != categories.end(); ++i) {
        categoryModelnew_->setData(row, 0, i->second, Wt::ItemDataRole::Display);
        categoryModelnew_->setData(row++, 0, i->first, Wt::ItemDataRole::User);
    }

    return;

}

std::shared_ptr<Wt::WValidator> MapperSoftware::createShortTextValidator(const std::string& field) {
    auto v = std::make_shared<Wt::WLengthValidator>();
    v->setMandatory(true);
    v->setMinimumLength(1);
    v->setMaximumLength(MAX_LENGTH);
    return v;
}

std::shared_ptr<Wt::WValidator> MapperSoftware::createShortNTextValidator(const std::string& field) {
    auto v = std::make_shared<Wt::WLengthValidator>();
    v->setMandatory(false);
    v->setMinimumLength(1);
    v->setMaximumLength(MAX_LENGTH);
    return v;
}

std::shared_ptr<Wt::WValidator> MapperSoftware::createLongTextValidator(const std::string& field) {
    auto v = std::make_shared<Wt::WLengthValidator>();
    v->setMandatory(true);
    v->setMinimumLength(1);
    v->setMaximumLength(MAX_LONGLENGTH);
    return v;
}

std::shared_ptr<Wt::WValidator> MapperSoftware::createBasicValidator() {
    auto v = std::make_shared<Wt::WLengthValidator>();
    v->setMandatory(true);
    return v;
}

std::shared_ptr<Wt::WValidator> MapperSoftware::createOptionalValidator() {
    auto v = std::make_shared<Wt::WLengthValidator>();
    v->setMandatory(false);
    return v;
}


void MapperSoftware::createviewnew() {


  //  mytemplateformview_ = nullptr;
   // mytemplateformview_ = new Wt::WTemplateFormView();
    auto view = Wt::WString::tr("appview-view14");
    mytemplateformview_->setTemplateText(view);
    mytemplateformview_->addFunction("id", &Wt::WTemplate::Functions::id);
    mytemplateformview_->addFunction("block", &Wt::WTemplate::Functions::id);

 /*    auto categoryCB = std::make_unique<Wt::WComboBox>();
    auto categoryCB_ = categoryCB.get();
    categoryCB->setModel(categoryModel());

    categoryCB_->activated().connect([=] {
        std::string myindexdata = categoryData(categoryCB_->currentIndex());

    });

   mytemplateformview_->setFormWidget(CategoryField, std::move(categoryCB),
        [=] { // updateViewValue()


            std::string code =
                Wt::asString(myformmodel_->value(CategoryField)).toUTF8();
            int row = categoryModelRow(code);
            categoryCB_->setCurrentIndex(row);

        },

        [=] { // updateModelValue()
            std::string data = categoryData(categoryCB_->currentIndex());
            myformmodel_->setValue(CategoryField, data);
        }); */

    auto categoryCB = std::make_unique<Wt::WComboBox>();
    categoryCB->setModel(categoryModelnew());
     mytemplateformview_->setFormWidget(CategoryField, std::move(categoryCB));



    mytemplateformview_->setFormWidget(ItemNameField ,
                  std::make_unique<Wt::WLineEdit>());


    mytemplateformview_->setFormWidget(QuantityField,
                  std::make_unique<Wt::WLineEdit>());

    mytemplateformview_->setFormWidget(PriceField ,
                  std::make_unique<Wt::WLineEdit>());


    auto desTA = std::make_unique<Wt::WTextArea>();
    desTA->setColumns(40);
    desTA->setRows(5);
    mytemplateformview_->setFormWidget(DescriptionField,std::move(desTA));


  /*  mytemplateformview_->setFormWidget(MediaField,
                  std::make_unique<Wt::WLineEdit>());  */


    mytemplateformview_->setFormWidget(StandardizedField,
                  std::make_unique<Wt::WLineEdit>());

    mytemplateformview_->setFormWidget(StandardizedMediaField,
                  std::make_unique<Wt::WLineEdit>());

    mytemplateformview_->setFormWidget(ItemUuidField,
                  std::make_unique<Wt::WLineEdit>());

    mytemplateformview_->setFormWidget(ManufacturerField,
                  std::make_unique<Wt::WLineEdit>());
    /*
     * Title & Buttons
     */
    Wt::WString title = Wt::WString("Upload New Item");
    mytemplateformview_->bindString("title", title);



    Wt::WLink linkarrowleftexitnewitemaaa = Wt::WLink(Wt::LinkType::InternalPath, "/back");
    arrowleftexitnewitemanchor_ = nullptr;
    arrowleftexitnewitemtxt_ = nullptr;
    arrowleftexitnewitemanchor_ = new Wt::WAnchor(linkarrowleftexitnewitemaaa ,"");
    arrowleftexitnewitemanchor_ ->setStyleClass("ericcolors1");
    arrowleftexitnewitemanchor_ ->setAttributeValue("aria-current","page");
    arrowleftexitnewitemtxt_ = arrowleftexitnewitemanchor_->addNew<Wt::WText>(arrowlefticon);
    arrowleftexitnewitemtxt_->show();
    arrowleftexitnewitemanchor_->clicked().connect([=] {

     menu->itemAt(0)->menu()->itemAt(3)->hide();
     menu->itemAt(0)->menu()->itemAt(1)->show();
     menu->itemAt(0)->menu()->itemAt(2)->hide();
     menu->itemAt(0)->menu()->select(0);




      });


    mytemplateformview_->bindEmpty("AnchorBItemsPage");
    std::unique_ptr<Wt::WAnchor> arrowleftexitnewitemanchor(arrowleftexitnewitemanchor_);
    mytemplateformview_->bindWidget("AnchorBItemsPage", std::move(arrowleftexitnewitemanchor));


   // mytemplateformview_->bindString("submit-info", Wt::WString());
    mytemplateformview_->bindEmpty("whichpage1");
    mytemplateformview_->bindString("whichpage1", Wt::WString("New item page"));






    auto button = mytemplateformview_->bindWidget("submit-button", std::make_unique<Wt::WPushButton>("Next"));


    button->clicked().connect(this, &MapperSoftware::processitemnew);

    mytemplateformview_->updateView(myformmodelnew_.get());
    return;
}




void MapperSoftware::processitemnew() {
    mytemplateformview_->updateModel(myformmodelnew_.get());

    if (myformmodelnew_->validate()) {
        // Do something with the data in the model: show it.
    /*   mytemplateformview_->bindString("submit-info",
                   Wt::WString("Upload Item with data ")
                   + userDatanew(), Wt::TextFormat::Plain);
        // Update the view: Delete any validation message in the view, etc. */

        mytemplateformview_->updateView(myformmodelnew_.get());
        // Set the focus on the first field in the form.

     arrowleftnewitemtxt_->show();
     arrowleftupdateitemtxt_->hide();
     menu->itemAt(0)->menu()->itemAt(1)->hide();
     menu->itemAt(0)->menu()->itemAt(2)->hide();
     menu->itemAt(0)->menu()->select(3);

    } else {
      //  mytemplateformview_->bindEmpty("submit-info"); // Delete the previous user data.
        mytemplateformview_->updateView(myformmodelnew_.get());
    }

    return;
}


void MapperSoftware::createviewupdate() {



   // updatetemplateformview_ = nullptr;
   // updatetemplateformview_ = new Wt::WTemplateFormView();

    auto view = Wt::WString::tr("uiux1-view2");
    updatetemplateformview_->setTemplateText(view);
    updatetemplateformview_->addFunction("id", &Wt::WTemplate::Functions::id);
    updatetemplateformview_->addFunction("block", &Wt::WTemplate::Functions::id);


    combocategoryupdate = new Wt::WComboBox();
    combocategoryupdate ->setModel(categoryModelupdate());
    std::unique_ptr<Wt::WComboBox> categoryCB(combocategoryupdate);
    updatetemplateformview_->setFormWidget(CategoryField, std::move(categoryCB));


    updateitemname = new Wt::WLineEdit();
    std::unique_ptr<Wt::WLineEdit> updateitemname_(updateitemname);
    updatetemplateformview_->setFormWidget(ItemNameField ,
                  std::move(updateitemname_) );


    updatequantity = new Wt::WLineEdit();
    std::unique_ptr<Wt::WLineEdit> updatequantity_(updatequantity);
    updatetemplateformview_->setFormWidget(QuantityField,
                  std::move(updatequantity_));

    updateprice = new Wt::WLineEdit();
    std::unique_ptr<Wt::WLineEdit> updateprice_(updateprice);
    updatetemplateformview_->setFormWidget(PriceField ,
                  std::move(updateprice_));



    updateitemdescription = new Wt::WTextArea();
    updateitemdescription->setColumns(40);
    updateitemdescription->setRows(5);
    std::unique_ptr<Wt::WTextArea> desTA(updateitemdescription);
    updatetemplateformview_->setFormWidget(DescriptionField,std::move(desTA));


    updatestandardized = new Wt::WLineEdit();
    std::unique_ptr<Wt::WLineEdit> updatestandardized_(updatestandardized);
    updatetemplateformview_->setFormWidget(StandardizedField,
                  std::move(updatestandardized_));

    updatestandardizedmedia = new Wt::WLineEdit();
    std::unique_ptr<Wt::WLineEdit> updatestandardizedmedia_(updatestandardizedmedia);
    updatetemplateformview_->setFormWidget(StandardizedMediaField,
                  std::move(updatestandardizedmedia_));

    updateitemuuid = new Wt::WLineEdit();
    std::unique_ptr<Wt::WLineEdit> updateitemuuid_(updateitemuuid);
    updatetemplateformview_->setFormWidget(ItemUuidField,
                  std::move(updateitemuuid_));

    updatemanufacturer = new Wt::WLineEdit();
    std::unique_ptr<Wt::WLineEdit> updatemanufacturer_(updatemanufacturer);
    updatetemplateformview_->setFormWidget(ManufacturerField,
                  std::move(updatemanufacturer_));

    /*
     * Title & Buttons
     */
    Wt::WString title = Wt::WString("Update Item");
    updatetemplateformview_->bindString("title", title);


    auto button = updatetemplateformview_->bindWidget("submit-button", std::make_unique<Wt::WPushButton>("Next"));

 // updatetemplateformview_->bindString("submit-info", Wt::WString());
    updatetemplateformview_->bindEmpty("whichpage1");
    updatetemplateformview_->bindString("whichpage1", Wt::WString("Item update page"));

    Wt::WLink linkarrowleftexitupdateitemaaa = Wt::WLink(Wt::LinkType::InternalPath, "/back");
    arrowleftexitupdateitemanchor_ =nullptr;
    arrowleftexitupdateitemtxt_ =nullptr;
    arrowleftexitupdateitemanchor_ = new Wt::WAnchor(linkarrowleftexitupdateitemaaa ,"");
    arrowleftexitupdateitemanchor_ ->setStyleClass("ericcolors1");
    arrowleftexitupdateitemanchor_ ->setAttributeValue("aria-current","page");
    arrowleftexitupdateitemtxt_ = arrowleftexitupdateitemanchor_->addNew<Wt::WText>(arrowlefticon);
    arrowleftexitupdateitemtxt_->show();
    arrowleftexitupdateitemanchor_->clicked().connect([=] {



     menu->itemAt(0)->menu()->itemAt(3)->hide();
     menu->itemAt(0)->menu()->itemAt(1)->show();
     menu->itemAt(0)->menu()->itemAt(2)->hide();
     menu->itemAt(0)->menu()->select(0);




      });

    updatetemplateformview_->bindEmpty("AnchorBItemsPage");
    std::unique_ptr<Wt::WAnchor> arrowleftexitupdateitemanchor(arrowleftexitupdateitemanchor_);
    updatetemplateformview_->bindWidget("AnchorBItemsPage", std::move(arrowleftexitupdateitemanchor));

    /*  nextbtnitemnew_ = nullptr;
      nextbtnitemnew_ = new Wt::WPushButton("Next");
      std::unique_ptr<Wt::WPushButton> nextbtnitemnew(nextbtnitemnew_);
      nextbtnitemnew->clicked().connect(this, &MapperSoftware::processitemnew);
      mytemplateformview_->bindWidget("submit-button", std::move(nextbtnitemnew));


      nextbtnitemupdate_ = nullptr;
      nextbtnitemupdate_ = new Wt::WPushButton("Next");
      nextbtnitemupdate_->clicked().connect(this, &MapperSoftware::processitemupdate);
      std::unique_ptr<Wt::WPushButton> nextbtnitemupdate(nextbtnitemupdate_);
      mytemplateformview_->bindWidget("submit-button", std::move(nextbtnitemupdate)); */





    button->clicked().connect(this, &MapperSoftware::processitemupdate);

    updatetemplateformview_->updateView(myformmodelupdate_.get());
    return;
}




void MapperSoftware::processitemupdate() {
    updatetemplateformview_->updateModel(myformmodelupdate_.get());

    if (myformmodelupdate_->validate()) {
        // Do something with the data in the model: show it.
     /*   updatetemplateformview_->bindString("submit-info",
                   Wt::WString("Upload Item with data ")
                   + userDataupdate(), Wt::TextFormat::Plain); */
     // Update the view: Delete any validation message in the view, etc.

      updatetemplateformview_->updateView(myformmodelupdate_.get());
      // Set the focus on the first field in the form.

      arrowleftnewitemtxt_->hide();
      arrowleftupdateitemtxt_->show();
      menu->itemAt(0)->menu()->itemAt(1)->hide();
      menu->itemAt(0)->menu()->itemAt(2)->hide();
      menu->itemAt(0)->menu()->select(3);

    }

    else {
      //  updatetemplateformview_->bindEmpty("submit-info"); // Delete the previous user data.
        updatetemplateformview_->updateView(myformmodelupdate_.get());
    }

    return;
}













MapperSoftware::CategoryMap MapperSoftware::getCategoryMap(std::map< std::string, std::vector<std::string> > mymap) {


  return mymap;


}



void MapperSoftware::timeoutnotifytimer3() {

if(timer3->isActive()) {


}

else {





menu->itemAt(0)->menu()->select(0);
menu->itemAt(0)->menu()->itemAt(1)->show();
menu->itemAt(0)->menu()->itemAt(2)->hide();
menu->itemAt(0)->menu()->itemAt(3)->hide();
notifytimer3->stop();



myformmodelnew_->reset();
//mytemplateformview_->updateModel(myformmodelnew_.get());

if(myformmodelnew_->validate()) {


mytemplateformview_->updateView(myformmodelnew_.get());
//qDebug() << "myformmodelnew_ model validated!" << Qt::endl;


      }

else {

mytemplateformview_->updateView(myformmodelnew_.get());
//qDebug() << "myformmodelnew_ model was not validated!" << Qt::endl;


      }


myformmodelupdate_.get()->reset();
//updatetemplateformview_->updateModel(myformmodelupdate_.get());

if(myformmodelupdate_->validate()) {

updatetemplateformview_->updateView(myformmodelupdate_.get());
//qDebug() << "myformmodelupdate_ model validated!" << Qt::endl;


    }

else {

updatetemplateformview_->updateView(myformmodelupdate_.get());


    }






}


}



void MapperSoftware::timeouttimer3() {

containerupload_ = nullptr;
containerupload_ = new Wt::WContainerWidget();
fu = nullptr;
fu = containerupload_->addNew<Wt::WFileUpload>();
   // fu->setMinimumSize(Wt::WLength::Auto,Wt::WLength::Auto);
fu->setFilters("image/*,video/*,.csv,.pdf");
fu->setProgressBar(std::make_unique<Wt::WProgressBar>());
fu->setMargin(10, Wt::Side::Right);
uploadButton = containerupload_->addNew<Wt::WPushButton>("Upload");
uploadButton->setMargin(10, Wt::Side::Left | Wt::Side::Right);
out = nullptr;
out = containerupload_->addNew<Wt::WText>();
fu->stealSpooledFile();
// Upload when the button is clicked.
uploadButton->clicked().connect([&] {
  // fu->upload();

if(doupdateitem) {

douploadnewitem = false;
timeritemnew->start();
networkutils->itemexistence(myuserDataupdate());


    }

else {

douploadnewitem = false;
timeritemnew->start();
networkutils->itemexistence(myuserDatanew());



  }

  // uploadButton->disable();
});

// Upload automatically when the user entered a file.
fu->changed().connect([=] {
//   fu->upload();
 //  uploadButton->disable();
 //  out->setText("File upload is changed.");


});

bool doupload =false;

// React to a succesfull upload.
fu->uploaded().connect([&] {


   for (const Wt::Http::UploadedFile & n : fu->uploadedFiles()) {

          qInfo() << "uploaded is: " << QString::fromStdString(n.clientFileName()) << Qt::endl;
          //qDebug() << "content type is: " << QString::fromStdString(n.contentType()) << Qt::endl;
          //qDebug() << "spooled file name is: " << QString::fromStdString(n.spoolFileName()) << Qt::endl;
          //qDebug() << "uuid payload is: " << mypayload->newuuid() << Qt::endl;
           //qDebug() << "long date payload is: " << mypayload->longdatetime() << Qt::endl;
          //qDebug() << "newfile is: " << mypayload->newfilename() << Qt::endl;
           //qDebug() << "mime type is: " << mypayload->fileSuffixFromPath(QString::fromStdString(n.clientFileName())) << Qt::endl;
           fu->stealSpooledFile();
          QString newfilename = mypayload->newfilename()+"."+mypayload->fileSuffixFromPath(QString::fromStdString(n.clientFileName()));
          qInfo() << "newfilename is: " << newfilename << Qt::endl;
             out->setText(Wt::WString(n.clientFileName())+" upload successful!");
           QString dirspool = mypayload->filePath(QString::fromStdString(n.spoolFileName()));
           qInfo() << "dirspool is: " << dirspool << Qt::endl;
           qInfo() << "path is: " << QString::fromStdString(mypayload->UPLOAD_DIRIDS)+"/"+newfilename << Qt::endl;

           QString destDir = QString::fromStdString(mypayload->UPLOAD_DIRIDS);
           // QString newfilename = mypayload->newfilename() + "." +
           //                       mypayload->fileSuffixFromPath(QString::fromStdString(n.clientFileName()));

           QString finalPath, err;
           QString cleanfile = mypayload->sanitizeFileName(newfilename, "_");
           bool succ = mypayload->moveOrCopySpooledFile(n.spoolFileName(), destDir, cleanfile, finalPath, err);

          if( succ) {
         //  if( mypayload->renamefile(dirspool,QString::fromStdWString(RandomLinks(LOCALSERVERTMP))+"/"+cleanfile)) {
               //qDebug() << "Rename success" << Qt::endl;

            if(doupdateitem) {

              timeritemnew->start();
              networkutils->updateitem(myuserDataupdate(cleanfile));
              timer3->start();
              notifytimer3->start();
              doupload=true;

              QString myfile =  QString::fromStdString(mypayload->UPLOAD_DIRIDS)+"/"+QString::fromStdString(retainmedianame.toUTF8());
              std::string oldavatarfile = myfile.toStdString();
              qInfo() << "old file is: " << oldavatarfile << Qt::endl;

              QFile file(QString::fromStdString(oldavatarfile));

              if (!file.open(QIODevice::ReadOnly | QIODevice::WriteOnly)) {

               qInfo() << "file could not be opened: " << QString::fromStdString(oldavatarfile) << Qt::endl;

                }

               else {

                   if(file.remove()) {

                    puts( "File successfully deleted" );
                    qInfo() << "File successfully deleted" << Qt::endl;

                    }

                   else {

                    perror( "Error deleting file:" );

                    qInfo() << "Error deleting file" << Qt::endl;


                    }


                 }

               file.close();


                 }

            else {

            qInfo() << "new item upload"  << Qt::endl;
            timeritemnew->start();
            networkutils->newitem(myuserDatanew(cleanfile));
            timer3->start();
            notifytimer3->start();
            doupload=true;


               }





           }

           else {

             qInfo() << "Rename failed" << Qt::endl;

           }




       }

    //qDebug() << "doupload is: " << doupload << Qt::endl;

    if(doupload==false) {

      if(doupdateitem) {

       timeritemnew->start();
       networkutils->updateitem(myuserDataupdate(QString::fromStdString(retainmedianame.toUTF8())));
       timer3->start();
       notifytimer3->start();
       doupload=true;

        }

    else {


    customToast("appview-view20","Please select a file to upload!");
    timerdeletewidget->start();
    timer3->start();

      }

    }

    //qDebug() << "content description is: " << QString::fromStdString(fu->contentDescription().toUTF8()) << Qt::endl;




});

// React to a file upload problem.
fu->fileTooLarge().connect([=] {
   out->setText("File is too large.");
});


uploadtemplate_->bindEmpty("uploadfile");
 //qDebug() << "timeouttimer3() running" << Qt::endl;
std::unique_ptr<Wt::WContainerWidget>  containerupload(containerupload_);
uploadtemplate_->bindWidget("uploadfile", std::move(containerupload));
//   std::unique_ptr<Wt::WTemplate>  uploadtemplate(uploadtemplate_);
//  root()->addWidget(std::move(uploadtemplate));
//  uploadtemplate_->show();



}





void MapperSoftware::populateSubMenu(Wt::WMenu *menu)
{

  menu->addItem("All Items",deferCreate([this]{ return allitems(); }));
  //   deferCreate([this]{ return allitems(); }))->setPathComponent("");
  menu->addItem("New Item",deferCreate([this]{ return newitem(); }));
  menu->addItem("Update Item", deferCreate([this]{ return updateitem(); }));
  menu->addItem("File Upload", deferCreate([this]{ return newmenuitem(); }));
  menu->addItem("Settings", deferCreate([this]{ return thesettings(); }));
  menu->addItem("Requests", deferCreate([this]{ return therequests(); }));
  menu->addItem("Help Page", deferCreate([this]{ return thehelp1(); }));

}




Wt::WMenuItem *MapperSoftware::addToMenu(Wt::WMenu *menu,
                                        const Wt::WString& name)
{

  auto result = std::make_unique<Wt::WContainerWidget>();

  auto subMenuPtr = std::make_unique<Wt::WMenu>(contentsStack_);
  auto subMenu = subMenuPtr.get();
  auto itemPtr = std::make_unique<Wt::WMenuItem>(name);
  itemPtr->setMenu(std::move(subMenuPtr));

  auto item = menu->addItem(std::move(itemPtr));
  subMenu->addStyleClass("nav-stacked submenu");

  subMenu->itemSelected().connect(this, &MapperSoftware::closeMenu);

  subMenu->setInternalPathEnabled("/" + item->pathComponent());

  populateSubMenu(subMenu);

  return item;
}

void MapperSoftware::handlemenuitems(Wt::WMenuItem * myitem) {

//qDebug() << "Menu text is: " << QString::fromStdString(myitem->text().toUTF8()) << Qt::endl;

if(myitem->text()=="New Item" || myitem->text()=="Update Item") {

if(timercarousel->isActive()) {

timercarousel->stop();
timergetdatauser->stop();


}


}
else if(myitem->text()=="All Items") {








if(!timercarousel->isActive()) {

/*


myformmodelnew_.get()->reset();
myformmodelupdate_.get()->reset();
mytemplateformview_->updateView(myformmodelnew_.get());
updatetemplateformview_->updateView(myformmodelupdate_.get());


*/


//timercarousel->start();


}


}

else {


}

}

void MapperSoftware::toggleMenu()
{
  if (menuOpen_) {
    closeMenu();
  } else {
    openMenu();
  }
}

void MapperSoftware::openMenu()
{
  if (menuOpen_)
    return;

  openMenuButton_->setText(closeMenuText);
  appvieweric_->addStyleClass("menu-open");

  menuOpen_ = true;
}

void MapperSoftware::closeMenu()
{
  if (!menuOpen_)
    return;

  openMenuButton_->setText(showMenuText);
  appvieweric_->removeStyleClass("menu-open");

  menuOpen_ = false;
}





std::unique_ptr<Wt::WWidget> MapperSoftware::allitems() {

std::unique_ptr<Wt::WTemplate> result(allitems_);
return result;


}


std::unique_ptr<Wt::WWidget> MapperSoftware::newmenuitem() {


  std::unique_ptr<Wt::WTemplate>  uploadtemplate(uploadtemplate_);
  return uploadtemplate;


 }


std::unique_ptr<Wt::WWidget> MapperSoftware::newitem() {


    std::unique_ptr<Wt::WTemplateFormView>  theview(mytemplateformview_);
    return theview;

   // std::unique_ptr<Wt::WTemplate>  uploadtemplate(uploadtemplate_);
   // return std::move(uploadtemplate);

 //   std::unique_ptr<Wt::WTemplate> result(newitem_);
 //   return std::move(result);

}

std::unique_ptr<Wt::WWidget> MapperSoftware::updateitem() {


    std::unique_ptr<Wt::WTemplateFormView>  theview(updatetemplateformview_);
    return theview;



}


std::unique_ptr<Wt::WWidget> MapperSoftware::thesettings() {


std::unique_ptr<Wt::WTemplate>  thesett(allsettings_);
return thesett;

}

std::unique_ptr<Wt::WWidget> MapperSoftware::therequests() {

std::unique_ptr<Wt::WTemplate>  thereqs(allrequests_);
return thereqs;

}


std::unique_ptr<Wt::WWidget> MapperSoftware::thehelp1() {

std::unique_ptr<Wt::WTemplate>  thehelptpl1(helptpl1_);
return thehelptpl1;


}




void MapperSoftware::restdata(std::string rawData) {

mypayload->processRequest(QByteArray::fromStdString(rawData));
QJsonObject mydearpayload = mypayload->m_requestJson.toObject();




if(timerloginuser->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

   }

else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


   }

}

else if(timerregisteruser->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

       }

else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


     }


}

else if(timerretrievepassword->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timeritemnew->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

}

else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timergeneric1->isActive() ) {

if (mydearpayload.contains("error")) {

 myerrormessage->clear();
 mysuccessmessage->clear();
 myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

                }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


     }

 }

else if(timergeneric2->isActive() ) {

if (mydearpayload.contains("error")) {

 myerrormessage->clear();
 mysuccessmessage->clear();
 myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

                }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


     }

 }

else if(timerdeleteitem->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timerupdateuser->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewcategory->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletecategory->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timerneworder->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}


else if(timerdeleteorder->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewlogistics->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletelogistics->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewdelivery->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletedelivery->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewlocation->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletelocation->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewmetrics->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletemetrics->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


    }

}

else if(timerupdatepassword->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

}

else {


     mysuccessmessage->clear();
     myerrormessage->clear();
     mysuccessmessage->append(QString::fromStdString(rawData));


}

}

else if(timerupdatephonenumber->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

}

else {


     mysuccessmessage->clear();
     myerrormessage->clear();
     mysuccessmessage->append(QString::fromStdString(rawData));


}

}

else if(timerlogout->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

}

else {


     mysuccessmessage->clear();
     myerrormessage->clear();
     mysuccessmessage->append(QString::fromStdString(rawData));


}

}

else if(timergetdataall->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

}

else {


     mysuccessmessage->clear();
     myerrormessage->clear();
     mysuccessmessage->append(QString::fromStdString(rawData));


}

}

else if(timergetdatauser->isActive() ) {

if (mydearpayload.contains("error")) {

 myerrormessage->clear();
 mysuccessmessage->clear();
 myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

                }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
mysuccessmessage->append(QString::fromStdString(rawData));


     }

 }

else if(timercarousel->isActive()) {

if (mydearpayload.contains("error")) {


     myerrormessage->clear();
     mysuccessmessage->clear();
     myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

       }

else {


     mysuccessmessage->clear();
     myerrormessage->clear();
     mysuccessmessage->append(QString::fromStdString(rawData));


   }

}





else {


}


 }



void MapperSoftware::restnetworkErrordata(std::string rawData) {




if(timerloginuser->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));




}

else if(timerregisteruser->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}

else if(timerretrievepassword->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}



else if(timeritemnew->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}

else if(timergeneric1->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}

else if(timergeneric2->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}

else if(timerdeleteitem->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}

else if(timerupdateuser->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));



}

else if(timernewcategory->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timerdeletecategory->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timerneworder->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}


else if(timerdeleteorder->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timernewlogistics->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timerdeletelogistics->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timernewdelivery->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timerdeletedelivery->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timernewlocation->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timerdeletelocation->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timernewmetrics->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timerdeletemetrics->isActive()) {


myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}

else if(timerupdatepassword->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}

else if(timerupdatephonenumber->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}

else if(timerlogout->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}

else if(timergetdataall->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}

else if(timergetdatauser->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}


else if(timercarousel->isActive()) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


}






else {


}





 }

void MapperSoftware::restserverErrordata(std::string rawData) {

// QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(rawData));
//const QString finaljson = doc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug() << "in wt slot final json is: " << qPrintable(finaljson) << Qt::endl;
mypayload->processRequest(QByteArray::fromStdString(rawData));
QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
//qDebug() << "in wt slot restserverErrordata  m_requestJson  is: " <<  mypayload->m_requestJson  << Qt::endl;



if(timerloginuser->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


 }

else {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));



 }

}

else if(timerregisteruser->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


   }

else {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));



  }

}

else if(timerretrievepassword->isActive()) {

 if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


   }

else {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));



     }

}



else if(timeritemnew->isActive() ) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


     }

else {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}



}

else if(timergeneric1->isActive() ) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


     }

else {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}



}

else if(timergeneric2->isActive() ) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


     }

else {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}



}

else if(timerdeleteitem->isActive() ) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


     }

else {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}



}

else if(timerupdateuser->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewcategory->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletecategory->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timerneworder->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}


else if(timerdeleteorder->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewlogistics->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletelogistics->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewdelivery->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletedelivery->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewlocation->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletelocation->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timernewmetrics->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}

else if(timerdeletemetrics->isActive()) {

if (mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());

     }

 else {

mysuccessmessage->clear();
myerrormessage->clear();
myerrormessage->append(QString::fromStdString(rawData));


    }

}


else if(timerupdatepassword->isActive()) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


      }

else {

   myerrormessage->clear();
   mysuccessmessage->clear();
   myerrormessage->append(QString::fromStdString(rawData));


 }


}

else if(timerupdatephonenumber->isActive()) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


      }

else {

   myerrormessage->clear();
   mysuccessmessage->clear();
   myerrormessage->append(QString::fromStdString(rawData));


 }


}


else if(timerlogout->isActive()) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


      }

else {

   myerrormessage->clear();
   mysuccessmessage->clear();
   myerrormessage->append(QString::fromStdString(rawData));


 }


}



else if(timergetdataall->isActive()) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


      }

else {

   myerrormessage->clear();
   mysuccessmessage->clear();
   myerrormessage->append(QString::fromStdString(rawData));


 }


}


else if(timergetdatauser->isActive() ) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


     }

else {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(QString::fromStdString(rawData));

}



}

else if(timercarousel->isActive()) {

if(mydearpayload.contains("error")) {

myerrormessage->clear();
mysuccessmessage->clear();
myerrormessage->append(mydearpayload.value("error").toObject().value("desc").toString());


      }

else {

   myerrormessage->clear();
   mysuccessmessage->clear();
   myerrormessage->append(QString::fromStdString(rawData));


 }


}




else {



}



}





std::shared_ptr<Wt::WValidator> MapperSoftware::createNameValidator() {
        auto v = std::make_shared<Wt::WLengthValidator>();
        v->setMandatory(true);
        v->setMinimumLength(1);
        v->setMaximumLength(MAX_LENGTH);
        return v;
    }

std::shared_ptr<Wt::WValidator> MapperSoftware::createStrValidator(int maxlen) {
        auto v = std::make_shared<Wt::WLengthValidator>();
        v->setMandatory(true);
        v->setMinimumLength(1);
        v->setMaximumLength(maxlen);
        return v;
    }


void MapperSoftware::convertToDate(Wt::WStandardItem *item) {

    Wt::WString dateDisplayFormat(Wt::WString("MMM dd, yyyy"));
    Wt::WString dateEditFormat(Wt::WString("dd-MM-yyyy"));

    Wt::WDate d = Wt::WDate::fromString(item->text(), dateEditFormat);
    item->setData(Wt::cpp17::any(d), Wt::ItemDataRole::Display);

}

void MapperSoftware::convertToNumber(Wt::WStandardItem *item) {


    int i = Wt::asNumber(item->text());
    item->setData(Wt::cpp17::any(i), Wt::ItemDataRole::Edit);

}



void MapperSoftware::refreshinternal() {


handleSpecificPathChange(Wt::WApplication::instance()->internalPath());

}



void MapperSoftware::handleSpecificPathChange(const std::string&)
{
    Wt::WApplication *app = Wt::WApplication::instance();
    std::string pathitems = app->internalPathNextPart("/items/");

    if (app->internalPath() == "/account-set-up/register") {

  //qDebug() << "/account-set-up/register called! " << Qt::endl;

   /* if(myaccounts_)
        myaccounts_->hide(); */
    if(accsetup_)
        accsetup_->hide();
    if(accretrieve_)
        accretrieve_->hide();
    if(accregister_ ) {

       accregister_ ->show();





       editpassreenterscope->changed().connect([&] {

          reentervalidatorscope->setRegExp("^"+editpassregscope->text());
          editpassregscope->setFocus(false);

       });

       editpassregscope->changed().connect([&] {


          reentervalidatorscope->setRegExp("^"+editpassregscope->text());
          editpassreenterscope->validate();
          editpassreenterscope->setFocus(false);

       });



     }


    }

    else if (app->internalPath() == "/account-set-up/retrieve-pin") {

   //qDebug() << "/account-set-up/retrieve-pin called! " << Qt::endl;

   /*  if(myaccounts_)
         myaccounts_->hide(); */
     if(accregister_)
        accregister_->hide();
     if(accsetup_)
         accsetup_->hide();
     if(accretrieve_)
         accretrieve_->show();



     }

  else if (app->internalPath() == "/account-set-up/") {

//qDebug() << "/account-set-up/ called! " << Qt::endl;

/* if(myaccounts_)
     myaccounts_->hide(); */
    if(accregister_)
        accregister_->hide();
    if(accretrieve_)
        accretrieve_->hide();
    if(accsetup_)
       accsetup_->show();









    }

    else if (app->internalPath() == "/account-set-up") {

     //qDebug() << "Top path is:  " << QString::fromStdString(app->internalPath()) << Qt::endl;

    }

    else if (app->internalPath() == "/my-items") {

      //qDebug() << "in /my-items my-items called! " << Qt::endl;

      if(appvieweric_->isHidden()) {

          if(accregister_)
              accregister_->hide();
          if(accretrieve_)
              accretrieve_->hide();
          if(accsetup_)
             accsetup_->show();


      }

      else {



      }



     }

    else if (app->internalPath() == "/my-items/all-items") {

   //   qDebug() << "/my-items/all-items path called! " << Qt::endl;

      if(appvieweric_->isHidden()) {

          if(accregister_)
              accregister_->hide();
          if(accretrieve_)
              accretrieve_->hide();
          if(accsetup_)
             accsetup_->show();


      }

      else {



      }



     }

    else if (app->internalPath() == "/login") {

      //qDebug() << "/login path called! " << Qt::endl;

   /*   if(appvieweric_->isVisible()) {

          if(accregister_)
              accregister_->hide();
          if(accretrieve_)
              accretrieve_->hide();
          if(accsetup_)
             accsetup_->show();

            appvieweric_->hide();


      }

      else {



      } */



     }

     else if (app->internalPath() == "/my-items/new-item") {

         //qDebug() << "/my-items/new-item called! " << Qt::endl;



      /*   if((idsstandardmodel->columnCount()>0 && idsstandardmodel->rowCount())>0) {


                  idsstandardmodel->clear();
              }  */




        }

     else if (app->internalPath() == "/my-items/update-item") {

       //qDebug() << "/my-items/update-item called! " << Qt::endl;




        }


    else if (pathitems == "name") {


        std::string itemname = app->internalPathNextPart("/items/name/");
        //qDebug() << "itemname is: " << QString::fromStdString(itemname) << Qt::endl;




       }

    else if (app->internalPath() == "/") {

     // qDebug() << "/ called! " << Qt::endl;



       }

    else if(app->internalPath() == "/helppage/loggedin") {

        menu->itemAt(0)->menu()->itemAt(0)->hide();
        menu->itemAt(0)->menu()->itemAt(1)->hide();
        menu->itemAt(0)->menu()->itemAt(2)->hide();
        menu->itemAt(0)->menu()->itemAt(3)->hide();
        menu->itemAt(0)->menu()->itemAt(4)->hide();
        menu->itemAt(0)->menu()->itemAt(5)->hide();
        menu->itemAt(0)->menu()->itemAt(6)->show();
        menu->itemAt(0)->menu()->select(6);




    }

    else if(app->internalPath() == "/helppage/loggedout") {

        menu->itemAt(0)->menu()->itemAt(0)->hide();
        menu->itemAt(0)->menu()->itemAt(1)->hide();
        menu->itemAt(0)->menu()->itemAt(2)->hide();
        menu->itemAt(0)->menu()->itemAt(3)->hide();
        menu->itemAt(0)->menu()->itemAt(4)->hide();
        menu->itemAt(0)->menu()->itemAt(5)->hide();
        menu->itemAt(0)->menu()->itemAt(6)->show();
        menu->itemAt(0)->menu()->select(6);


    }

     else {

       // qDebug() << "other path " << QString::fromStdString(app->internalPath()) << " called!" << Qt::endl;

      }
}

void MapperSoftware::setInternalBasePath(const std::string& basePath) {
  basePath_ = basePath;
  refreshinternal();
}



bool MapperSoftware::yearMonthDiffer(const Wt::WDateTime& dt1, const Wt::WDateTime& dt2) {

  return dt1.date().year() != dt2.date().year()
    || dt1.date().month() != dt2.date().month();

}



void MapperSoftware::runapp() {
    QThread *thread = QThread::currentThread();

    //qDebug() << thread;
    std::cout << QThread::currentThreadId() << std::endl;
    std::cout << std::this_thread::get_id() << std::endl;

    thread->sleep(1);
    std::cout << "finished\n";
}




void MapperSoftware::testthreads() {


    std::thread t1(runapp);
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    t1.join();






}




void MapperSoftware::thetimer1(bool timeout) {

 if(timeout) {

 //qDebug() << "timer1 timeout is true" << Qt::endl;
  //   networkutils->getuseritems("ericm");
 }

else {

 //qDebug() << "timer1 timeout is false" << Qt::endl;

 }

}

void MapperSoftware::thetimer2(bool timeout) {

    if(timeout) {

      //qDebug() << "timer2 timeout is true" << Qt::endl;
    }

   else {

      //qDebug() << "timer2 timeout is false" << Qt::endl;

    }
}

void MapperSoftware::thetimer3(bool timeout) {

    if(timeout) {

      //qDebug() << "timer3 timeout is true" << Qt::endl;
    }

   else {

     //qDebug() << "timer3 timeout is false" << Qt::endl;

    }

}



  namespace {

  int countSpaces(const std::string& line) {
    for (unsigned int pos=0; pos<line.length(); ++pos) {
      if (line[pos] != ' ')
        return pos;
    }
    return line.length();
  }

  std::string skipSpaces(const std::string& line, unsigned int count) {
    if (line.length() >= count)
      return line.substr(count);
    else
      return std::string();
  }

  }

  std::wstring RandomLinks(AppLinks thelinks)
  {
      // create stream server and get IPs
      std::unique_ptr<StreamServer> stream = std::make_unique<StreamServer>(nullptr);
      QString ip4 = stream->getLocalIp();
      QString ip6 = stream->getLocalIpV6();
      std::wstring theurl = std::wstring();

      QString chosen;
      QString listenIpval;
      if (!ip4.isEmpty()) chosen = ip4;
      else if (!ip6.isEmpty()) chosen = ip6;
      else {
          WarnL << "RandomLinks QxHttpServerController::applyListenIpFrom: StreamServer has no non-loopback IP";
          theurl = std::wstring(); // <- RETURN empty on error
      }

      // validate (refuse loopback/wildcard)
      if (chosen.isEmpty() || chosen == QLatin1String("0.0.0.0") ||
          chosen == QLatin1String("::") || chosen == QLatin1String("::1") ||
          chosen.startsWith(QLatin1String("127."))) {
          WarnL << "RandomLinks QxHttpServerController::applyListenIpFrom: refused loopback/wildcard ip: " << chosen.toStdString();
          theurl = std::wstring();
      }

      else {

        listenIpval = chosen; // store chosen IP
      //  qInfo() << "listenIpval chosen: " << ("http://"+listenIpval) << Qt::endl;
       // qInfo() << "APIURL listenIpval chosen: " << ("http://"+listenIpval + ":9642") << Qt::endl;
       // qInfo() << "UPLOADURL listenIpval chosen: " << ("http://"+listenIpval + ":8180") << Qt::endl;
       // qInfo() << "MEDIAURL listenIpval chosen: " << ("http://"+listenIpval + ":8184/media/") << Qt::endl;


        }


      // Direct string-form URLs that don't require a file
      if (thelinks == APIURL) {
          theurl = ("http://"+listenIpval + ":9642").toStdWString();
         // qInfo() << "APIURL listenIpval chosen: " << theurl << Qt::endl;
          return theurl;
      } else if (thelinks == UPLOADURL) {
          theurl = ("http://"+listenIpval + ":8180").toStdWString();
         // qInfo() << "UPLOADURL listenIpval chosen: " << theurl << Qt::endl;
          return theurl;
      } else if (thelinks == MEDIAURL) {
          theurl = ("http://"+listenIpval + ":8184/Orders_files/App/media/").toStdWString();
         // qInfo() << "MEDIAURL listenIpval chosen: " << theurl << Qt::endl;
          return theurl;
      }

      // For other cases we open a file under appRoot and pick a random line
      std::string appRoot = Wt::WApplication::appRoot(); // returns std::string
      std::string filename;

      switch (thelinks) {
          case STREAMURL:       filename = appRoot + "streamurl.txt"; break;
          case HELPURL:         filename = appRoot + "helpurl.txt"; break;
          case LOCALDATAURL:    filename = appRoot + "localdataurl.txt"; break;
          case LOCALMEDIAURL:   filename = appRoot + "localmediaurl.txt"; break;
          case LOCALSTREAMURL:  filename = appRoot + "localstreamurl.txt"; break;
          case LOCALSERVERTMP:  filename = appRoot + "localservertmp.txt"; break;
          case LOCALSERVERROOT: filename = appRoot + "localserverroot.txt"; break;
          case REGISTERTOKEN:   filename = appRoot + "registertoken.txt"; break;
          case LOGINTOKEN:      filename = appRoot + "logintoken.txt"; break;
          default:
              filename = appRoot + "apiurl.txt";
              break;
      }

      std::ifstream infile(filename);
      if (!infile.is_open()) {
          WarnL << "RandomLinks: failed to open " << filename;
          return std::wstring();
      }

      std::vector<std::string> lines;
      std::string line;
      while (std::getline(infile, line)) {
          if (!line.empty()) lines.push_back(line);
      }
      infile.close();

      if (lines.empty()) {
          WarnL << "RandomLinks: no lines found in " << filename;
          return std::wstring();
      }

      // choose a random line
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<std::size_t> dist(0, lines.size() - 1);
      std::size_t idx = dist(gen);

      return Wt::widen(lines[idx]);
  }


  // std::wstring RandomLinks(AppLinks thelinks)
  // {
  //    std::ifstream link;

  //    if (thelinks == APIURL) {
  //      link.open((Wt::WApplication::appRoot() + "apiurl.txt").c_str());
  //    }
  //    else if (thelinks == UPLOADURL) {
  //          link.open((Wt::WApplication::appRoot() + "uploadurl.txt").c_str());
  //        }
  //    else if (thelinks == MEDIAURL) {
  //          link.open((Wt::WApplication::appRoot() + "mediaurl.txt").c_str());
  //        }
  //    else if (thelinks == STREAMURL) {
  //          link.open((Wt::WApplication::appRoot() + "streamurl.txt").c_str());
  //        }
  //   else if (thelinks == HELPURL) {
  //         link.open((Wt::WApplication::appRoot() + "helpurl.txt").c_str());
  //       }
  //    else if (thelinks == LOCALDATAURL) {
  //          link.open((Wt::WApplication::appRoot() + "localdataurl.txt").c_str());
  //        }
  //    else if (thelinks == LOCALMEDIAURL) {
  //          link.open((Wt::WApplication::appRoot() + "localmediaurl.txt").c_str());
  //        }
  //    else if (thelinks == LOCALSTREAMURL) {
  //          link.open((Wt::WApplication::appRoot() + "localstreamurl.txt").c_str());
  //        }
  //    else if (thelinks == LOCALSERVERTMP) {
  //          link.open((Wt::WApplication::appRoot() + "localservertmp.txt").c_str());
  //        }
  //    else if (thelinks == LOCALSERVERROOT) {
  //          link.open((Wt::WApplication::appRoot() + "localserverroot.txt").c_str());
  //        }
  //    else if (thelinks == REGISTERTOKEN) {
  //          link.open((Wt::WApplication::appRoot() + "registertoken.txt").c_str());
  //        }
  //    else if (thelinks == LOGINTOKEN) {
  //          link.open((Wt::WApplication::appRoot() + "logintoken.txt").c_str());
  //        }
  //    else {

  //        link.open((Wt::WApplication::appRoot() + "apiurl.txt").c_str());
  //    }

  //    std::string strvalue;
  //  //  int numwords = 0;
  //    int numwords = -1;
  //    while(link) {
  //       getline(link, strvalue);
  //       numwords++;
  //    }
  //    link.clear();
  //    link.seekg(0);

  //    srand(time(0));
  //    int myrand = rand();
  //    int selection = myrand % numwords;

  //    while(selection--) {
  //       getline(link, strvalue);
  //    }
  //    getline(link, strvalue);

  //    return Wt::widen(strvalue);
  // }


  std::wstring RandomRole(AppRoles theroles)
  {
     std::ifstream role;

     if (theroles == GUEST) {
       role.open((Wt::WApplication::appRoot() + "guest.txt").c_str());
     }
    else if (theroles == WEBMASTER) {
          role.open((Wt::WApplication::appRoot() + "webmaster.txt").c_str());
        }
     else if (theroles == ADMIN) {
           role.open((Wt::WApplication::appRoot() + "admin.txt").c_str());
         }
     else if (theroles == SUPERPOWER) {
           role.open((Wt::WApplication::appRoot() + "superpower.txt").c_str());
         }

     else {

         role.open((Wt::WApplication::appRoot() + "normaluser.txt").c_str());
     }

     std::string strvalue;
   //  int numwords = 0;
     int numwords = -1;
     while(role) {
        getline(role, strvalue);
        numwords++;
     }
     role.clear();
     role.seekg(0);

     srand(time(0));
     int myrand = rand();
     int selection = myrand % numwords;

     while(selection--) {
        getline(role, strvalue);
     }
     getline(role, strvalue);

     return Wt::widen(strvalue);
  }




  MainTemplate::MainTemplate(const char *trKey)
    : WTemplate(tr(trKey))
  {

  }


  Wt::WString MainTemplate::reindent(const Wt::WString& text)
  {
    std::vector<std::string> lines;
    std::string s = text.toUTF8();
    boost::split(lines, s, boost::is_any_of("\n"));

    std::string result;
    int indent = -1;
    int newlines = 0;
    for (unsigned i = 0; i < lines.size(); ++i) {
      const std::string& line = lines[i];

      if (line.empty()) {
        ++newlines;
      } else {
        if (indent == -1) {
          indent = countSpaces(line);
        } else {
          for (int j = 0; j < newlines; ++j)
            result += '\n';
        }

        newlines = 0;

        if (!result.empty())
          result += '\n';

        result += skipSpaces(line, indent);
      }
    }
    return Wt::WString(result);
  }


  std::string MainTemplate::getString(const std::string& varName)
  {
    std::stringstream ss;
    std::vector<Wt::WString> args;

    resolveString(varName, args, ss);

    return ss.str();
  }

  std::string MainTemplate::docUrl(const std::string& className)
  {
    Wt::WStringStream ss;

  #if !defined(WT_TARGET_JAVA)
    ss << getString("doc-url") << "class" << escape("Wt::" + className)
       << ".html";
  #else
    std::string cn = className;
    boost::replace_all(cn, ".", "/");
    ss << getString("doc-url") << cn << ".html";
  #endif

    return ss.str();
  }

  void MainTemplate::resolveString(const std::string& varName,
                    const std::vector<Wt::WString>& args,
                    std::ostream& result)
  {
    if (varName == "doc-link") {
      std::string className = args[0].toUTF8();

  #ifndef WT_TARGET_JAVA
      boost::replace_all(className, "-", "::");
  #else
      boost::replace_all(className, "Render-", "render.");
  #endif

      result << "<a href=\"" << docUrl(className)
         << "\" target=\"_blank\">";

  #ifdef WT_TARGET_JAVA
      boost::replace_all(className, "render.", "");
  #endif // WT_TARGET_JAVA

      result << className << "</a>";
    } else if (varName == "src") {
      std::string exampleName = args[0].toUTF8();
      result << "<fieldset class=\"src\">"
         << "<legend>source</legend>"
             << tr("src-" + exampleName).toXhtmlUTF8()
         << "</fieldset>";
    } else
      WTemplate::resolveString(varName, args, result);
  }

  std::string MainTemplate::escape(const std::string &name)
  {
    Wt::WStringStream ss;

    for (unsigned i = 0; i < name.size(); ++i) {
      if (name[i] != ':')
        ss << name[i];
      else
        ss << "_1";
    }

    return ss.str();
  }


  UserNameResource::UserNameResource()

  {



  }

  UserNameResource::~UserNameResource()
  {

  }





 void UserNameResource::mytimerendpointuser() {


     //qDebug() << "timerendpointuser timer timeout" << Qt::endl;
     //allitems_->bindEmpty("items");

   //   networkutils->getallitems();
    //  timergetdataall->start();
    //  showCustomWidgetT("spinner1","myspinner1",mapper_->allitems_);
    //  isstartup = true;

 }

 void UserNameResource::handlenameresource(std::string myuser) {

  // https://0.0.0.0:8180/deliva/username/ericm
  // https://0.0.0.0:8180/deliva/business/ericm

   //qDebug() << "My User handlenameresource is: " << QString::fromStdString(myuser) << Qt::endl;


  }

  void UserNameResource::handleRequest(const Wt::Http::Request& request,
                             Wt::Http::Response& response)
  {

  // bool triggerUpdate = false;

   auto params = request.urlParams();
   if(params.empty()) {

    // response.out() << "(empty)\n";

         }

    for (const auto &param : params) {
           const auto &name = param.first;
           const auto &value = param.second;
        //   response.out() << name << ": " << value << '\n';
     }

// https://0.0.0.0:8180/deliva/business/ericm


if((request.urlParam("data")=="business")) {



  std::string mybiz = request.urlParam("business");
  //qDebug() << "My Biz is: " << QString::fromStdString(mybiz) << Qt::endl;
  usernameresource_.emit(mybiz);



           }


else  if((request.urlParam("data")=="all")) { // begin


   }

 else {


 const std::string ErrorTemplate ="BAD_REQUEST";
 response.setMimeType("text/plain");
 response.setStatus(400);
 response.out() <<ErrorTemplate;



    }

  }

std::string UserNameResource::fromIstream(std::istream &stream)
   {
         std::istreambuf_iterator<char> eos;
         return std::string(std::istreambuf_iterator<char>(stream), eos);
   }




  ServerResource::ServerResource()

   {



   }

  ServerResource::~ServerResource()

   {
      beingDeleted();
   }


   std::string ServerResource::fromIstream(std::istream &stream)
    {
          std::istreambuf_iterator<char> eos;
          return std::string(std::istreambuf_iterator<char>(stream), eos);
    }


   void ServerResource::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) { // begin virtual void handleRequest

    response.addHeader("Server", "Universal Data Server");

    // bool triggerUpdate = false;

     auto params = request.urlParams();
     if(params.empty()) {

      // response.out() << "(empty)\n";

           }

      for (const auto &param : params) {
             const auto &name = param.first;
             const auto &value = param.second;
          //   response.out() << name << ": " << value << '\n';
       }

    // https://0.0.0.0:8180/deliva/business/ericm


    if((request.urlParam("data")=="business")) {



    std::string mybiz = request.urlParam("business");
    //qDebug() << "My Biz ServerResource is: " << QString::fromStdString(mybiz) << Qt::endl;

    /*

    Wt::WApplication::instance()->attachThread(true);
    Wt::WApplication::UpdateLock lck = Wt::WApplication::instance()->getUpdateLock();

    Wt::WApplication::instance()->attachThread(false);

    Wt::WApplication::UpdateLock lock(Wt::WApplication::instance());
    if (lock) {
      // manipulate UI, or Q_EMIT a signal to manipulate the UI, etc...
}


*/




             }


    else  if((request.urlParam("data")=="all")) { // begin


     }

    else {


    const std::string ErrorTemplate ="BAD_REQUEST";
    response.setMimeType("text/plain");
    response.setStatus(400);
    response.out() <<ErrorTemplate;



      }
   }





 static const char *OrdersUrl = "/";




std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env)
{
  return std::make_unique<MapperSoftware>(env);
}


QStringList ServerController::availableIPAddresses() {
    QStringList list;
    const auto addrs = QNetworkInterface::allAddresses();
    for (const QHostAddress &addr : addrs) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && !addr.isLoopback()) {
            list << addr.toString();
        }
    }
    // If no non-loopback IPv4 found, include loopback as fallback
    if (list.isEmpty()) {
        list << QStringLiteral("127.0.0.1");
    }
    return list;
}



// start best working
// top of file: only declare environ on POSIX non-Android targets
#if !defined(WT_WIN32) && !defined(__ANDROID__)
extern char **environ; // for Wt::WServer::restart on POSIX (not Android)
#endif

namespace {
    std::atomic<int> g_lastSignal{0};
    void signalHandler(int sig) {
        g_lastSignal.store(sig);
    }
}

ServerController::ServerController(QObject *parent)
    : QObject(parent)
{ }

ServerController::~ServerController() {
    stopServer();
}

void ServerController::applyListenIpFrom(StreamServer *server) {
    if (!server) {
        WarnL << "ServerControllerWT::applyListenIpFrom: null server pointer";
        return;
    }

    QString ip4 = server->getLocalIp();
    QString ip6 = server->getLocalIpV6();
    QString chosen;
    if (!ip4.isEmpty()) chosen = ip4;
    else if (!ip6.isEmpty()) chosen = ip6;
    else {
        WarnL << "ServerControllerWT::applyListenIpFrom: StreamServer has no non-loopback IP";
        return;
    }

    if (chosen.isEmpty() || chosen == QLatin1String("0.0.0.0") ||
        chosen == QLatin1String("::") || chosen == QLatin1String("::1") ||
        chosen.startsWith(QLatin1String("127."))) {
        WarnL << "ServerControllerWT::applyListenIpFrom: refused loopback/wildcard ip: " << chosen.toStdString();
        listenIp_ = QString();
        return;
    }

    else {

        listenIp_ = chosen;
    }


    InfoL << "ServerControllerWT::applyListenIpFrom: listenIp set to " << listenIp_.toStdString();

    // attempt graceful restart using reflective calls (same pairs as above)
    bool restarted = false;
    if (QMetaObject::invokeMethod(this, "stop")) {
        if (QMetaObject::invokeMethod(this, "start")) { restarted = true; InfoL << "restarted stop()/start()"; }
        else if (QMetaObject::invokeMethod(this, "startServer")) { restarted = true; InfoL << "restarted stop()/startServer()"; }
    }
    if (!restarted && QMetaObject::invokeMethod(this, "stopServer")) {
        if (QMetaObject::invokeMethod(this, "startServer")) { restarted = true; InfoL << "restarted stopServer()/startServer()"; }
    }
    if (!restarted && QMetaObject::invokeMethod(this, "shutdown")) {
        if (QMetaObject::invokeMethod(this, "start")) { restarted = true; InfoL << "restarted shutdown()/start()"; }
        else if (QMetaObject::invokeMethod(this, "startServer")) { restarted = true; InfoL << "restarted shutdown()/startServer()"; }
    }
    if (!restarted) {
        InfoL << "ServerControllerWT::applyListenIpFrom: no restart pair found; please restart controller manually.";
    }
}



bool ServerController::startServer(int port) {
    if (running_.load()) return false;
    running_ = true;

   m_url = QStringLiteral("http://%1:%2/").arg(listenIp_, QString::number(port));
    Q_EMIT urlChanged();

    QString downloadsPath = "";

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#else
    downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif

    if (downloadsPath.isEmpty())
        downloadsPath = QDir::currentPath();

    qInfo() << "[ServerController] Using downloadsPath:" << downloadsPath;

    QDir baseDir(downloadsPath);
    if (!baseDir.exists("Orders_files")) {
        qInfo() << "[ServerController] Creating Orders_files folder in" << baseDir.absolutePath();
        if (!baseDir.mkdir("Orders_files")) {
            qWarning() << "[ServerController] Failed to create Orders_files folder in" << baseDir.absolutePath();
        }
    }
    baseDir.cd("Orders_files");

    QString appPath = baseDir.filePath("App");
    qInfo() << "[ServerController] appPath will be:" << appPath;

    if (!QDir(appPath).exists()) {
        qInfo() << "[ServerController] Extracting AppArchive from QRC...";
        QFile qrcFile(":/myserverassets/AppArchive");
        if (!qrcFile.exists()) {
            qWarning() << "[ServerController] QRC resource missing: :/myserverassets/AppArchive";
            Q_EMIT errorOccurred("QRC resource missing");
            running_ = false;
            return false;
        }

        if (!qrcFile.open(QIODevice::ReadOnly)) {
            qWarning() << "[ServerController] Failed to open QRC archive file:" << qrcFile.errorString();
            Q_EMIT errorOccurred("Failed to open archive from QRC: " + qrcFile.errorString());
            running_ = false;
            return false;
        }

        QByteArray data = qrcFile.readAll();
        qrcFile.close();

        qInfo() << "[ServerController] Read archive size (bytes):" << data.size();
        if (data.isEmpty()) {
            qWarning() << "[ServerController] Archive read empty — resource probably not packaged.";
            Q_EMIT errorOccurred("Archive empty");
            running_ = false;
            return false;
        }

        LibArchiveExtractor extractor;
        qInfo() << "[ServerController] Calling extractor.extractTarXzFromMemory(dest:" << baseDir.absolutePath() << ") ...";
        if (!extractor.extractTarXzFromMemory(data.data(), static_cast<size_t>(data.size()), baseDir.absolutePath())) {
            qWarning() << "[ServerController] Extraction failed (see extractor logs).";
            Q_EMIT errorOccurred("Extraction failed from memory");
            running_ = false;
            return false;
        }
        qInfo() << "[ServerController] Archive extracted to" << appPath;
    } else {
       // qInfo() << "[ServerController] Archive already extracted to" << appPath << ", skipping extraction.";
    }

    QString approotDir = QDir(appPath).filePath("approot");
    QString docrootDir = QDir(appPath).filePath("docroot");
    QString resourcesDir = QDir(appPath).filePath("resources");

    // qInfo() << "[ServerController] Checking paths:\n appPath =" << appPath
    //         << "\n approotDir =" << approotDir
    //         << "\n docrootDir =" << docrootDir
    //         << "\n resourcesDir =" << resourcesDir;

    // qInfo() << "[ServerController] exists(appPath):" << QDir(appPath).exists()
    //         << " exists(approot):" << QDir(approotDir).exists()
    //         << " exists(docroot):" << QDir(docrootDir).exists();

    if (!QDir(approotDir).exists() || !QDir(docrootDir).exists()) {
        qWarning() << "[ServerController] Required folders not found after extraction.";
        Q_EMIT errorOccurred("Required folders missing");
        running_ = false;
        return false;
    }

    m_assetPath = appPath;
    Q_EMIT assetPathChanged();

    // Start the server thread
    serverThread_ = std::make_unique<std::thread>([this, port, approotDir, docrootDir, resourcesDir]() mutable {
        try {
            qInfo() << "[ServerThread] Building WServer argument list...";
            std::vector<std::string> argHolder;
            QStringList qargs;
            qargs << "Orders"
                  << "--http-address=" + listenIp_
                  << "--http-port=" + QString::number(port)
                  << "--docroot=" + docrootDir
                  << "--approot=" + approotDir
                  << "--resources-dir=" + resourcesDir;

            qInfo() << "[ServerThread] qargs:";
            for (const QString &a : qargs) qInfo() << "  " << a;

            for (const QString &arg : qargs) {
                argHolder.push_back(arg.toStdString());
            }

            std::vector<char*> argv;
            argv.reserve(argHolder.size() + 1);
            for (auto &s : argHolder) {
                argv.push_back(const_cast<char*>(s.c_str()));
            }
            argv.push_back(nullptr);
            int argc = static_cast<int>(argv.size() - 1);

            qInfo() << "[ServerThread] argc =" << argc;
            for (int i = 0; i < argc; ++i) {
                qInfo() << "[ServerThread] argv[" << i << "] =" << (argv[i] ? argv[i] : "<null>");
            }

            // ----- Port probe (diagnostic) -----
            {
                QTcpServer probeAny;
                bool bindAny = probeAny.listen(QHostAddress::Any, port);
                qInfo() << "[PortProbe] bind Any (" << port << ") ->" << (bindAny ? "OK" : QString("FAIL (%1)").arg(probeAny.errorString()));
                if (bindAny) probeAny.close();
            }
            {
                QTcpServer probe127;
                bool bind127 = probe127.listen(QHostAddress::LocalHost, port);
                qInfo() << "[PortProbe] bind 127.0.0.1 (" << port << ") ->" << (bind127 ? "OK" : QString("FAIL (%1)").arg(probe127.errorString()));
                if (bind127) probe127.close();
            }

            qInfo() << "[ServerThread] Constructing WServer...";
            try {
                server_ = std::make_unique<Wt::WServer>(argc, argv.data(), WTHTTP_CONFIGURATION);
                qInfo() << "[ServerThread] WServer constructed successfully";
            } catch (const std::exception &ex) {
                qWarning() << "[ServerThread] WServer construction failed:" << ex.what();
                Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
                running_ = false;
                QMetaObject::invokeMethod(this, [this]() { Q_EMIT stopped(); }, Qt::QueuedConnection);
                return;
            }

            ServerResource myserverResource;

            qInfo() << "[ServerThread] Adding resources and entry points...";
            try {
               // server_->logger().configure("-*");
                server_->addResource(&myserverResource, "/deliva/${data}/${business}");
                server_->addEntryPoint(Wt::EntryPointType::Application,
                                      std::bind(&createApplication, std::placeholders::_1),
                                      OrdersUrl);
                qInfo() << "[ServerThread] addResource/addEntryPoint OK";
            } catch (const std::exception &ex) {
                qWarning() << "[ServerThread] Exception while adding resource/entrypoint:" << ex.what();
                Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
            }

            qInfo() << "[ServerThread] Attempting server_->start()...";
            bool serverStarted = false;

            try {
                serverStarted = server_->start();
            } catch (const std::exception &ex) {
                qWarning() << "[ServerThread] server_->start() threw exception:" << ex.what();
                Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
            } catch (...) {
                qWarning() << "[ServerThread] server_->start() threw unknown exception";
                Q_EMIT errorOccurred("Unknown exception when starting server");
            }

            qInfo() << "[ServerThread] server_->start() returned:" << serverStarted;

            if (serverStarted) {
                QMetaObject::invokeMethod(this, [this]() { Q_EMIT started(); }, Qt::QueuedConnection);
                qInfo() << "[ServerThread] Emitted started()";

                // ---- Interface enumeration and self-connection tests ----
                qInfo() << "[ServerThread] Enumerating network interfaces and addresses:";
                for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
                    if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
                    QStringList addrs;
                    for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                        addrs << entry.ip().toString();
                    }
                    qInfo() << "[NetIf]" << iface.humanReadableName() << iface.name() << "flags=" << iface.flags() << "addrs=" << addrs;
                }

                qInfo() << "[ServerThread] Local self-connection tests:";
                for (const QHostAddress &addr : QNetworkInterface::allAddresses()) {
                    if (addr.protocol() != QAbstractSocket::IPv4Protocol) continue;
                    if (addr.isLoopback()) continue; // skip loopback here
                    QTcpSocket sock;
                    sock.connectToHost(addr, port);
                    bool ok = sock.waitForConnected(500);
                    qInfo() << "[SelfTest] connect to" << addr.toString() << ":" << port << " -> " << (ok ? "OK" : QString("FAIL (%1)").arg(sock.errorString()));
                    if (ok) sock.disconnectFromHost();
                }
                qInfo() << "[ServerThread] Self tests complete";

                QCoreApplication *qcore = QCoreApplication::instance();
                if (qcore) {
                    qInfo() << "[ServerThread] QCoreApplication::instance() type:" << qcore->metaObject()->className();
                    QObject::connect(qcore, &QCoreApplication::aboutToQuit, this, [this]() {
                        qInfo() << "[ServerThread] aboutToQuit -> stopping server";
                        try { if (server_) server_->stop(); } catch (...) {}
                    }, Qt::QueuedConnection);
                }

                // Do not register SIGHUP on Android; only register on non-Windows non-Android POSIX
#if !defined(WT_WIN32) && !defined(__ANDROID__)
                signal(SIGHUP, signalHandler);
#endif
#ifdef SIGTERM
                signal(SIGTERM, signalHandler);
#endif
#ifdef SIGABRT
                signal(SIGABRT, signalHandler);
#endif
#ifdef SIGINT
                signal(SIGINT, signalHandler);
#endif

                qInfo() << "[ServerThread] Waiting for shutdown...";
                int sig = Wt::WServer::waitForShutdown();
                qInfo() << "[ServerThread] waitForShutdown returned sig=" << sig;

#if !defined(WT_WIN32) && !defined(__ANDROID__)
                if (sig == SIGHUP) {
                    qInfo() << "[ServerThread] Received SIGHUP; attempting restart...";
                    try {
                        Wt::WServer::restart(argc, argv.data(), environ);
                    } catch (const std::exception &ex) {
                        qWarning() << "[ServerThread] restart failed:" << ex.what();
                        Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
                    } catch (...) {
                        qWarning() << "[ServerThread] restart failed with unknown exception";
                        Q_EMIT errorOccurred("WServer restart failed with unknown exception");
                    }
                }
#endif
            } else {
                qWarning() << "[ServerThread] server_->start() returned false";
                Q_EMIT errorOccurred("WServer failed to start (returned false)");
            }

            try {
                if (server_) {
                    qInfo() << "[ServerThread] Stopping server_ ...";
                    server_->stop();
                }
            } catch (const std::exception &ex) {
                qWarning() << "[ServerThread] Exception while stopping server_:" << ex.what();
            } catch (...) {
                qWarning() << "[ServerThread] Unknown exception while stopping server_";
            }

        } catch (...) {
            qWarning() << "[ServerThread] Unknown exception in server thread";
            Q_EMIT errorOccurred("Unknown exception in server thread");
        }

        running_ = false;
        QMetaObject::invokeMethod(this, [this]() { Q_EMIT stopped(); }, Qt::QueuedConnection);
    });

    return true;
}


void ServerController::stopServer() {
    if (!running_.load()) return;

    if (server_) {
        try {
            server_->stop();
        } catch (const std::exception &ex) {
            Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
        } catch (...) {
            Q_EMIT errorOccurred(QStringLiteral("Unknown exception while stopping server"));
        }
    }

    if (serverThread_ && serverThread_->joinable()) {
        serverThread_->join();
    }

    serverThread_.reset();
    server_.reset();
    running_ = false;
    Q_EMIT stopped();
}




// // start best working
// // top of file: only declare environ on POSIX non-Android targets
// #if !defined(WT_WIN32) && !defined(__ANDROID__)
// extern char **environ; // for Wt::WServer::restart on POSIX (not Android)
// #endif

// namespace {
//     std::atomic<int> g_lastSignal{0};
//     void signalHandler(int sig) {
//         g_lastSignal.store(sig);
//     }
// }

// ServerController::ServerController(QObject *parent)
//     : QObject(parent)
// { }

// ServerController::~ServerController() {
//     stopServer();
// }

// bool ServerController::startServer(int port) {
//     if (running_.load()) return false;
//     running_ = true;

//     m_url = QStringLiteral("http://0.0.0.0:%1/").arg(port);
//     Q_EMIT urlChanged();

//     QString downloadsPath = "";

// #if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
//     downloadsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
// #else
//     downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
// #endif

//     if (downloadsPath.isEmpty())
//         downloadsPath = QDir::currentPath();

//     qInfo() << "[ServerController] Using downloadsPath:" << downloadsPath;

//     QDir baseDir(downloadsPath);
//     if (!baseDir.exists("Orders_files")) {
//         qInfo() << "[ServerController] Creating Orders_files folder in" << baseDir.absolutePath();
//         if (!baseDir.mkdir("Orders_files")) {
//             qWarning() << "[ServerController] Failed to create Orders_files folder in" << baseDir.absolutePath();
//         }
//     }
//     baseDir.cd("Orders_files");

//     QString appPath = baseDir.filePath("App");
//     qInfo() << "[ServerController] appPath will be:" << appPath;

//     if (!QDir(appPath).exists()) {
//         qInfo() << "[ServerController] Extracting AppArchive from QRC...";
//         QFile qrcFile(":/myserverassets/AppArchive");
//         if (!qrcFile.exists()) {
//             qWarning() << "[ServerController] QRC resource missing: :/myserverassets/AppArchive";
//             Q_EMIT errorOccurred("QRC resource missing");
//             running_ = false;
//             return false;
//         }

//         if (!qrcFile.open(QIODevice::ReadOnly)) {
//             qWarning() << "[ServerController] Failed to open QRC archive file:" << qrcFile.errorString();
//             Q_EMIT errorOccurred("Failed to open archive from QRC: " + qrcFile.errorString());
//             running_ = false;
//             return false;
//         }

//         QByteArray data = qrcFile.readAll();
//         qrcFile.close();

//         qInfo() << "[ServerController] Read archive size (bytes):" << data.size();
//         if (data.isEmpty()) {
//             qWarning() << "[ServerController] Archive read empty — resource probably not packaged.";
//             Q_EMIT errorOccurred("Archive empty");
//             running_ = false;
//             return false;
//         }

//         LibArchiveExtractor extractor;
//         qInfo() << "[ServerController] Calling extractor.extractTarXzFromMemory(dest:" << baseDir.absolutePath() << ") ...";
//         if (!extractor.extractTarXzFromMemory(data.data(), static_cast<size_t>(data.size()), baseDir.absolutePath())) {
//             qWarning() << "[ServerController] Extraction failed (see extractor logs).";
//             Q_EMIT errorOccurred("Extraction failed from memory");
//             running_ = false;
//             return false;
//         }
//         qInfo() << "[ServerController] Archive extracted to" << appPath;
//     } else {
//         qInfo() << "[ServerController] Archive already extracted to" << appPath << ", skipping extraction.";
//     }

//     QString approotDir = QDir(appPath).filePath("approot");
//     QString docrootDir = QDir(appPath).filePath("docroot");
//     QString resourcesDir = QDir(appPath).filePath("resources");

//     qInfo() << "[ServerController] Checking paths:\n appPath =" << appPath
//             << "\n approotDir =" << approotDir
//             << "\n docrootDir =" << docrootDir
//             << "\n resourcesDir =" << resourcesDir;

//     qInfo() << "[ServerController] exists(appPath):" << QDir(appPath).exists()
//             << " exists(approot):" << QDir(approotDir).exists()
//             << " exists(docroot):" << QDir(docrootDir).exists();

//     if (!QDir(approotDir).exists() || !QDir(docrootDir).exists()) {
//         qWarning() << "[ServerController] Required folders not found after extraction.";
//         Q_EMIT errorOccurred("Required folders missing");
//         running_ = false;
//         return false;
//     }

//     m_assetPath = appPath;
//     Q_EMIT assetPathChanged();

//     // Start the server thread
//     serverThread_ = std::make_unique<std::thread>([this, port, approotDir, docrootDir, resourcesDir]() mutable {
//         try {
//             qInfo() << "[ServerThread] Building WServer argument list...";
//             std::vector<std::string> argHolder;
//             QStringList qargs;
//             qargs << "Orders"
//                   << "--http-address=0.0.0.0"
//                   << "--http-port=" + QString::number(port)
//                   << "--docroot=" + docrootDir
//                   << "--approot=" + approotDir
//                   << "--resources-dir=" + resourcesDir;

//             qInfo() << "[ServerThread] qargs:";
//             for (const QString &a : qargs) qInfo() << "  " << a;

//             for (const QString &arg : qargs) {
//                 argHolder.push_back(arg.toStdString());
//             }

//             std::vector<char*> argv;
//             argv.reserve(argHolder.size() + 1);
//             for (auto &s : argHolder) {
//                 argv.push_back(const_cast<char*>(s.c_str()));
//             }
//             argv.push_back(nullptr);
//             int argc = static_cast<int>(argv.size() - 1);

//             qInfo() << "[ServerThread] argc =" << argc;
//             for (int i = 0; i < argc; ++i) {
//                 qInfo() << "[ServerThread] argv[" << i << "] =" << (argv[i] ? argv[i] : "<null>");
//             }

//             qInfo() << "[ServerThread] Constructing WServer...";
//             try {
//                 server_ = std::make_unique<Wt::WServer>(argc, argv.data(), WTHTTP_CONFIGURATION);
//                 qInfo() << "[ServerThread] WServer constructed successfully";
//             } catch (const std::exception &ex) {
//                 qWarning() << "[ServerThread] WServer construction failed:" << ex.what();
//                 Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//                 running_ = false;
//                 QMetaObject::invokeMethod(this, [this]() { Q_EMIT stopped(); }, Qt::QueuedConnection);
//                 return;
//             }

//             ServerResource myserverResource;

//             qInfo() << "[ServerThread] Adding resources and entry points...";
//             try {
//                 server_->addResource(&myserverResource, "/deliva/${data}/${business}");
//                 server_->addEntryPoint(Wt::EntryPointType::Application,
//                                       std::bind(&createApplication, std::placeholders::_1),
//                                       OrdersUrl);
//                 qInfo() << "[ServerThread] addResource/addEntryPoint OK";
//             } catch (const std::exception &ex) {
//                 qWarning() << "[ServerThread] Exception while adding resource/entrypoint:" << ex.what();
//                 Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//             }

//             qInfo() << "[ServerThread] Warning: Site is deployed at" << OrdersUrl;

//             qInfo() << "[ServerThread] Attempting server_->start()...";
//             bool serverStarted = false;

//             try {
//                 serverStarted = server_->start();
//             } catch (const std::exception &ex) {
//                 qWarning() << "[ServerThread] server_->start() threw exception:" << ex.what();
//                 Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//             } catch (...) {
//                 qWarning() << "[ServerThread] server_->start() threw unknown exception";
//                 Q_EMIT errorOccurred("Unknown exception when starting server");
//             }

//             qInfo() << "[ServerThread] server_->start() returned:" << serverStarted;

//             if (serverStarted) {
//                 QMetaObject::invokeMethod(this, [this]() { Q_EMIT started(); }, Qt::QueuedConnection);
//                 qInfo() << "[ServerThread] Emitted started()";

//                 QCoreApplication *qcore = QCoreApplication::instance();
//                 if (qcore) {
//                     qInfo() << "[ServerThread] QCoreApplication::instance() type:" << qcore->metaObject()->className();
//                     QObject::connect(qcore, &QCoreApplication::aboutToQuit, this, [this]() {
//                         qInfo() << "[ServerThread] aboutToQuit -> stopping server";
//                         try { if (server_) server_->stop(); } catch (...) {}
//                     }, Qt::QueuedConnection);
//                 }

//                 // Do not register SIGHUP on Android; only register on non-Windows non-Android POSIX
// #if !defined(WT_WIN32) && !defined(__ANDROID__)
//                 signal(SIGHUP, signalHandler);
// #endif
//                 // Register common signals where available
// #ifdef SIGTERM
//                 signal(SIGTERM, signalHandler);
// #endif
// #ifdef SIGABRT
//                 signal(SIGABRT, signalHandler);
// #endif
// #ifdef SIGINT
//                 signal(SIGINT, signalHandler);
// #endif

//                 qInfo() << "[ServerThread] Waiting for shutdown...";
//                 int sig = Wt::WServer::waitForShutdown();
//                 qInfo() << "[ServerThread] waitForShutdown returned sig=" << sig;

//                 // Only attempt restart on POSIX non-Android systems where environ is declared
// #if !defined(WT_WIN32) && !defined(__ANDROID__)
//                 if (sig == SIGHUP) {
//                     qInfo() << "[ServerThread] Received SIGHUP; attempting restart...";
//                     try {
//                         Wt::WServer::restart(argc, argv.data(), environ);
//                     } catch (const std::exception &ex) {
//                         qWarning() << "[ServerThread] restart failed:" << ex.what();
//                         Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//                     } catch (...) {
//                         qWarning() << "[ServerThread] restart failed with unknown exception";
//                         Q_EMIT errorOccurred("WServer restart failed with unknown exception");
//                     }
//                 }
// #endif
//             } else {
//                 qWarning() << "[ServerThread] server_->start() returned false";
//                 Q_EMIT errorOccurred("WServer failed to start (returned false)");
//             }

//             try {
//                 if (server_) {
//                     qInfo() << "[ServerThread] Stopping server_ ...";
//                     server_->stop();
//                 }
//             } catch (const std::exception &ex) {
//                 qWarning() << "[ServerThread] Exception while stopping server_:" << ex.what();
//             } catch (...) {
//                 qWarning() << "[ServerThread] Unknown exception while stopping server_";
//             }

//         } catch (...) {
//             qWarning() << "[ServerThread] Unknown exception in server thread";
//             Q_EMIT errorOccurred("Unknown exception in server thread");
//         }

//         running_ = false;
//         QMetaObject::invokeMethod(this, [this]() { Q_EMIT stopped(); }, Qt::QueuedConnection);
//     });

//     return true;
// }

// void ServerController::stopServer() {
//     if (!running_.load()) return;

//     if (server_) {
//         try {
//             server_->stop();
//         } catch (const std::exception &ex) {
//             Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//         } catch (...) {
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception while stopping server"));
//         }
//     }

//     if (serverThread_ && serverThread_->joinable()) {
//         serverThread_->join();
//     }

//     serverThread_.reset();
//     server_.reset();
//     running_ = false;
//     Q_EMIT stopped();
// }

// // end  best working
// bool ServerController::startServer(int port) {
//     if (running_.load()) return false;
//     running_ = true;

//     m_url = QStringLiteral("http://0.0.0.0:%1/").arg(port);
//     Q_EMIT urlChanged();

//     QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
//     if (downloadsPath.isEmpty())
//         downloadsPath = QDir::currentPath();

//     QDir baseDir(downloadsPath);
//     if (!baseDir.exists("Orders_files")) baseDir.mkdir("Orders_files");
//     baseDir.cd("Orders_files");

//     QString appPath = baseDir.filePath("App");
//     if (!QDir(appPath).exists()) {
//         qInfo() << "Extracting AppArchive from QRC...";
//         QFile qrcFile(":/myserverassets/AppArchive");
//         if (!qrcFile.open(QIODevice::ReadOnly)) {
//             qWarning() << "Failed to open QRC archive file";
//             Q_EMIT errorOccurred("Failed to open archive from QRC");
//             running_ = false;
//             return false;
//         }

//         QByteArray data = qrcFile.readAll();
//         qrcFile.close();

//         LibArchiveExtractor extractor;
//         // extract into baseDir because archive contains the top-level "App" directory
//         if (!extractor.extractTarXzFromMemory(data.data(), static_cast<size_t>(data.size()), baseDir.absolutePath())) {
//             qWarning() << "Extraction failed";
//             Q_EMIT errorOccurred("Extraction failed from memory");
//             running_ = false;
//             return false;
//         }
//         qInfo() << "Archive extracted to" << appPath;
//     } else {
//         qInfo() << "Archive already extracted to" << appPath << ", skipping extraction.";
//     }

//     QString approotDir = QDir(appPath).filePath("approot");
//     QString docrootDir = QDir(appPath).filePath("docroot");
//     QString resourcesDir = QDir(appPath).filePath("resources");

//     if (!QDir(approotDir).exists() || !QDir(docrootDir).exists()) {
//         qWarning() << "Required folders not found after extraction.";
//         Q_EMIT errorOccurred("Required folders missing");
//         running_ = false;
//         return false;
//     }

//     m_assetPath = appPath;
//     Q_EMIT assetPathChanged();

//     // Start the server thread. Build argv inside the thread so that string storage
//     // outlives the pointers passed to WServer.
//     serverThread_ = std::make_unique<std::thread>([this, port, approotDir, docrootDir, resourcesDir]() mutable {
//         try {
//             // Build argument list here so the std::strings live inside this thread's stack
//             std::vector<std::string> argHolder;
//             QStringList qargs;
//             qargs << "Orders"
//                   << "--http-address=0.0.0.0"
//                   << "--http-port=" + QString::number(port)
//                   // deploy-path intentionally omitted (it's a URL path, not a filesystem path)
//                   << "--docroot=" + docrootDir
//                   << "--approot=" + approotDir
//                   << "--resources-dir=" + resourcesDir;

//             for (const QString &arg : qargs)
//                 argHolder.push_back(arg.toStdString());

//             // Build argv (vector of char*) that points into argHolder's string storage.
//             std::vector<char*> argv;
//             argv.reserve(argHolder.size());
//             for (auto &s : argHolder)
//                 argv.push_back(s.data());

//             int argc = static_cast<int>(argv.size());

//             // Create and start server
//             ServerResource myserverResource;
//             server_ = std::make_unique<Wt::WServer>(argc, argv.data(), WTHTTP_CONFIGURATION);

//             // Add your resources and entry point(s)
//             server_->addResource(&myserverResource, "/deliva/${data}/${business}");
//             server_->addEntryPoint(Wt::EntryPointType::Application,
//                                   std::bind(&createApplication, std::placeholders::_1),
//                                   OrdersUrl);

//             std::cerr << "\n\n -- Warning: Site is deployed at '" << OrdersUrl << "'\n\n";

//             if (server_->start()) {
//                 Q_EMIT started();

//                 if (QCoreApplication *qapp = QCoreApplication::instance()) {
//                     QObject::connect(qapp, &QCoreApplication::aboutToQuit, this, [this]() {
//                         try { if (server_) server_->stop(); } catch (...) {}
//                     }, Qt::QueuedConnection);
//                 }

// #ifndef WT_WIN32
//                 signal(SIGHUP, signalHandler);
// #endif
//                 signal(SIGTERM, signalHandler);
//                 signal(SIGABRT, signalHandler);
//                 signal(SIGINT, signalHandler);

//                 int sig = Wt::WServer::waitForShutdown();

// #ifndef WT_WIN32
//                 if (sig == SIGHUP) {
//                     // environ is available only on POSIX; we declared it above conditionally.
//                     Wt::WServer::restart(argc, argv.data(), environ);
//                 }
// #endif
//             }

//             try { if (server_) server_->stop(); } catch (...) {}

//         } catch (Wt::WServer::Exception &e) {
//             std::cerr << "WServer exception: " << e.what() << std::endl;
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (std::exception &e) {
//             std::cerr << "exception: " << e.what() << std::endl;
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (...) {
//             std::cerr << "unknown exception in server thread" << std::endl;
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception in server thread"));
//         }

//         running_ = false;
//         Q_EMIT stopped();
//     });

//     return true;
// }



// #if !defined(WT_WIN32)
// extern char **environ;
// #endif

// namespace {
//     std::atomic<int> g_lastSignal{0};
//     void signalHandler(int sig) {
//         g_lastSignal.store(sig);
//     }
// }

// ServerController::ServerController(QObject *parent)
//   : QObject(parent) {}

// ServerController::~ServerController() {
//     stopServer();
// }

// bool ServerController::startServer(int port) {
//     if (running_.load()) return false;
//     running_ = true;
//     m_url = QStringLiteral("http://127.0.0.1:%1/").arg(port);
//     Q_EMIT urlChanged();

//     QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
//     if (downloadsPath.isEmpty()) downloadsPath = QDir::currentPath();

//     QDir baseDir(downloadsPath);
//     if (!baseDir.exists("Orders_files")) baseDir.mkdir("Orders_files");
//     baseDir.cd("Orders_files");
//     QString basePath = baseDir.absolutePath();

//     QString appFolder = baseDir.filePath("App");
//     QString approotDir = appFolder + "/approot";
//     QString docrootDir = appFolder + "/docroot";
//     QString resourcesDir = appFolder + "/resources";

//     QFileInfo checkDir(approotDir);
//     if (!checkDir.exists() || !checkDir.isDir()) {
//         qInfo() << "App folder or approot not found. Extracting AppArchive from QRC...";

//         QResource archiveRes(":/myserverassets/AppArchive");
//         if (!archiveRes.isValid()) {
//             qWarning() << "AppArchive not found in QRC.";
//             Q_EMIT errorOccurred("Missing QRC archive");
//             return false;
//         }

//         LibArchiveExtractor extractor;
//         if (!extractor.extractTarXzFromMemory(archiveRes.data(), archiveRes.size(), basePath)) {
//             qWarning() << "Extraction failed.";
//             Q_EMIT errorOccurred("Extraction failed");
//             return false;
//         }
//         qInfo() << "Extraction complete.";
//     } else {
//         qInfo() << "App folder already extracted. Skipping extraction.";
//     }

//     m_assetPath = appFolder;
//     Q_EMIT assetPathChanged();

//     QStringList args;
//     args << "Orders"
//          << "--http-address=127.0.0.1"
//          << "--http-port=" + QString::number(port)
//          << "--deploy-path=" + appFolder
//          << "--docroot=" + docrootDir
//          << "--approot=" + approotDir
//          << "--resources-dir=" + resourcesDir;

//     std::vector<std::string> argHolder;
//     for (const QString &arg : args) argHolder.push_back(arg.toStdString());

//     std::vector<char*> argv;
//     for (auto &arg : argHolder) argv.push_back(arg.data());
//     int argc = static_cast<int>(argv.size());

//     serverThread_ = std::make_unique<std::thread>([this, argc, argv]() mutable {
//         try {
//             ServerResource myserverResource;
//             server_ = std::make_unique<Wt::WServer>(argc, argv.data(), WTHTTP_CONFIGURATION);
//             server_->addResource(&myserverResource, "/deliva/${data}/${business}");
//             server_->addEntryPoint(Wt::EntryPointType::Application,
//                                   std::bind(&createApplication, std::placeholders::_1),
//                                   OrdersUrl);

//             if (server_->start()) {
//                 Q_EMIT started();
//                 QCoreApplication *qapp = QCoreApplication::instance();
//                 if (qapp) {
//                     QObject::connect(qapp, &QCoreApplication::aboutToQuit,
//                                      this, [this]() {
//                                          try { if (server_) server_->stop(); } catch (...) {}
//                                      }, Qt::QueuedConnection);
//                 }
// #ifndef WT_WIN32
//                 signal(SIGHUP, signalHandler);
// #endif
//                 signal(SIGTERM, signalHandler);
//                 signal(SIGABRT, signalHandler);
//                 signal(SIGINT, signalHandler);

//                 int sig = Wt::WServer::waitForShutdown();
// #ifndef WT_WIN32
//                 if (sig == SIGHUP) {
//                     Wt::WServer::restart(argc, argv.data(), environ);
//                 }
// #endif
//             }

//             try { if (server_) server_->stop(); } catch (...) {}

//         } catch (Wt::WServer::Exception &e) {
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (std::exception &e) {
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (...) {
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception in server thread"));
//         }

//         running_ = false;
//         Q_EMIT stopped();
//     });

//     return true;
// }

// void ServerController::stopServer() {
//     if (!running_.load()) return;

//     if (server_) {
//         try {
//             server_->stop();
//         } catch (const std::exception &ex) {
//             Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//         } catch (...) {
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception while stopping server"));
//         }
//     }

//     if (serverThread_ && serverThread_->joinable()) {
//         serverThread_->join();
//     }

//     serverThread_.reset();
//     server_.reset();
//     running_ = false;
//     Q_EMIT stopped();
// }


// // For POSIX WServer::restart usage
// #if !defined(WT_WIN32)
// extern char **environ;
// #endif

// namespace {
//     // Signal flag: handlers set this; server thread may observe if needed
//     std::atomic<int> g_lastSignal{0};
//     void signalHandler(int sig) {
//         g_lastSignal.store(sig);
//         // Do not call non-async-safe functions here.
//     }
// }

// ServerController::ServerController(QObject *parent)
//   : QObject(parent)
// {}

// ServerController::~ServerController() {
//     stopServer();
// }

// bool ServerController::startServer(int port) {
//     if (running_.load()) return false;
//     running_ = true;
//     m_url = QStringLiteral("http://127.0.0.1:%1/").arg(port);
//     Q_EMIT urlChanged();

//     QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
//     if (downloadsPath.isEmpty()) {
//         downloadsPath = QDir::currentPath();
//     }

//     QDir baseDir(downloadsPath);
//     if (!baseDir.exists("Orders_files")) baseDir.mkdir("Orders_files");
//     baseDir.cd("Orders_files");
//     QString basePath = baseDir.absolutePath();

//     // Extract App.tar.xz using Boost
//     AssetExtractor extractor;

//     QObject::connect(&extractor, &AssetExtractor::errorOccurred, this, [](const QString &msg) {
//         qWarning() << "Archive Error:" << msg;
//     });

//     if (!extractor.extractLzmaArchive(":/serverassets/AppArchive", basePath)) {
//         qWarning() << "Failed to extract App.tar.xz archive";
//         return false;
//     }

//     m_assetPath = basePath;
//     Q_EMIT assetPathChanged();
//     qInfo() << "Assets extracted to:" << m_assetPath;

//     QString systemPrefix = basePath + "/App";
//     qDebug() << "SystemPrefix" << systemPrefix << Qt::endl;
//     QStringList args;

//  /*   args << "Orders"
//          << "--ssl-certificate=" + systemPrefix + "/encrypt/donate.crt"
//          << "--ssl-private-key=" + systemPrefix + "/encrypt/donate.key"
//          << "--ssl-tmp-dh=" + systemPrefix + "/encrypt/dhparam.pem"
//          << "--ssl-cipherlist=TLS13-CHACHA20-POLY1305-SHA256:TLS13-AES-128-GCM-SHA256:TLS13-AES-256-GCM-SHA384:ECDHE:!COMPLEMENTOFDEFAULT:!ECDHE-RSA-AES128-GCM-SHA256:!ECDHE-ECDSA-AES128-GCM-SHA256:!ECDHE-RSA-AES256-GCM-SHA384:!ECDHE-ECDSA-AES256-GCM-SHA384:!DHE-RSA-AES128-GCM-SHA256:!DHE-DSS-AES128-GCM-SHA256:!kEDH+AESGCM:!ECDHE-RSA-AES128-SHA256:!ECDHE-ECDSA-AES128-SHA256:!ECDHE-RSA-AES128-SHA:!ECDHE-ECDSA-AES128-SHA:!ECDHE-RSA-AES256-SHA384:!ECDHE-ECDSA-AES256-SHA384:!ECDHE-RSA-AES256-SHA:!ECDHE-ECDSA-AES256-SHA:!DHE-RSA-AES128-SHA256:!DHE-RSA-AES128-SHA:!DHE-DSS-AES128-SHA256:!DHE-RSA-AES256-SHA256:!DHE-DSS-AES256-SHA:!DHE-RSA-AES256-SHA:!AES128-GCM-SHA256:!AES256-GCM-SHA384:!AES128-SHA256:!AES256-SHA256:!AES128-SHA:!AES256-SHA:!AES:!CAMELLIA:!DES-CBC3-SHA:HIGH:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA:!TLSv1:!SSLv2:!SSLv3"
//          << "--ssl-prefer-server-ciphers=1"
//          << "--ssl-client-verification=none"
//          << "--https-listen=159.223.118.66:8180"
//          << "--deploy-path=" + systemPrefix
//          << "--docroot=" + systemPrefix + "/docroot"
//          << "--approot=" + systemPrefix + "/approot"
//          << "--resources-dir=" + systemPrefix + "/resources"; */

//     args << "Orders"
//          << "--http-address=127.0.0.1"
//          << "--http-port=8180"
//          << "--deploy-path=" + systemPrefix
//          << "--docroot=" + systemPrefix + "/docroot"
//          << "--approot=" + systemPrefix + "/approot"
//          << "--resources-dir=" + systemPrefix + "/resources";


//     std::vector<std::string> argHolder;
//     for (const QString &arg : args)
//         argHolder.push_back(arg.toStdString());

//     std::vector<char*> argv;
//     for (auto &arg : argHolder)
//         argv.push_back(arg.data());

//     int argc = argv.size();

//     serverThread_ = std::make_unique<std::thread>([this, argc, argv]() mutable {
//         try {
//             ServerResource myserverResource;
//             server_ = std::make_unique<Wt::WServer>(argc, argv.data(), WTHTTP_CONFIGURATION);
//             server_->addResource(&myserverResource, "/deliva/${data}/${business}");
//             server_->addEntryPoint(Wt::EntryPointType::Application,
//                                   std::bind(&createApplication, std::placeholders::_1),
//                                   OrdersUrl);

//             std::cerr << "\n\n -- Warning: Site is deployed at '" << OrdersUrl << "'\n\n";

//             if (server_->start()) {
//                 Q_EMIT started();

//                 QCoreApplication *qapp = QCoreApplication::instance();
//                 if (qapp) {
//                     QObject::connect(qapp, &QCoreApplication::aboutToQuit,
//                                      this, [this]() {
//                                          try { if (server_) server_->stop(); } catch (...) {}
//                                      }, Qt::QueuedConnection);
//                 }

// #ifndef WT_WIN32
//                 signal(SIGHUP, signalHandler);
// #endif
//                 signal(SIGTERM, signalHandler);
//                 signal(SIGABRT, signalHandler);
//                 signal(SIGINT, signalHandler);

//                 int sig = Wt::WServer::waitForShutdown();

//                 if (sig == 15) { }
//                 else if (sig == 6) { }
//                 else if (sig == 1) { }
//                 else if (sig == 2) { }
//                 else if (sig == 9) { }
//                 else { }

// #ifndef WT_WIN32
//                 if (sig == SIGHUP) {
//                     Wt::WServer::restart(argc, argv.data(), environ);
//                 }
// #endif
//             }

//             try { if (server_) server_->stop(); } catch (...) {}

//         } catch (Wt::WServer::Exception &e) {
//             std::cerr << "WServer exception: " << e.what() << std::endl;
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (std::exception &e) {
//             std::cerr << "exception: " << e.what() << std::endl;
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (...) {
//             std::cerr << "unknown exception in server thread" << std::endl;
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception in server thread"));
//         }

//         running_ = false;
//         Q_EMIT stopped();
//     });

//     return true;
// }

// void ServerController::stopServer() {
//     if (!running_.load()) return;

//     if (server_) {
//         try {
//             server_->stop();
//         } catch (const std::exception &ex) {
//             Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//         } catch (...) {
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception while stopping server"));
//         }
//     }

//     if (serverThread_ && serverThread_->joinable()) {
//         serverThread_->join();
//     }

//     serverThread_.reset();
//     server_.reset();
//     running_ = false;
//     Q_EMIT stopped();
// }


// // End Server Ops

// // For POSIX WServer::restart usage
// #if !defined(WT_WIN32)
// extern char **environ;
// #endif


// namespace {
//     // Signal flag: handlers set this; server thread may observe if needed
//     std::atomic<int> g_lastSignal{0};
//     void signalHandler(int sig) {
//         g_lastSignal.store(sig);
//         // Do not call non-async-safe functions here.
//     }
// }

// ServerController::ServerController(QObject *parent)
//   : QObject(parent)
// {}

// ServerController::~ServerController() {
//     stopServer();
// }

// bool ServerController::startServer(int port) {
//     if (running_.load()) return false;
//     running_ = true;
//     m_url = QStringLiteral("http://127.0.0.1:%1/").arg(port);
//     Q_EMIT urlChanged();

//     QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
//     if (downloadsPath.isEmpty()) {
//         downloadsPath = QDir::currentPath();
//     }

//     QDir baseDir(downloadsPath);
//     if (!baseDir.exists("Orders_files")) baseDir.mkdir("Orders_files");
//     baseDir.cd("Orders_files");
//     QString basePath = baseDir.absolutePath();

//     // Extract App.tar.xz using Boost
//     AssetExtractor extractor;
//     if (!extractor.extractLzmaArchive(":/serverassets/AppArchive", basePath)) {
//         qWarning() << "Failed to extract App.tar.xz archive";
//         return false;
//     }

//     m_assetPath = basePath;
//     Q_EMIT assetPathChanged();
//     qInfo() << "Assets extracted to:" << m_assetPath;

//     QString systemPrefix = basePath;
//     QStringList args;

//     args << "Orders"
//          << "--ssl-certificate=" + systemPrefix + "/encrypt/donate.crt"
//          << "--ssl-private-key=" + systemPrefix + "/encrypt/donate.key"
//          << "--ssl-tmp-dh=" + systemPrefix + "/encrypt/dhparam.pem"
//          << "--ssl-cipherlist=TLS13-CHACHA20-POLY1305-SHA256:TLS13-AES-128-GCM-SHA256:TLS13-AES-256-GCM-SHA384:ECDHE:!COMPLEMENTOFDEFAULT:!ECDHE-RSA-AES128-GCM-SHA256:!ECDHE-ECDSA-AES128-GCM-SHA256:!ECDHE-RSA-AES256-GCM-SHA384:!ECDHE-ECDSA-AES256-GCM-SHA384:!DHE-RSA-AES128-GCM-SHA256:!DHE-DSS-AES128-GCM-SHA256:!kEDH+AESGCM:!ECDHE-RSA-AES128-SHA256:!ECDHE-ECDSA-AES128-SHA256:!ECDHE-RSA-AES128-SHA:!ECDHE-ECDSA-AES128-SHA:!ECDHE-RSA-AES256-SHA384:!ECDHE-ECDSA-AES256-SHA384:!ECDHE-RSA-AES256-SHA:!ECDHE-ECDSA-AES256-SHA:!DHE-RSA-AES128-SHA256:!DHE-RSA-AES128-SHA:!DHE-DSS-AES128-SHA256:!DHE-RSA-AES256-SHA256:!DHE-DSS-AES256-SHA:!DHE-RSA-AES256-SHA:!AES128-GCM-SHA256:!AES256-GCM-SHA384:!AES128-SHA256:!AES256-SHA256:!AES128-SHA:!AES256-SHA:!AES:!CAMELLIA:!DES-CBC3-SHA:HIGH:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA:!TLSv1:!SSLv2:!SSLv3"
//          << "--ssl-prefer-server-ciphers=1"
//          << "--ssl-client-verification=none"
//          << "--https-listen=159.223.118.66:8180"
//          << "--deploy-path=" + systemPrefix
//          << "--docroot=" + systemPrefix + "/docroot"
//          << "--approot=" + systemPrefix + "/approot"
//          << "--resources-dir=" + systemPrefix + "/resources";

//     std::vector<std::string> argHolder;
//     for (const QString &arg : args)
//         argHolder.push_back(arg.toStdString());

//     std::vector<char*> argv;
//     for (auto &arg : argHolder)
//         argv.push_back(arg.data());

//     int argc = argv.size();

//     serverThread_ = std::make_unique<std::thread>([this, argc, argv]() mutable {
//         try {
//             ServerResource myserverResource;
//             server_ = std::make_unique<Wt::WServer>(argc, argv.data(), WTHTTP_CONFIGURATION);
//             server_->addResource(&myserverResource, "/deliva/${data}/${business}");
//             server_->addEntryPoint(Wt::EntryPointType::Application,
//                                   std::bind(&createApplication, std::placeholders::_1),
//                                   OrdersUrl);

//             std::cerr << "\n\n -- Warning: Site is deployed at '" << OrdersUrl << "'\n\n";

//             if (server_->start()) {
//                 Q_EMIT started();

//                 QCoreApplication *qapp = QCoreApplication::instance();
//                 if (qapp) {
//                     QObject::connect(qapp, &QCoreApplication::aboutToQuit,
//                                      this, [this]() {
//                                          try { if (server_) server_->stop(); } catch (...) {}
//                                      }, Qt::QueuedConnection);
//                 }

// #ifndef WT_WIN32
//                 signal(SIGHUP, signalHandler);
// #endif
//                 signal(SIGTERM, signalHandler);
//                 signal(SIGABRT, signalHandler);
//                 signal(SIGINT, signalHandler);

//                 int sig = Wt::WServer::waitForShutdown();

//                 if (sig == 15) { }
//                 else if (sig == 6) { }
//                 else if (sig == 1) { }
//                 else if (sig == 2) { }
//                 else if (sig == 9) { }
//                 else { }

// #ifndef WT_WIN32
//                 if (sig == SIGHUP) {
//                     Wt::WServer::restart(argc, argv.data(), environ);
//                 }
// #endif
//             }

//             try { if (server_) server_->stop(); } catch (...) {}

//         } catch (Wt::WServer::Exception &e) {
//             std::cerr << "WServer exception: " << e.what() << std::endl;
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (std::exception &e) {
//             std::cerr << "exception: " << e.what() << std::endl;
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (...) {
//             std::cerr << "unknown exception in server thread" << std::endl;
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception in server thread"));
//         }

//         running_ = false;
//         Q_EMIT stopped();
//     });

//     return true;
// }

// void ServerController::stopServer() {
//     if (!running_.load()) return;

//     if (server_) {
//         try {
//             server_->stop();
//         } catch (const std::exception &ex) {
//             Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//         } catch (...) {
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception while stopping server"));
//         }
//     }

//     if (serverThread_ && serverThread_->joinable()) {
//         serverThread_->join();
//     }

//     serverThread_.reset();
//     server_.reset();
//     running_ = false;
//     Q_EMIT stopped();
// }


// // End Server Ops

// // For POSIX WServer::restart usage
// #if !defined(WT_WIN32)
// extern char **environ;
// #endif

// namespace {
//     // Signal flag: handlers set this; server thread may observe if needed
//     std::atomic<int> g_lastSignal{0};
//     void signalHandler(int sig) {
//         g_lastSignal.store(sig);
//         // Do not call non-async-safe functions here.
//     }
// }

// ServerController::ServerController(QObject *parent)
//   : QObject(parent)
// {}

// ServerController::~ServerController() {
//     stopServer();
// }

// bool ServerController::startServer(int port) {
//     if (running_.load()) return false;
//     running_ = true;
//     m_url = QStringLiteral("http://127.0.0.1:%1/").arg(port);
//     Q_EMIT urlChanged();

//     // ---- Prepare asset folders in Downloads (synchronously, BEFORE server starts) ----
//     QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
//     if (downloadsPath.isEmpty()) {
//         downloadsPath = QDir::currentPath(); // fallback
//     }

//     QDir baseDir(downloadsPath);
//     // create isolated folder
//     if (!baseDir.exists("Orders_files")) baseDir.mkdir("Orders_files");
//     baseDir.cd("Orders_files");

//     if (!baseDir.exists("files")) baseDir.mkdir("files");
//     if (!baseDir.exists("approot")) baseDir.mkdir("approot");

//     QString configqrc = QStringLiteral(":/data-assets/config");
//     QString jsonqrc   = QStringLiteral(":/data-assets/json");
//     QString approotqrc= QStringLiteral(":/data-assets/approot");

//     QString configdir  = baseDir.filePath("files/config");
//     QString jsondir    = baseDir.filePath("files/json");
//     QString approotdir = baseDir.filePath("approot");

//     QDir().mkpath(configdir);
//     QDir().mkpath(jsondir);
//     QDir().mkpath(approotdir);

//     try {
//         CopyAssets ericassets; // requires CopyAssets implementation in your project
//         ericassets.cpDir(configqrc, configdir);
//         ericassets.cpDir(jsonqrc, jsondir);
//         ericassets.cpDir(approotqrc, approotdir);
//     } catch (...) {
//         qWarning() << "Warning: copying assets failed; ensure CopyAssets is available and correct.";
//     }

//     m_assetPath = baseDir.absolutePath();
//     Q_EMIT assetPathChanged();
//     qInfo() << "Assets copied to:" << m_assetPath;

//     // ---- Build argv-style args for Wt server (include approot as docroot) ----
//     std::vector<std::string> argHolder;
//     argHolder.push_back("app");
//     argHolder.push_back("--http-address");
//     argHolder.push_back("127.0.0.1");
//     argHolder.push_back("--http-port");
//     argHolder.push_back(std::to_string(port));
//     argHolder.push_back("--docroot");
//     argHolder.push_back(approotdir.toStdString());

//     std::vector<char*> argv;
//     argv.reserve(argHolder.size());
//     for (auto &s : argHolder) argv.push_back(const_cast<char*>(s.c_str()));
//     int argc = static_cast<int>(argv.size());

//     // ---- Launch the server in a background thread (serverThread_ joinable) ----
//     serverThread_ = std::make_unique<std::thread>([this, argc, argv]() mutable {
//         try {
//             ServerResource myserverResource; // uses your ServerResource type

//             // Construct server using your project's WTHTTP_CONFIGURATION macro (same as original)
//             server_ = std::make_unique<Wt::WServer>(argc, argv.data(), WTHTTP_CONFIGURATION);

//             // register resource and entrypoint just like your original main():
//             server_->addResource(&myserverResource, "/deliva/${data}/${business}");
//             server_->addEntryPoint(Wt::EntryPointType::Application,
//                                   std::bind(&createApplication, std::placeholders::_1),
//                                   OrdersUrl);

//             std::cerr << "\n\n -- Warning: Site is deployed at '" << OrdersUrl << "'\n\n";

//             if (server_->start()) {
//                 Q_EMIT started();

//                 // make sure server stops when Qt app quits:
//                 QCoreApplication *qapp = QCoreApplication::instance();
//                 if (qapp) {
//                     QObject::connect(qapp, &QCoreApplication::aboutToQuit,
//                                      this, [this]() {
//                                          try { if (server_) server_->stop(); } catch (...) {}
//                                      }, Qt::QueuedConnection);

//                 }

//                 // Install signal handlers (set atomic flag)
// #ifndef WT_WIN32
//                 signal(SIGHUP, signalHandler);
// #endif
//                 signal(SIGTERM, signalHandler);
//                 signal(SIGABRT, signalHandler);
//                 signal(SIGINT, signalHandler);
//                 // SIGKILL cannot be handled

//                 // wait for shutdown (blocks here until server->stop() or signal)
//                 int sig = Wt::WServer::waitForShutdown();

//                 // Mirror your original signal checks (no-ops here)
//                 if (sig == 15) { /* SIGTERM */ }
//                 else if (sig == 6) { /* SIGABRT */ }
//                 else if (sig == 1) { /* SIGHUP */ }
//                 else if (sig == 2) { /* SIGINT */ }
//                 else if (sig == 9) { /* SIGKILL */ }
//                 else { /* other */}
// #ifndef WT_WIN32
//                 if (sig == SIGHUP) {
//                     // restart as original attempted:
//                     Wt::WServer::restart(argc, argv.data(), environ);
//                 }
// #endif
//             }

//             // ensure stopped
//             try { if (server_) server_->stop(); } catch (...) {}

//         } catch (Wt::WServer::Exception &e) {
//             std::cerr << "WServer exception: " << e.what() << std::endl;
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (std::exception &e) {
//             std::cerr << "exception: " << e.what() << std::endl;
//             Q_EMIT errorOccurred(QString::fromUtf8(e.what()));
//         } catch (...) {
//             std::cerr << "unknown exception in server thread" << std::endl;
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception in server thread"));
//         }

//         running_ = false;
//         Q_EMIT stopped();
//     });

//     // keep serverThread_ joinable; stopServer will join it.
//     return true;
// }

// void ServerController::stopServer() {
//     if (!running_.load()) return;

//     if (server_) {
//         try {
//             server_->stop();
//         } catch (const std::exception &ex) {
//             Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
//         } catch (...) {
//             Q_EMIT errorOccurred(QStringLiteral("Unknown exception while stopping server"));
//         }
//     }

//     if (serverThread_ && serverThread_->joinable()) {
//         serverThread_->join();
//     }

//     serverThread_.reset();
//     server_.reset();
//     running_ = false;
//     Q_EMIT stopped();
// }
