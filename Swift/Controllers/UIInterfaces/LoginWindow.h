/*
 * Copyright (c) 2010-2012 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <Swiften/Base/boost_bsignals.h>
#include <boost/shared_ptr.hpp>

#include <string>
#include <Swiften/TLS/Certificate.h>
#include <Swiften/TLS/CertificateWithKey.h>
#include <Swiften/Client/ClientOptions.h>
#include <Swift/Controllers/AccountsManager.h>
//#include <Swift/Controllers/UIInterfaces/AccountsList.h>

namespace Swift {
	class MainWindow;
	class LoginWindow {
		public:
			virtual ~LoginWindow() {}
			virtual void selectUser(const std::string&) = 0;
			virtual bool isInMultiaccountView() = 0;
			virtual void morphInto(MainWindow *mainWindow) = 0;
			virtual void loggedOut() = 0;
			virtual void setShowNotificationToggle(bool) = 0;
			virtual void setMessage(const std::string&) = 0;
			virtual void setIsLoggingIn(bool loggingIn) = 0;
			virtual void setAccountsManager(AccountsManager* manager) = 0;
			virtual void addAvailableAccount(boost::shared_ptr<Account> account) = 0;
			virtual void removeAvailableAccount(int index) = 0;
			virtual void clearPassword() = 0;
			/** The certificate is what is used for login, the certificatePath is used for remembering paths to populate the loginwindow with*/
			boost::signal<void (const std::string&, const std::string&, const std::string& /*CertificatePath*/, const ClientOptions& /*options*/, bool /* remember password*/, bool /* login automatically */)> onLoginRequest;
			virtual void setLoginAutomatically(bool loginAutomatically) = 0;
			virtual void quit() = 0;
			/** Blocking request whether a cert should be permanently trusted.*/
			virtual bool askUserToTrustCertificatePermanently(const std::string& message, const std::vector<Certificate::ref>& certificateChain) = 0;
			//virtual AccountsList* getAccountsList() = 0;

			boost::signal<void (const std::string&)> onCancelLoginRequest;
			boost::signal<void ()> onQuitRequest;
			boost::signal<void (const std::string&)> onPurgeSavedLoginRequest;
			boost::signal<void (int index)> onDefaultAccountChanged;
	};
}
