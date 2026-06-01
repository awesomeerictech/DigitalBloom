#ifndef MAPPERSOFTWARE_H
#define MAPPERSOFTWARE_H

#include "json.hpp"
#include "CopyAssets.h"
#include "base/DeferredWidget.h"
#include "views/MainTemplate.h"
#include "components/AnchorRegister.cpp"
#include "components/AnchorRetrieve.cpp"
#include "components/AnchorSetUp.cpp"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <Wt/WAbstractItemModel.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WString.h>
#include "base/CsvUtil.h"
#include "base/DataNet.h"
#include "components/AnchorAllItems.cpp"
#include "components/AnchorNewItem.cpp"
#include "components/AnchorUpdateItems.cpp"
#include <Wt/WString.h>
#include <Wt/WText.h>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "views/AppRoles.h"
#include "views/AppLinks.h"
#include "tools/NotificationManager.h"
#include "orderserver/DataUploader.h"
#include "orderserver/LibArchiveQmlBridge.h"


class MapperSoftware : public Wt::WApplication
{



public:

  MapperSoftware(const Wt::WEnvironment& myenv);
   ~MapperSoftware();
  std::shared_ptr<Wt::WValidator> createNameValidator();
  std::shared_ptr<Wt::WValidator> createStrValidator(int maxlen);
  void handleSpecificPathChange(const std::string&);
  void setInternalBasePath(const std::string& basePath);
  bool yearMonthDiffer(const Wt::WDateTime& dt1, const Wt::WDateTime& dt2);
  std::unique_ptr<Wt::WWidget> allitems();
  std::unique_ptr<Wt::WWidget> newitem();
  std::unique_ptr<Wt::WWidget> updateitem();
  std::unique_ptr<Wt::WWidget> newmenuitem();
  std::unique_ptr<Wt::WWidget> thesettings();
  std::unique_ptr<Wt::WWidget> therequests();
  std::unique_ptr<Wt::WWidget> thehelp1();
  std::shared_ptr<DataUploader> massuploader;
  std::shared_ptr<LibArchiveQmlBridge> extractionbridge;
  std::shared_ptr<DataNet> networkutils;
  std::shared_ptr<MyJsonTools> mypayload;
  std::shared_ptr<eric::JsonUtils> myjsonutils;
  std::shared_ptr<Wt::WSortFilterProxyModel> idsfilter;
  std::shared_ptr<Wt::WSortFilterProxyModel> cryptocenterfilter;
  std::shared_ptr<Wt::WSortFilterProxyModel> catsfilter;
  std::shared_ptr<Wt::WSortFilterProxyModel> itemsfilter;
  std::shared_ptr<Wt::WSortFilterProxyModel> ordersfilter;
  std::shared_ptr<Wt::WSortFilterProxyModel> carouselfilter;
  std::shared_ptr<Wt::WSortFilterProxyModel> deliveryfilter;
  std::shared_ptr<Wt::WSortFilterProxyModel> metricsfilter;
  std::shared_ptr<Wt::WSortFilterProxyModel> logisticsfilter;
  std::shared_ptr<Wt::WSortFilterProxyModel> locationsfilter;
  std::shared_ptr<Wt::WStandardItemModel> idsstandardmodel;
  std::shared_ptr<Wt::WStandardItemModel> cryptocenterstandardmodel;
  std::shared_ptr<Wt::WStandardItemModel> catsstandardmodel;
  std::shared_ptr<Wt::WStandardItemModel> itemsstandardmodel;
  std::shared_ptr<Wt::WStandardItemModel> ordersstandardmodel;
  std::shared_ptr<Wt::WStandardItemModel> logisticsstandardmodel;
  std::shared_ptr<Wt::WStandardItemModel> deliverystandardmodel;
  std::shared_ptr<Wt::WStandardItemModel> metricsstandardmodel;
  std::shared_ptr<Wt::WStandardItemModel> locationsstandardmodel;
  std::shared_ptr<Wt::WStandardItemModel> carouselstandardmodel;
  bool isstartup;
  Wt::WComboBox * combocategoryupdate;
  Wt::WLineEdit * updateitemname;
  Wt::WLineEdit * updatequantity;
  Wt::WLineEdit * updateprice;
  Wt::WTextArea * updateitemdescription;
  Wt::WLineEdit * updatestandardized;
  Wt::WLineEdit * updatestandardizedmedia;
  Wt::WLineEdit * updateitemuuid;
  Wt::WLineEdit * updatemanufacturer;
  Wt::WLineEdit * searchitems_;
  std::unique_ptr<std::regex> mypatternitem;
  std::unique_ptr<std::regex> mypatterncarousel;
  Wt::WString retainmedianame = "";
  Wt::WString retainuniqueid = "";
  QVariantMap deletemap = QVariantMap();
  QMultiMap<QString, QVariant> requestsmap = QMultiMap<QString, QVariant>();
  QMultiMap<QString, QVariant> usersmap = QMultiMap<QString, QVariant>();
  QMultiMap<QString, QVariant> locationmap = QMultiMap<QString, QVariant>();
  QMultiMap<QString, QVariant> categoriesmap = QMultiMap<QString, QVariant>();
  QMultiMap<QString, QVariant> itemsmap = QMultiMap<QString, QVariant>();
  QMultiMap<QString, QVariant> ordersmap = QMultiMap<QString, QVariant>();
  QMultiMap<QString, QVariant> logisticsmap = QMultiMap<QString, QVariant>();
  QMultiMap<QString, QVariant> deliverymap = QMultiMap<QString, QVariant>();
  QMultiMap<QString, QVariant> metricsmap = QMultiMap<QString, QVariant>();
  QMultiMap<QString, QVariant> advertisementmap = QMultiMap<QString, QVariant>();
  QString businessusername = QString();
  QString bizusername = QString();
  QString businessuuid = QString();
  QString businessphonenumber = QString();
  int businesssubscription = -1;
  int businessblacklist = -1;
  int businessresponsibilities = -1;
  int businesspolicy = -1;
  int businessverification = -1;
  bool isnew = true;
  bool inmap = false;
  QString reqidids = "";
  QString reqrecipient = "";
  QString reqdeliverer = "";
  QString reqlocationto = "";
  QString reqlocationfrom = "";
  QString businessavatar = QString();
  QString businesswhichbiz = QString();
  QString businessemail = QString();
  std::unique_ptr<Wt::WTimer> timer1;
  std::unique_ptr<Wt::WTimer> timer2;
  std::unique_ptr<Wt::WTimer> timer3;
  std::unique_ptr<Wt::WTimer> notifytimer3;
  std::unique_ptr<Wt::WTimer> timerdata1;
  std::unique_ptr<Wt::WTimer> timerdata2;
  std::unique_ptr<Wt::WTimer> timerloginuser;
  std::unique_ptr<Wt::WTimer> timerregisteruser;
  std::unique_ptr<Wt::WTimer> timerretrievepassword;
  std::unique_ptr<Wt::WTimer> timergetdataall;
  std::unique_ptr<Wt::WTimer> timergetdatauser;
  std::unique_ptr<Wt::WTimer> timerupdateuser;
  std::unique_ptr<Wt::WTimer> timernewcategory;
  std::unique_ptr<Wt::WTimer> timerdeletecategory;
  std::unique_ptr<Wt::WTimer> timeritemnew;
  std::unique_ptr<Wt::WTimer> timerdeleteitem;
  std::unique_ptr<Wt::WTimer> timerneworder;
  std::unique_ptr<Wt::WTimer> timerdeleteorder;
  std::unique_ptr<Wt::WTimer> timernewlogistics;
  std::unique_ptr<Wt::WTimer> timerdeletelogistics;
  std::unique_ptr<Wt::WTimer> timernewdelivery;
  std::unique_ptr<Wt::WTimer> timerdeletedelivery;
  std::unique_ptr<Wt::WTimer> timernewlocation;
  std::unique_ptr<Wt::WTimer> timerdeletelocation;
  std::unique_ptr<Wt::WTimer> timernewmetrics;
  std::unique_ptr<Wt::WTimer> timerdeletemetrics;
  std::unique_ptr<Wt::WTimer> timerdeletewidget;
  std::unique_ptr<Wt::WTimer> timercarousel;
  std::unique_ptr<Wt::WTimer> timerstartup;
  std::unique_ptr<Wt::WTimer> timerdelay;
  std::unique_ptr<Wt::WTimer> timerrequestaction;
  std::unique_ptr<Wt::WTimer> timergeneric1;
  std::unique_ptr<Wt::WTimer> timergeneric2;
  std::unique_ptr<Wt::WTimer> timerupdatepassword;
  std::unique_ptr<Wt::WTimer> timerupdatephonenumber;
  std::unique_ptr<Wt::WTimer> timerlogout;
  NotificationManager *m_notificationManager = nullptr;
  void timeouttimer3();
  void timeoutnotifytimer3();
  void removewidget();
  void removetpl(Wt::WTemplate * mytpl=nullptr,std::string name="");
  void loginuser();
  void registeruser();
  void retrievepassword();
  void thedataall();
  void thedatauser();
  QVariantMap userdata();
  void mycarousel();
  void thedatauserftr();
  void mycarouselftr();
  void theitemnew();
  void thedeleteitem();
  void theupdateuser();
  void thecategorynew();
  void thedeletecategory();
  void thelocationnew();
  void thedeletelocation();
  void theordernew();
  void thedeleteorder();
  void thelogisticsnew();
  void thedeletelogistics();
  void thedeliverynew();
  void thedeletedelivery();
  void runorderrequest();
  void thegenericdata1();
  void thegenericdata2();
  void theupdatepassword();
  void theupdatephonenumber();
  void thelogout();
  void themetricsnew();
  void thedeletemetrics();
  void mystartup();
  void mydelay();
  void showToast(const Wt::WString& msg);
  void customToast(const Wt::WString& tpl,std::string msg);
  void customToastUD(const Wt::WString& tpl,QVariantMap msg);
  void customToastID(const Wt::WString& tpl,QVariantMap msg);
  void showCustomWidgetC(const Wt::WString& tpl="", Wt::WContainerWidget * mywid=nullptr);
  void showCustomWidgetT(const Wt::WString& tpl="",std::string name ="", Wt::WTemplate * mytpl=nullptr);
  void refreshinternal();
  void mytimerendpointuser();
  std::shared_ptr<QString> myerrormessage;
  std::shared_ptr<QString> mysuccessmessage;
  Wt::WWidget * deletewidget;
  Wt::WWidget * delcustomwidget;
  Wt::WWidget * delcustomtpl;
  Wt::WWidget * delitem1;
  Wt::WWidget * delitem2;
  Wt::WWidget * delitem3;
  Wt::WWidget * delitem1a;
  Wt::WWidget * delitem2b;
  Wt::WWidget * delitem3c;
  Wt::WTemplate * header1_;
 // Associate each field with a unique string literal.
  // start Item New
  static const Wt::WFormModel::Field CategoryField;
  static const Wt::WFormModel::Field LocationField;
  static const Wt::WFormModel::Field ItemNameField;
  static const Wt::WFormModel::Field QuantityField;
  static const Wt::WFormModel::Field PriceField;
  static const Wt::WFormModel::Field DescriptionField;
  static const Wt::WFormModel::Field MediaField;
  static const Wt::WFormModel::Field StandardizedField;
  static const Wt::WFormModel::Field StandardizedMediaField;
  static const Wt::WFormModel::Field ItemUuidField;
  static const Wt::WFormModel::Field ManufacturerField;
  static const Wt::WFormModel::Field PasswordField;
  static const Wt::WFormModel::Field PhoneField;
  Wt::WTemplate *uploadtemplate_;
 // typedef std::map<std::string, std::string> CategoryMap;
 // std::map<std::string, std::string> mycategories;
//  std::map< std::string, std::vector<std::string> > datauser;
  std::vector<std::unique_ptr<Wt::WTemplate>> myvector;
  std::map< std::string, std::vector<std::string> > mycategories;
  typedef std::map< std::string, std::vector<std::string> > CategoryMap;
  MapperSoftware::CategoryMap getCategoryMap(std::map< std::string, std::vector<std::string> > mymap);
  static const CategoryMap categories;
  static const int MAX_LENGTH;
  static const int MAX_LONGLENGTH;
  std::shared_ptr<Wt::WValidator> createShortTextValidator(const std::string& field);
  std::shared_ptr<Wt::WValidator> createShortNTextValidator(const std::string& field);
  std::shared_ptr<Wt::WValidator> createLongTextValidator(const std::string& field);
  std::shared_ptr<Wt::WValidator> createBasicValidator();
  std::shared_ptr<Wt::WValidator> createOptionalValidator();
  int categoryModelnewRow(const std::string& data);
  int categoryModelupdateRow(const std::string& data);
  std::string categoryDatanew(int row);
  std::string categoryDataupdate(int row);
  Wt::WString userDatanew();
  Wt::WString userDataupdate();
  QVariantMap myuserDatanew(QString mediafile="");
  QVariantMap myuserDataupdate(QString mediafile="");
  std::shared_ptr<Wt::WFormModel> myformmodelnew_;
  std::shared_ptr<Wt::WStandardItemModel> categoryModelnew_;
  std::shared_ptr<Wt::WFormModel> myformmodelupdate_;
  std::shared_ptr<Wt::WStandardItemModel> categoryModelupdate_;
  Wt::WTemplateFormView *mytemplateformview_;
  Wt::WTemplateFormView *updatetemplateformview_;
  Wt::WTemplateFormView *categorytemplateformview_;
  std::shared_ptr<Wt::WFormModel> mycategoryformmodel_;
  Wt::WTemplateFormView *passupdatetemplateformview_;
  std::shared_ptr<Wt::WFormModel> passupdateformmodel_;
  Wt::WTemplateFormView *locinfotemplateformview_;
  std::shared_ptr<Wt::WFormModel> locinfoformmodel_;
  Wt::WTemplateFormView *phoneupdatetemplateformview_;
  std::shared_ptr<Wt::WFormModel> phoneupdateformmodel_;
  std::shared_ptr<Wt::WAbstractItemModel> categoryModelnew();
  std::shared_ptr<Wt::WAbstractItemModel> categoryModelupdate();
  void createmodelnew();
  void initModelnew();
  void createmodelupdate();
  void initModelupdate();
  void createviewnew();
  void processitemnew();
  void createviewupdate();
  void processitemupdate();
  void initializeModels();
  Wt::WPushButton *nextbtnitemnew_;
  Wt::WPushButton *nextbtnitemupdate_;
  Wt::WContainerWidget *containerupload_;
  Wt::WFileUpload *fu;
  Wt::WPushButton *uploadButton;
  Wt::WText *out;
  //End Item New
  int globalcarousel =0;
  int carouselcount =3;
  int counter =0;
  int reqcounter = 0;
  int makereqcount = 0;
  Wt::WPushButton * execbtn = nullptr;
  void convertToDate(Wt::WStandardItem *item);
  void convertToNumber(Wt::WStandardItem *item);
  Wt::WTemplate *allitems_;
  Wt::WTemplate *newitem_;
  Wt::WTemplate *updateitem_;
  Wt::WTemplate *allsettings_;
  Wt::WTemplate *allrequests_;
  Wt::WTemplate *helptpl1_;
 // Wt::WTemplate *helptpl2_;
  std::recursive_mutex    mutex_;
  void testthreads();
  void handlemenuitems(Wt::WMenuItem * myitem);
  void restdata(std::string rawData);
  void restnetworkErrordata(std::string rawData);
  void restserverErrordata(std::string rawData);
  void thetimer1(bool timeout);
  void thetimer2(bool timeout);
  void thetimer3(bool timeout);
  static void runapp();
  std::mutex some_mutex;
  Wt::WStackedWidget *contentsStack_;
  Wt::WPushButton *openMenuButton_;
  bool menuOpen_;
  Wt::WMenu * menu;
  Wt::WMenuItem *addToMenu(Wt::WMenu *menu,const Wt::WString& name);
  void toggleMenu();
  void openMenu();
  void closeMenu();
  void populateSubMenu(Wt::WMenu *menu);
  static Wt::WString reindent(const Wt::WString& text);
  Wt::WTemplate * appvieweric_;
  Wt::Http::Client *client_;
  QString userdue = "";
  QString loggeduserdue = "";
  void clearregister();
  void clearlogin();
  void clearretrieve();
  Wt::WAnchor *helppageanchor1_;
  Wt::WAnchor *helppageanchor2_;
  static const int MAX_DROPS = 25;




private:

