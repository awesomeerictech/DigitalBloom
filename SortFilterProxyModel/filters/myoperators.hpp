#ifndef MYOPERATORS_HPP
#define MYOPERATORS_HPP

#include <QVariant>



inline bool operator < ( QVariant rightValue, QVariant leftValue )
{



bool x =false;

if ( leftValue <  rightValue )
{
 x =true;
}

return x;







}

inline bool operator > ( QVariant rightValue, QVariant leftValue )
{



bool x =false;

if ( leftValue >  rightValue )
{
 x =true;
}

return x;







}


inline bool operator <= ( QVariant rightValue, QVariant leftValue )
{



bool x =false;

if ( leftValue <=  rightValue )
{
 x =true;
}

return x;







}

inline bool operator >= ( QVariant rightValue, QVariant leftValue )
{



bool x =false;

if ( leftValue >=  rightValue )
{
 x =true;
}

return x;







}



#endif // MYOPERATORS_HPP
