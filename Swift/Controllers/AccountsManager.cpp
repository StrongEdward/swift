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

#include <Swift/Controllers/AccountsManager.h>

#include <vector>

#include <Swiften/Network/NetworkFactories.h>

#include <SwifTools/Dock/Dock.h>
#include <SwifTools/Idle/IdleDetector.h>
#include <SwifTools/Notifier/Notifier.h>

#include <Swift/Controllers/MainController.h>
#include <Swift/Controllers/Settings/SettingsProvider.h>
#include <Swift/Controllers/SoundPlayer.h>
#include <Swift/Controllers/Storages/StoragesFactory.h>
#include <Swift/Controllers/SystemTray.h>
#include <Swift/Controllers/UIEvents/UIEventStream.h>
#include <Swift/Controllers/UIInterfaces/LoginWindow.h>
#include <Swift/Controllers/UIInterfaces/UIFactory.h>


namespace Swift {

AccountsManager::AccountsManager(EventLoop* eventLoop, NetworkFactories* networkFactories, UIFactory* uiFactory, SettingsProvider* settings, SystemTray* systemTray, SoundPlayer* soundPlayer, StoragesFactory* storagesFactory, CertificateStorageFactory* certificateStorageFactory, Dock* dock, Notifier* notifier, URIHandler* uriHandler, IdleDetector* idleDetector, const std::map<std::string, std::string> emoticons, bool useDelayForLatency) :
	settings_(settings),
	eventLoop_(eventLoop),
	networkFactories_(networkFactories),
	uiFactory_(uiFactory),
	soundPlayer_(soundPlayer),
	storagesFactory_(storagesFactory),
	certificateStorageFactory_(certificateStorageFactory),
	dock_(dock),
	uriHandler_(uriHandler),
	idleDetector_(idleDetector),
	emoticons_(emoticons),
	useDelayForLatency_(useDelayForLatency) {

	uiEventStream_ = new UIEventStream();

	loginWindow_ = uiFactory_->createLoginWindow(uiEventStream_);
	loginWindow_->setShowNotificationToggle(!notifier->isExternallyConfigured());

	MainController* mainController = new MainController(eventLoop_, uiEventStream_, networkFactories_, uiFactory_, loginWindow_, settings_, systemTray, soundPlayer_, storagesFactory_, certificateStorageFactory_, dock_, notifier, uriHandler_, idleDetector_, emoticons_, useDelayForLatency_);
	mainControllers_.push_back(mainController);
}

AccountsManager::~AccountsManager() {
	foreach (MainController* controller, mainControllers_) {
		delete controller;
	}

	delete uiEventStream_;
}

} // namespace Swift
