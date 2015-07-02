/*
 * Copyright (c) 2015 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace Swift {

struct TLSOptions {
	TLSOptions() : schannelTLS1_0Workaround(false) {
	}

	/**
	* A bug in the Windows SChannel TLS stack, combined with
	* overly-restrictive server stacks means it's sometimes necessary to
	* not use TLS>1.0. This option has no effect unless compiled on
	* Windows against SChannel (OpenSSL users are unaffected).
	*/
	bool schannelTLS1_0Workaround;

	bool operator==(const TLSOptions& other) const {
	    return schannelTLS1_0Workaround == other.schannelTLS1_0Workaround;
	}
};
}

namespace boost {
namespace serialization {

template<typename Archive>
void serialize(Archive& ar, Swift::TLSOptions& o, const unsigned int) {
	ar & o.schannelTLS1_0Workaround;
}

}
}
