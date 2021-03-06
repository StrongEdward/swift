/*
 * Copyright (c) 2010 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include "Swiften/Elements/Presence.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Swift {

	class StatusTracker {
		public:
			StatusTracker();
			boost::shared_ptr<Presence> getNextPresence();
			void setRequestedPresence(boost::shared_ptr<Presence> presence);
			bool goAutoAway(const int& seconds);
			bool goAutoUnAway();
		private:
			boost::shared_ptr<Presence> queuedPresence_;
			bool isAutoAway_;
			boost::posix_time::ptime isAutoAwaySince_;
	};
}
