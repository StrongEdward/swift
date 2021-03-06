/*
 * Copyright (c) 2011 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#include <Swiften/Network/NetworkEnvironment.h>

#include <Swiften/Network/NetworkInterface.h>
#include <Swiften/Network/HostAddress.h>
#include <Swiften/Base/foreach.h>

namespace Swift {

NetworkEnvironment::~NetworkEnvironment() {
}

HostAddress NetworkEnvironment::getLocalAddress() const {
	std::vector<NetworkInterface> networkInterfaces = getNetworkInterfaces();
	foreach (const NetworkInterface& iface, networkInterfaces) {
		if (!iface.isLoopback()) {
			foreach (const HostAddress& address, iface.getAddresses()) {
				if (address.getRawAddress().is_v4()) {
					return address;
				}
			}
		}
	}
	return HostAddress();
}

}
