/*
 * Copyright (c) 2010 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */


#pragma once

#include "Swift/Controllers/UIInterfaces/MainWindowFactory.h"
#include "Swift/Controllers/UnitTest/MockMainWindow.h"

namespace Swift {

	class MockMainWindowFactory : public MainWindowFactory {
		public:
			MockMainWindowFactory(boost::shared_ptr<Account> fakeAccount) : last(NULL), account(fakeAccount) {}

			virtual ~MockMainWindowFactory() {}

			/**
			 * Transfers ownership of result.
			 */
			virtual MainWindow* createMainWindow(UIEventStream*) {last = new MockMainWindow(account);return last;}
			MockMainWindow* last;
			boost::shared_ptr<Account> account;
	};
}