  std::string basePath_;
  Wt::WTemplate *accsetup_;
  Wt::WTemplate *accregister_;
  Wt::WTemplate *accretrieve_;
  Wt::WLineEdit  *editusersetupscope;
  Wt::WLineEdit  *editpasssetupscope;
  Wt::WCheckBox *checkloginsetupscope;
  Wt::WPushButton *loginbtnscope;
  Wt::WLineEdit  *edituserretrievescope;
  Wt::WLineEdit  *edithintretrievescope;
  Wt::WPushButton *retrievebtnretrievescope;
  Wt::WLineEdit  *edituserregscope;
  Wt::WLineEdit  *editemailregscope;
  Wt::WLineEdit   *edithintregscope;
  Wt::WLineEdit  *editphoneregscope;
  Wt::WLineEdit *editpassreenterscope;
  Wt::WLineEdit *editpassregscope;
  Wt::WPushButton *registerbtnscope;
  Wt::WRegExpValidator *reentervalidatorscope;
  std::string showeye = "<i class='fas fa-eye' ></i>";
  std::string hideeye = "<i class='fas fa-eye-slash' ></i>";
  std::string homeicon = "<i class='fas fa-home fa-lg' ></i>";
  std::string signinicon = "<i class='fas fa-sign-in fa-lg' ></i>";
  std::string signouticon = "<i class='fas fa-sign-out fa-lg' ></i>";
  std::string arrowlefticon = "<i class='fas fa-arrow-left fa-lg' ></i>";
  std::string deleteicon = "<i class='fas fa-trash fa-lg' ></i>";
  std::string delivericon = "<i class='fas fa-compass fa-lg' ></i>";
  std::string infoicon = "<i class='fas fa-info fa-lg' ></i>";
  std::string infoiconc = "<i class='fas fa-info-circle fa-lg' ></i>";
  std::string usericonc = "<i class='fas fa-user-circle fa-lg' ></i>";
  std::string usericon = "<i class='fas fa-user fa-lg' ></i>";
  std::string carticon = "<i class='fas fa-cart-shopping fa-lg' ></i>";
  std::string carticonc = "<i class='fas fa-shopping-cart fa-lg' ></i>";
  std::string settingsicon = "<i class='fas fa-gear fa-lg' ></i>";
  std::string settingsiconc = "<i class='fas fa-gears fa-lg' ></i>";
  Wt::WText *showeyescope1;
  Wt::WText *hideeyescope1;
  Wt::WContainerWidget *showhideeyescope1;
  Wt::WText *showeyescope2;
  Wt::WText *hideeyescope2;
  Wt::WContainerWidget *showhideeyescope2;
  Wt::WText *showeyescope3;
  Wt::WText *hideeyescope3;
  Wt::WContainerWidget *showhideeyescope3;
  Wt::WText *showeyescope4;
  Wt::WText *hideeyescope4;
  Wt::WContainerWidget *showhideeyescope4;
  Wt::WText *showeyescope5;
  Wt::WText *hideeyescope5;
  Wt::WContainerWidget *showhideeyescope5;
  Wt::WContainerWidget *myaccounts_;
  Wt::WText *homeiconscope1_;
  Wt::WAnchor *homeiconanchor1_;
  Wt::WText *signiniconscope1_;
  Wt::WAnchor *signiniconanchor1_;
  Wt::WText *signouticonscope1_;
  Wt::WAnchor *signouticonanchor1_;
  Wt::WText *arrowlefticonscope1_;
  Wt::WAnchor *arrowlefticonanchor1_;
  Wt::WAnchor *arrowleftnewitemanchor_;
  Wt::WText *arrowleftnewitemtxt_;
  Wt::WAnchor *arrowleftupdateitemanchor_;
  Wt::WText *arrowleftupdateitemtxt_;
  Wt::WAnchor *arrowleftexitnewitemanchor_;
  Wt::WText *arrowleftexitnewitemtxt_;
  Wt::WAnchor *arrowleftexitupdateitemanchor_;
  Wt::WText *arrowleftexitupdateitemtxt_;
  Wt::WAnchor *arrowleftexitcategoriesanchor_;
  Wt::WText *arrowleftexitcategoriestxt_;
  Wt::WAnchor *arrowleftexitrequestsanchor_;
  Wt::WText *arrowleftexitrequeststxt_;
  Wt::WAnchor *arrowleftexithelptpl1anchor_;
  Wt::WText *arrowleftexithelptpl1txt_;
 /* Wt::WPushButton *categoriesbtn_;
  Wt::WPushButton *requestsbtn_; */
  Wt::WAnchor *categoriesbtn_;
  Wt::WText *categoriesbtntxt_;
  Wt::WAnchor *requestsbtn_;
  Wt::WText *requestsbtntxt_;
  bool douploadnewitem = false;
  bool doupdateitem = false;
  friend class DataNet;

  // Start FileDrop

  Wt::WTemplate *tpl_;
  Wt::WText *progress_;
  Wt::WContainerWidget *log_;
  Wt::WFileDropWidget *drop_;




  std::map<Wt::WFileDropWidget::File*, Wt::WContainerWidget*> icons_;
  std::map<Wt::WFileDropWidget::File*, Wt::WFileDropWidget::Directory*> dirFiles_;

  void handleDrop(std::vector<Wt::WFileDropWidget::File *> files);
  void tooLarge(Wt::WFileDropWidget::File *file, ::uint64_t);
  void failed(Wt::WFileDropWidget::File *file);
  void saveFile(Wt::WFileDropWidget::File *file);
  void cancelUpload();
  void updateProgressListener();

  void showProgress(::uint64_t current, ::uint64_t total);
  Wt::WContainerWidget* getIcon(Wt::WFileDropWidget::File *file);
  void addLogLine(const std::string& msg);

// End FileDrop

protected:


};

#endif // MAPPERSOFTWARE_H
