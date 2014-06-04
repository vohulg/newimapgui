#include "tcontact.h"

TContact::TContact(const QString& accountId, QObject *parent) :
    QObject(parent), AccountId(accountId)
{
}
