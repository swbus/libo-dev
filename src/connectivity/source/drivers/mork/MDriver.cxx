/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "MDriver.hxx"
#include "MConnection.hxx"

#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>

using namespace connectivity::mork;

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL com_sun_star_comp_sdbc_MorkDriver_get_implementation(
    css::uno::XComponentContext* context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new MorkDriver(context));
}

MorkDriver::MorkDriver(const css::uno::Reference< css::uno::XComponentContext >& context):
    context_(context),
    m_xFactory(context_->getServiceManager(), css::uno::UNO_QUERY)
{
    SAL_INFO("connectivity.mork", "=> MorkDriver::MorkDriver()" );
}

OUString SAL_CALL MorkDriver::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString(MORK_DRIVER_IMPL_NAME);
}

sal_Bool SAL_CALL MorkDriver::supportsService(const OUString& serviceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > MorkDriver::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    return { "com.sun.star.sdbc.Driver" };
}

css::uno::Reference< css::sdbc::XConnection > MorkDriver::connect(
    OUString const & url,
    css::uno::Sequence< css::beans::PropertyValue > const & info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception)
{
    SAL_INFO("connectivity.mork", "=> MorkDriver::connect()" );

    (void) url; (void) info; // avoid warnings

    // Profile discovery
    css::uno::Reference<css::uno::XInterface> xInstance = context_->getServiceManager()->createInstanceWithContext("com.sun.star.mozilla.MozillaBootstrap", context_);
    OSL_ENSURE( xInstance.is(), "failed to create instance" );

    css::uno::Reference<::com::sun::star::mozilla::XMozillaBootstrap> xMozillaBootstrap(xInstance, css::uno::UNO_QUERY);
    OSL_ENSURE( xMozillaBootstrap.is(), "failed to create instance" );

    if (xMozillaBootstrap.is())
    {
        OUString defaultProfile = xMozillaBootstrap->getDefaultProfile(::com::sun::star::mozilla::MozillaProductType_Thunderbird);

        if (!defaultProfile.isEmpty())
        {
            m_sProfilePath = xMozillaBootstrap->getProfilePath(::com::sun::star::mozilla::MozillaProductType_Thunderbird, defaultProfile);
            SAL_INFO("connectivity.mork", "Using Thunderbird profile " << m_sProfilePath);
        }
    }

    css::uno::Reference< css::sdbc::XConnection > xCon;
    OConnection* pCon = new OConnection(this);
    xCon = pCon;    // important here because otherwise the connection could be deleted inside (refcount goes -> 0)
    pCon->construct(url, info);
    return xCon;
}

sal_Bool MorkDriver::acceptsURL(OUString const & url)
    throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception)
{
    SAL_INFO("connectivity.mork", "=> MorkDriver::acceptsURL()" );
    // Skip 'sdbc:mozab: part of URL

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    OUString aAddrbookURI(url.copy(nLen+1));
    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    OUString aAddrbookScheme;
    if ( nLen == -1 )
    {
        // There isn't any subschema: - but could be just subschema
        if ( !aAddrbookURI.isEmpty() )
        {
            aAddrbookScheme= aAddrbookURI;
        }
        else if( url == "sdbc:address:" )
        {
            return false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        aAddrbookScheme = aAddrbookURI.copy(0, nLen);
    }

    return aAddrbookScheme == "thunderbird" || aAddrbookScheme == "mozilla";
}

css::uno::Sequence< css::sdbc::DriverPropertyInfo > MorkDriver::getPropertyInfo(
    OUString const & url,
    css::uno::Sequence< css::beans::PropertyValue > const & info)
    throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception)
{
    //... TODO
    (void) url; (void) info; // avoid warnings
    return css::uno::Sequence< css::sdbc::DriverPropertyInfo >();
}

sal_Int32 MorkDriver::getMajorVersion() throw (css::uno::RuntimeException, std::exception) {
    //... TODO
    return 0;
}

sal_Int32 MorkDriver::getMinorVersion() throw (css::uno::RuntimeException, std::exception) {
    //... TODO
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
