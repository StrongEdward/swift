/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <string>

#include <Swiften/Client/ClientOptions.h>

namespace Swift {

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

		public: // maybe struct?
			std::string accountName_;
			std::string jid_;
			std::string password_;
			std::string certificatePath_;
			ClientOptions options_;
			bool rememberPassword_;
			bool enabledAutoLogin_;
	};
}
