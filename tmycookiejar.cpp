#include "tmycookiejar.h"

TMyCookieJar::TMyCookieJar()
{
}

TMyCookieJar::~TMyCookieJar()
{
}


QList<QNetworkCookie> TMyCookieJar::getAllCookies()
{
 return this->allCookies();
}

void TMyCookieJar::setAllSitesCookies(const QList<QNetworkCookie>& cookieList)
{
 this->setAllCookies(cookieList);
}
