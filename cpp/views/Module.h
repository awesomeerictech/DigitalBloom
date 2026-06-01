#ifndef WT_TARGET_JAVA
//#include <Wt/WWidget.h>

#define MODULE_BEGIN(name) \
  namespace { \
    std::unique_ptr<Wt::WWidget> name() {

#define MODULE_BEGIN2(type, name) \
  namespace { \
    type name() {

#define MODULE_END(...) __VA_ARGS__; } }

#else

#ifdef MODULE_BEGIN
#undef MODULE_BEGIN
#endif

#define MODULE_BEGIN(name) \
  namespace { \
    extern std::unique_ptr<Wt::WWidget> name() {

#ifdef MODULE_BEGIN2
#undef MODULE_BEGIN2
#endif

#define MODULE_BEGIN2(type, name) \
  namespace { \
    extern type name() {

#ifdef MODULE_END
#undef MODULE_END
#endif

#define MODULE_END(...) __VA_ARGS__; } }

#endif
