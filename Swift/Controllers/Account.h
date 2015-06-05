/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <string>

#include <Swiften/Client/ClientOptions.h>
#include <Swiften/JID/JID.h>

namespace Swift {
	class JID;

	class Account {
		public:
			Account() {}
			Account(const std::string& accountName,
					const std::string& jid,
					const std::string& password,
					const std::string& certificatePath,
					const ClientOptions& options,
					bool rememberPassword,
					bool enabledAutoLogin);

		public: // public temporarily
			std::string accountName_;
			JID jid_;
			std::string password_;
			std::string certificatePath_;
			ClientOptions options_;
			bool rememberPassword_;
			bool enabledAutoLogin_;
	};
}
