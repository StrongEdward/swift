/*
 * Copyright (c) 2010-2015 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>

#include <Swiften/TLS/PlatformTLSFactories.h>
#include <Swiften/Network/BoostNetworkFactories.h>
#include <string>
#include "Swiften/Base/Platform.h"
#include "Swiften/EventLoop/Qt/QtEventLoop.h"
#include "QtSettingsProvider.h"
#if defined(SWIFTEN_PLATFORM_MACOSX)
#include "SwifTools/Application/CocoaApplication.h"
#include "CocoaApplicationActivateHelper.h"
#endif
#if defined(SWIFTEN_PLATFORM_WINDOWS)
#include "WindowsNotifier.h"
#endif
#include "SwifTools/Idle/PlatformIdleQuerier.h"
#include "SwifTools/Idle/ActualIdleDetector.h"


namespace po = boost::program_options;

class QSplitter;

namespace Swift {
	class AccountsManager;
	class ApplicationPathProvider;
	class AutoUpdater;
	class AvatarStorage;
	class CapsStorage;
	class CertificateStorageFactory;
	class Dock;
	class EventLoop;
	class Notifier;
	class QtChatTabsBase;
	class QtChatWindowFactory;
	class QtMUCSearchWindowFactory;
	class QtSingleWindow;
	class QtSoundPlayer;
	class QtSystemTray;
	class QtUIFactory;
	class QtUserSearchWindowFactory;
	class SettingsProviderHierachy;
	class StatusCache;
	class StoragesFactory;
	class URIHandler;
	class XMLSettingsProvider;

	class QtSwift : public QObject {
		Q_OBJECT
		public:
			QtSwift(const po::variables_map& options);
			static po::options_description getOptionsDescription();
			~QtSwift();
		private:
			XMLSettingsProvider* loadSettingsFile(const QString& fileName);
			void loadEmoticonsFile(const QString& fileName, std::map<std::string, std::string>& emoticons);

		private:
			QtEventLoop clientMainThreadCaller_;
			PlatformTLSFactories tlsFactories_;
			BoostNetworkFactories networkFactories_;
			QtChatWindowFactory* chatWindowFactory_;
			std::vector<AccountsManager*> accountsManagers_;
			std::vector<QtSystemTray*> systemTrays_;
			std::vector<QtUIFactory*> uiFactories_;
			QtSettingsProvider* qtSettings_;
			XMLSettingsProvider* xmlSettings_;
			SettingsProviderHierachy* settingsHierachy_;
			QtSingleWindow* splitter_;
			QtSoundPlayer* soundPlayer_;
			Dock* dock_;
			URIHandler* uriHandler_;
			QtChatTabsBase* tabs_;
			ApplicationPathProvider* applicationPathProvider_;
			StoragesFactory* storagesFactory_;
			CertificateStorageFactory* certificateStorageFactory_;
			AutoUpdater* autoUpdater_;
			Notifier* notifier_;
			StatusCache* statusCache_;
			PlatformIdleQuerier idleQuerier_;
			ActualIdleDetector idleDetector_;
#if defined(SWIFTEN_PLATFORM_MACOSX)
			CocoaApplication cocoaApplication_;
			CocoaApplicationActivateHelper cocoaApplicationActivateHelper_;
#endif
	};
}
