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
#include <string>

namespace Swift {
	class CertificateStorageFactory;
	class Dock;
	class EventLoop;
	class IdleDetector;
	class LoginWindow;
	class MainController;
	class NetworkFactories;
	class Notifier;
	class SettingsProvider;
	class SoundPlayer;
	class StoragesFactory;
	class SystemTray;
	class UIEventStream;
	class UIFactory;
	class URIHandler;

	class AccountsManager {
		public:
			AccountsManager(EventLoop* eventLoop, NetworkFactories* networkFactories, UIFactory* uiFactories, SettingsProvider* settings, SystemTray* systemTray, SoundPlayer* soundPlayer, StoragesFactory* storagesFactory, CertificateStorageFactory* certificateStorageFactory, Dock* dock, Notifier* notifier, URIHandler* uriHandler, IdleDetector* idleDetector, const std::map<std::string, std::string> emoticons, bool useDelayForLatency);
			~AccountsManager();

		private:
			std::vector<MainController*> mainControllers_;
			LoginWindow* loginWindow_;

			UIEventStream* uiEventStream_;
			SettingsProvider* settings_;

			EventLoop* eventLoop_;
			NetworkFactories* networkFactories_;
			UIFactory* uiFactory_;
			SoundPlayer* soundPlayer_;
			StoragesFactory* storagesFactory_;
			CertificateStorageFactory* certificateStorageFactory_;
			Dock* dock_;
			URIHandler* uriHandler_;
			IdleDetector* idleDetector_;
			const std::map<std::string, std::string> emoticons_;
			bool useDelayForLatency_;
	};
}

