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

#include <boost/shared_ptr.hpp>

namespace Swift {
	class Account;
	class CertificateStorageFactory;
	class ClientOptions;
	class Dock;
	class EventController;
	class EventLoop;
	class HighlightEditorController;
	class HighlightManager;
	class IdleDetector;
	class JID;
	class LoginWindow;
	class MainController;
	class NetworkFactories;
	class Notifier;
	class SettingsProvider;
	class SoundPlayer;
	class StoragesFactory;
	class SystemTray;
	class SystemTrayController;
	class TogglableNotifier;
	class UIEventStream;
	class UIFactory;
	class URIHandler;
	class XMLConsoleController;

	class AccountsManager {
		public:
			AccountsManager(EventLoop* eventLoop, NetworkFactories* networkFactories, UIFactory* uiFactories, SettingsProvider* settings, SystemTray* systemTray, SoundPlayer* soundPlayer, StoragesFactory* storagesFactory, CertificateStorageFactory* certificateStorageFactory, Dock* dock, Notifier* notifier, URIHandler* uriHandler, IdleDetector* idleDetector, const std::map<std::string, std::string> emoticons, bool useDelayForLatency);
			~AccountsManager();

			JID getDefaultJID();
			boost::shared_ptr<Account> getDefaultAccount();
			boost::shared_ptr<Account> getAccountByJIDString(const std::string& jid); // Assumption: JID is unique
			MainController* getMainControllerByJIDString(const std::string& jid);
			boost::shared_ptr<Account> getAccountAt(unsigned int index);
			int accountsCount();
			void addAccount(boost::shared_ptr<Account> account = boost::shared_ptr<Account>());
			void removeAccount(const std::string& username);

		private:
			void loadAccounts();
			void storeAccounts();
			void createMainController(boost::shared_ptr<Account> account, bool triggeredByCombobox);
			void setDefaultAccount(boost::shared_ptr<Account> account);
			void clearAutoLogins();
			int getMaxAccountIndex();
			std::string serializeClientOptions(const ClientOptions& options);
			ClientOptions parseClientOptions(const std::string& optionString);

			void handleDefaultAccountChanged(int index);
			void handleAccountDataChanged();
			void handleLoginRequestTriggeredByCombobox(const std::string &username, const std::string &password, const std::string& certificatePath, const ClientOptions& options, bool remember, bool loginAutomatically);
			void handleCancelLoginRequest(const std::string currentUsername);
			void handleMainControllerConnected(const MainController* controller);
			void handlePurgeSavedLoginRequest(const std::string& username);
			void handleQuitRequest();

			static bool compareAccounts (MainController* a, MainController* b);


		private:
			std::vector<MainController*> mainControllers_;
			LoginWindow* loginWindow_;
			boost::shared_ptr<Account> defaultAccount_;
			int maxAccountIndex_;

			UIEventStream* uiEventStream_;
			SettingsProvider* settings_;

			EventLoop* eventLoop_;
			EventController* eventController_;
			NetworkFactories* networkFactories_;
			UIFactory* uiFactory_;
			SystemTrayController* systemTrayController_;
			SoundPlayer* soundPlayer_;
			StoragesFactory* storagesFactory_;
			CertificateStorageFactory* certificateStorageFactory_;
			Dock* dock_;
			TogglableNotifier* togglableNotifier_;
			URIHandler* uriHandler_;
			IdleDetector* idleDetector_;
			const std::map<std::string, std::string> emoticons_;
			bool useDelayForLatency_;
			HighlightManager* highlightManager_;
			HighlightEditorController* highlightEditorController_;
			XMLConsoleController* xmlConsoleController_;
	};
}

