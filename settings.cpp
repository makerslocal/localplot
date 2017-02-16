#include "settings.h"

void init_localplot_settings()
{
    qDebug() << "QSettings: initializing.";
    // Only initialize settings if needed.
    if (QCoreApplication::organizationName() != ORGANIZATION_NAME)
    {
        qDebug() << "QSettings: setting org properties.";
        QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
        QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
        QCoreApplication::setApplicationName(APPLICATION_NAME);
    }
}
