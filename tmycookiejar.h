#ifndef TMYCOOKIEJAR_H
#define TMYCOOKIEJAR_H

#include <QNetworkCookieJar>
#include <QList>
#include <QNetworkCookie>

class TMyCookieJar : public QNetworkCookieJar
{
public:
    TMyCookieJar();
    virtual ~TMyCookieJar();

    QList<QNetworkCookie> getAllCookies();
    void setAllSitesCookies(const QList<QNetworkCookie>& cookieList);
};

#endif // TMYCOOKIEJAR_H
