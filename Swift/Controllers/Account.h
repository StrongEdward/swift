/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

/*
 * Copyright (c) 2010-2015 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#pragma once

#include <string>

#include <Swiften/Base/boost_bsignals.h>
#include <Swiften/Client/ClientOptions.h>
#include <Swiften/JID/JID.h>

namespace Swift {
	class JID;
	class ProfileSettingsProvider;

	class Account {
		public:
			Account() {}
			Account(ProfileSettingsProvider* profileSettings);
			Account(const std::string accountName,
					const std::string jid,
					const std::string password,
					const std::string certificatePath,
					const ClientOptions options,
					bool rememberPassword,
					bool autoLogin,
					bool enabled,
					ProfileSettingsProvider* profileSettings);
			~Account();

			void clearPassword();

			// Getters
			std::string getAccountName();
			const JID& getJID();
			const std::string& getPassword();
			const std::string& getCertificatePath();
			const ClientOptions& getClientOptions();
			bool forgetPassword();
			bool loginAutomatically();
			bool isEnabled();
			ProfileSettingsProvider* getProfileSettings();

			// Setters
			void setAccountName(const std::string& newName);
			void setJID(const std::string& newJID);
			void setPassword(const std::string& newPassword);
			void setCertificatePath(const std::string& newPath);
			void setClientOptions(const ClientOptions& newOptions);
			void setRememberPassword(bool remember);
			void setLoginAutomatically(bool autoLogin);
			void setEnabled(bool enabled);

			// Signals
			boost::signal<void ()> onEnabled;

		private:
			std::string serializeClientOptions(const ClientOptions& options);
			ClientOptions parseClientOptions(const std::string& optionString);

		private:
			std::string accountName_;
			JID jid_;
			std::string password_;
			std::string certificatePath_;
			ClientOptions clientOptions_;
			bool rememberPassword_;
			bool autoLogin_;
			bool enabled_;

			ProfileSettingsProvider* profileSettings_;

	};
}
