/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <vector>

#include <Swift/Controllers/Account.h>

namespace Swift {
	class SettingsProviderHierachy;
	class LoginWindow;

	class AccountsManager {
		public:
			AccountsManager(SettingsProviderHierachy* settings, LoginWindow* loginWindow);

			std::string getDefaultJid();
			Account getAccountByJid(std::string jid); // assumption: jid is unique
			ClientOptions parseClientOptions(const std::string& optionString);

		private:
			std::vector<Account> accounts_;
			std::string defaultAccountJid_;
			SettingsProviderHierachy* settings_;
			LoginWindow* loginWindow_;

	};
}

