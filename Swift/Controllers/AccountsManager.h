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

#include <vector>
#include <map>

#include <Swift/Controllers/Account.h>

namespace Swift {
	class Account;
	class SettingsProviderHierachy;
	class SettingsProvider;
	class LoginWindow;

	class MainController;
	class EventLoop;
	class UIEventStream;
	class UIFactory;
	class EventController;
	class NetworkFactories;
	class LoginWindow;
	class SystemTrayController;
	class SoundPlayer;
	class StoragesFactory;
	class CertificateStorageFactory;
	class Dock;
	class Notifier;
	class TogglableNotifier;
	class URIHandler;
	class IdleDetector;


	class AccountsManager {
		public:
			AccountsManager(EventLoop* eventLoop,
							UIEventStream *uiEventStream,
							EventController* eventController,
							NetworkFactories* networkFactories,
							UIFactory* uiFactories,
							SettingsProvider* settings,
							SystemTrayController* systemTrayController,
							SoundPlayer* soundPlayer,
							StoragesFactory* storagesFactory,
							CertificateStorageFactory* certificateStorageFactory,
							Dock* dock,
							Notifier* notifier,
							TogglableNotifier* togglableNotifier,
							URIHandler* uriHandler,
							IdleDetector* idleDetector,
							const std::map<std::string, std::string>& emoticons,
							bool useDelayForLatency);
			~AccountsManager();

			JID getDefaultJid();
			Account getAccountByJid(std::string jid); // assumption: jid is unique
			ClientOptions parseClientOptions(const std::string& optionString);

		private:
			std::vector<Account> accounts_;
			std::string defaultAccountJid_;
			std::vector<MainController*> mainControllers_;
			LoginWindow* loginWindow_;

			UIEventStream* uiEventStream_;
			SettingsProvider* settings_;

	};
}

