/*
 * Copyright (c) 2011-2015 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <boost/serialization/split_free.hpp>
#include <boost/shared_ptr.hpp>

#include <Swiften/Base/API.h>
#include <Swiften/Base/URL.h>
#include <Swiften/Base/SafeByteArray.h>
#include <Swiften/Base/SafeString.h>
#include <Swiften/TLS/TLSOptions.h>


namespace Swift {
	class HTTPTrafficFilter;

	struct SWIFTEN_API ClientOptions {
		enum UseTLS {
			NeverUseTLS,
			UseTLSWhenAvailable,
			RequireTLS
		};

		enum ProxyType {
			NoProxy,
			SystemConfiguredProxy,
			SOCKS5Proxy,
			HTTPConnectProxy
		};

		ClientOptions() : 
				useStreamCompression(true), 
				useTLS(UseTLSWhenAvailable), 
				allowPLAINWithoutTLS(false), 
				useStreamResumption(false), 
				forgetPassword(false), 
				useAcks(true), 
				manualHostname(""),
				manualPort(-1),
				proxyType(SystemConfiguredProxy),
				manualProxyHostname(""),
				manualProxyPort(-1),
				boshHTTPConnectProxyAuthID(""), 
				boshHTTPConnectProxyAuthPassword("") {
		}

		/**
		 * Whether ZLib stream compression should be used when available.
		 *
		 * Default: true
		 */
		bool useStreamCompression;

		/**
		 * Sets whether TLS encryption should be used.
		 *
		 * Default: UseTLSWhenAvailable
		 */
		UseTLS useTLS;

		/**
		 * Sets whether plaintext authentication is 
		 * allowed over non-TLS-encrypted connections.
		 *
		 * Default: false
		 */
		bool allowPLAINWithoutTLS;

		/**
		 * Use XEP-196 stream resumption when available.
		 *
		 * Default: false
		 */
		bool useStreamResumption;

		/**
		 * Forget the password once it's used.
		 * This makes the Client useless after the first login attempt.
		 *
		 * FIXME: This is a temporary workaround.
		 *
		 * Default: false
		 */
		bool forgetPassword;

		/**
		 * Use XEP-0198 acks in the stream when available.
		 * Default: true
		 */
		bool useAcks;

		/**
		 * The hostname to connect to.
		 * Leave this empty for standard XMPP connection, based on the JID domain.
		 */
		std::string manualHostname;

		/**
		 * The port to connect to.
		 * Leave this to -1 to use the port discovered by SRV lookups, and 5222 as a
		 * fallback.
		 */
		int manualPort;

		/**
		 * The type of proxy to use for connecting to the XMPP
		 * server.
		 */
		ProxyType proxyType;

		/**
		 * Override the system-configured proxy hostname.
		 */
		std::string manualProxyHostname;

		/**
		 * Override the system-configured proxy port.
		 */
		int manualProxyPort;

		/**
		 * If non-empty, use BOSH instead of direct TCP, with the given URL.
		 * Default: empty (no BOSH)
		 */
		URL boshURL;

		/**
		 * If non-empty, BOSH connections will try to connect over this HTTP CONNECT
		 * proxy instead of directly.
		 * Default: empty (no proxy)
		 */
		URL boshHTTPConnectProxyURL;

		/**
		 * If this and matching Password are non-empty, BOSH connections over
		 * HTTP CONNECT proxies will use these credentials for proxy access.
		 * Default: empty (no authentication needed by the proxy)
		 */
		SafeString boshHTTPConnectProxyAuthID;
		SafeString boshHTTPConnectProxyAuthPassword;

		/**
		 * This can be initialized with a custom HTTPTrafficFilter, which allows HTTP CONNECT
		 * proxy initialization to be customized.
		 */
		boost::shared_ptr<HTTPTrafficFilter> httpTrafficFilter;

		/**
		 * Options passed to the TLS stack
		 */
		TLSOptions tlsOptions;

		bool operator ==(const ClientOptions& o) const {
			return useStreamCompression == o.useStreamCompression &&
				useTLS == o.useTLS &&
				allowPLAINWithoutTLS == o.allowPLAINWithoutTLS &&
				useStreamResumption == o.useStreamResumption &&
				forgetPassword == o.forgetPassword &&
				useAcks == o.useAcks &&
				manualHostname == o.manualHostname &&
				manualPort == o.manualPort &&
				proxyType == o.proxyType &&
				manualProxyHostname == o.manualProxyHostname &&
				manualProxyPort == o.manualProxyPort &&
				boshURL.toString() == o.boshURL.toString() &&
				boshHTTPConnectProxyURL.toString() == o.boshHTTPConnectProxyURL.toString() &&
				safeByteArrayToString(SafeByteArray(boshHTTPConnectProxyAuthID)) == safeByteArrayToString(SafeByteArray(o.boshHTTPConnectProxyAuthID)) &&
				safeByteArrayToString(SafeByteArray(boshHTTPConnectProxyAuthPassword)) == safeByteArrayToString(SafeByteArray(o.boshHTTPConnectProxyAuthPassword)) &&
				httpTrafficFilter == o.httpTrafficFilter &&
				tlsOptions == o.tlsOptions;
		}
	};
}

namespace boost {
namespace serialization {

template<class Archive>
void save(Archive& ar, const Swift::ClientOptions& o, const unsigned int) {
	ar << o.useStreamCompression;
	ar << o.useTLS;
	ar << o.allowPLAINWithoutTLS;
	ar << o.useStreamResumption;
	ar << o.forgetPassword;
	ar << o.useAcks;
	ar << o.manualHostname;
	ar << o.manualPort;
	ar << o.proxyType;
	ar << o.manualProxyHostname;
	ar << o.manualProxyPort;

	std::string boshURL = o.boshURL.toString();
	ar << boshURL;

	std::string boshHTTPConnectProxyURL = o.boshHTTPConnectProxyURL.toString();
	ar << boshHTTPConnectProxyURL;

	// Warning: Breaking the safety
	std::string ID = Swift::safeByteArrayToString(Swift::SafeByteArray(o.boshHTTPConnectProxyAuthID));
	ar << ID;

	// Warning: Breaking the safety
	std::string password = Swift::safeByteArrayToString(Swift::SafeByteArray(o.boshHTTPConnectProxyAuthPassword));
	ar << password;

	ar << o.tlsOptions;
}

template<class Archive>
void load(Archive& ar, Swift::ClientOptions& o, const unsigned int) {
	ar >> o.useStreamCompression;
	ar >> o.useTLS;
	ar >> o.allowPLAINWithoutTLS;
	ar >> o.useStreamResumption;
	ar >> o.forgetPassword;
	ar >> o.useAcks;
	ar >> o.manualHostname;
	ar >> o.manualPort;
	ar >> o.proxyType;
	ar >> o.manualProxyHostname;
	ar >> o.manualProxyPort;

	std::string boshURL;
	ar >> boshURL;
	o.boshURL = Swift::URL::fromString(boshURL);

	std::string boshHTTPConnectProxyURL;
	ar >> boshHTTPConnectProxyURL;
	o.boshHTTPConnectProxyURL = Swift::URL::fromString(boshHTTPConnectProxyURL);

	std::string ID;
	ar >> ID;
	o.boshHTTPConnectProxyAuthID = Swift::SafeString(ID);

	std::string password;
	ar >> password;
	o.boshHTTPConnectProxyAuthPassword = Swift::SafeString(password);

	ar >> o.tlsOptions;
}

} // namespace serialization
} // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(Swift::ClientOptions)
