#ifndef UTILS_MATHS_H
#define UTILS_MATHS_H
/* ************************************************************************** */

//! Align buffer sizes to multiples of 'roundTo'
int roundTo(const int value, const int roundTo);

//! Map a number from range [a1-a2] to [b1:b2]
int mapNumber(const int value, const int a1, const int a2, const int b1, const int b2, bool checks = true);

/* ************************************************************************** */

//! Calculate haversine distance for linear distance (km)
double haversine_km(double lat1, double long1, double lat2, double long2);

//! Calculate haversine distance for linear distance (miles)
double haversine_mi(double lat1, double long1, double lat2, double long2);

/* ************************************************************************** */
#endif // UTILS_MATHS_H
