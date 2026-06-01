#ifndef WT_TARGET_JAVA

#define SOFTWARE_BEGIN(name) \
  namespace { \
    std::unique_ptr<Subject> name() {

#define SOFTWARE_BEGIN2(type, name) \
  namespace { \
    type name() {

#define SOFTWARE_END(...) __VA_ARGS__; } }

#else

#ifdef SOFTWARE_BEGIN
#undef SOFTWARE_BEGIN
#endif

#define SOFTWARE_BEGIN(name) \
  namespace { \
    extern std::unique_ptr<Subject> name() {

#ifdef SOFTWARE_BEGIN2
#undef SOFTWARE_BEGIN2
#endif

#define SOFTWARE_BEGIN2(type, name) \
  namespace { \
    extern type name() {

#ifdef SOFTWARE_END
#undef SOFTWARE_END
#endif

#define SOFTWARE_END(...) __VA_ARGS__; } }

#endif
