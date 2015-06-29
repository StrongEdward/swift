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

#include <3rdParty/Boost/src/boost/shared_ptr.hpp>
#include <3rdParty/Boost/src/boost/enable_shared_from_this.hpp>

#include <Swiften/TLS/CertificateWithKey.h>

#include <Swift/Controllers/UIInterfaces/AccountsList.h>
#include <Swift/Controllers/Account.h>

//#include <Swift/Controllers/Storages/CertificateStorageFactory.h>
//#include <Swift/Controllers/Storages/CertificateStorage.h>


namespace Swift {
	class Account;
	class SettingsProviderHierachy;
	class SettingsProvider;
	class ProfileSettingsProvider;
	class LoginWindow;
	class AccountsList;

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
	//class CertificateStorage;
	//class CertificateWithKey;
	class Dock;
	class Notifier;
	class TogglableNotifier;
	class URIHandler;
	class IdleDetector;


	class AccountsManager : public boost::enable_shared_from_this<AccountsManager> {
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
							const std::map<std::string, std::string> emoticons,
							bool useDelayForLatency);
			~AccountsManager();

			void createMainController(boost::shared_ptr<Account> account, bool triggeredByCombobox);

			JID getDefaultJID();
			boost::shared_ptr<Account> getDefaultAccount();
			boost::shared_ptr<Account> getAccountByJIDString(std::string jid); // assumption: jid is unique
			MainController* getMainControllerByJIDString(const std::string& jid);
			boost::shared_ptr<Account> getAccountAt(unsigned int index);
			int accountsCount();
			void addAccount(boost::shared_ptr<Account> account = boost::shared_ptr<Account>());
			void removeAccount(const std::string& username);

			void handleLoginRequestTriggeredByCombobox(const std::string &username, const std::string &password, const std::string& certificatePath, const ClientOptions& options, bool remember, bool loginAutomatically);

			//boost::signal<void (const MainController*, const std::string&, CertificateWithKey::ref, const ClientOptions&) > onLoginRequest;

		private:
			static bool compareAccounts (MainController* a, MainController* b);
			int maxAccountIndex();
			void handleDefaultAccountChanged(int index);
			void handlePurgeSavedLoginRequest(const std::string& username); // to be removed, remove account directly
			void setDefaultAccount(boost::shared_ptr<Account> account);

		private:
			//std::vector< boost::shared_ptr<Account> > accounts_;
			boost::shared_ptr<Account> defaultAccount_;
			std::vector<MainController*> mainControllers_;
			LoginWindow* loginWindow_;
			//AccountsList* accountsList_; // avoid it

			UIEventStream* uiEventStream_;
			SettingsProvider* settings_;

			std::string serializeClientOptions(const ClientOptions& options);
			ClientOptions parseClientOptions(const std::string& optionString);
			void clearAutoLogins();

			// MainController parameters
			EventLoop* eventLoop_;
			//UIEventStream* uiEventStream_;
			EventController* eventController_;
			NetworkFactories* networkFactories_;
			UIFactory* uiFactory_;
			//SettingsProvider* settings_;
			SystemTrayController* systemTrayController_;
			SoundPlayer* soundPlayer_;
			StoragesFactory* storagesFactory_;
			CertificateStorageFactory* certificateStorageFactory_;
			Dock* dock_;
			//Notifier* notifier_;
			TogglableNotifier* togglableNotifier_;
			URIHandler* uriHandler_;
			IdleDetector* idleDetector_;
			const std::map<std::string, std::string> emoticons_;
			bool useDelayForLatency_;

			int maxAccountIndex_;

	};
}

