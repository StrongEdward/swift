/*
 * Copyright (c) 2010-2013 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#pragma once

#include <vector>
#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include <Swiften/Base/boost_bsignals.h>
#include <Swiften/Network/Timer.h>
#include <Swiften/Client/ClientError.h>
#include <Swiften/JID/JID.h>
#include <Swiften/Elements/DiscoInfo.h>
#include <Swiften/Elements/VCard.h>
#include <Swiften/Elements/ErrorPayload.h>
#include <Swiften/Elements/Presence.h>
#include <Swiften/Elements/CapsInfo.h>
#include <Swiften/Client/ClientXMLTracer.h>

#include <Swift/Controllers/Settings/SettingsProvider.h>
#include <Swift/Controllers/ProfileSettingsProvider.h>
#include <Swift/Controllers/XMPPEvents/ErrorEvent.h>
#include <Swift/Controllers/UIEvents/UIEvent.h>


namespace Swift {
	class Account;
	class AccountsManager;
	class IdleDetector;
	class UIFactory;
	class EventLoop;
	class Client;
	class ChatController;
	class ChatsManager;
	class CertificateStorageFactory;
	class CertificateStorage;
	class CertificateStorageTrustChecker;
	class EventController;
	class MainWindow;
	class RosterController;
	class LoginWindow;
	class EventLoop;
	class MUCController;
	class Notifier;
	class ProfileController;
	class ShowProfileController;
	class ContactEditController;
	class TogglableNotifier;
	class PresenceNotifier;
	class EventNotifier;
	//class SystemTray;
	class SystemTrayController;
	class SoundEventController;
	class SoundPlayer;
	class XMLConsoleController;
	class HistoryViewController;
	class HistoryController;
	class FileTransferListController;
	class UIEventStream;
	class EventWindowFactory;
	class EventWindowController;
	class MUCSearchController;
	class UserSearchController;
	class StatusTracker;
	class Dock;
	class Storages;
	class StoragesFactory;
	class NetworkFactories;
	class URIHandler;
	class XMPPURIController;
	class AdHocManager;
	class AdHocCommandWindowFactory;
	class FileTransferOverview;
	class WhiteboardManager;
	class HighlightManager;
	class HighlightEditorController;
	class BlockListController;
	class ContactSuggester;
	class ContactsFromXMPPRoster;

	class MainController {

		public:
			MainController(boost::shared_ptr<Account> account,
						   EventLoop* eventLoop,
						   UIEventStream* uiEventStream,
						   EventController* eventController,
						   NetworkFactories* networkFactories,
						   UIFactory* uiFactories,
						   LoginWindow* loginWindow,
						   SettingsProvider* settings,
						   SystemTrayController* systemTrayController,
						   SoundPlayer* soundPlayer,
						   StoragesFactory* storagesFactory,
						   CertificateStorageFactory* certificateStorageFactory,
						   Dock* dock,
						   TogglableNotifier* togglableNotifier,
						   URIHandler* uriHandler,
						   IdleDetector* idleDetector,
						   const std::map<std::string, std::string>& emoticons,
						   bool useDelayForLatency,
						   bool createdInCombobox);
			~MainController();

			const std::string getJIDString();
			boost::shared_ptr<Account> getAccount();
			bool shouldBeDeleted();
			boost::signal<void(const std::string) > onShouldBeDeleted;

		private:
			void resetClient();
			void handleConnected();
			void handleLoginRequest();
			void handleCancelLoginRequest();
			void handleQuitRequest();
			void handleChangeStatusRequest(StatusShow::Type show, const std::string &statusText);
			void handleDisconnected(const boost::optional<ClientError>& error);
			void handleServerDiscoInfoResponse(boost::shared_ptr<DiscoInfo>, ErrorPayload::ref);
			void handleEventQueueLengthChange(int count);
			void handleVCardReceived(const JID& j, VCard::ref vCard);
			void handleSettingChanged(const std::string& settingPath);
			//void handlePurgeSavedLoginRequest(const std::string& username);
			void sendPresence(boost::shared_ptr<Presence> presence);
			void handleInputIdleChanged(bool);
			void handleShowCertificateRequest();
			void logout();
			void signOut();
			void setReconnectTimer();
			void resetPendingReconnects();
			void resetCurrentError();
			//std::string serializeClientOptions(const ClientOptions& options); // to be moved to AccountsManager
			//void handleCacheCredentials(const std::string& password, CertificateWithKey::ref certificate, const ClientOptions& options);
			void performLoginFromCachedCredentials();
			void reconnectAfterError();
			void setManagersOffline();
			void handleNotificationClicked(const JID& jid);
			void handleForceQuit();
			void purgeCachedCredentials();

		private:
			boost::shared_ptr<Account> account_;
			boost::shared_ptr<AccountsManager> accountsManager_;
			bool createdInCombobox_;
			bool beforeFirstLogin_;
			bool firstLoginFailed_;

			// Probably have to remove them then
			//JID jid_;
			JID boundJID_;

			// Cached credentials
			std::string cachedPassword_;
			CertificateWithKey::ref certificate_;
			ClientOptions clientOptions_;

			ProfileSettingsProvider* profileSettings_;

			EventLoop* eventLoop_;
			UIEventStream* uiEventStream_;
			EventController* eventController_;
			NetworkFactories* networkFactories_;
			UIFactory* uiFactory_;
			StoragesFactory* storagesFactory_;
			Storages* storages_;
			CertificateStorageFactory* certificateStorageFactory_;
			CertificateStorage* certificateStorage_;
			CertificateStorageTrustChecker* certificateTrustChecker_;
			bool clientInitialized_;
			boost::shared_ptr<Client> client_;
			SettingsProvider *settings_;

			Dock* dock_;
			URIHandler* uriHandler_;
			IdleDetector* idleDetector_;
			TogglableNotifier* togglableNotifier_;
			PresenceNotifier* presenceNotifier_;
			EventNotifier* eventNotifier_;
			RosterController* rosterController_;
			EventWindowController* eventWindowController_;
			AdHocManager* adHocManager_;
			LoginWindow* loginWindow_;
			XMLConsoleController* xmlConsoleController_;
			HistoryViewController* historyViewController_;
			HistoryController* historyController_;
			FileTransferListController* fileTransferListController_;
			BlockListController* blockListController_;
			ChatsManager* chatsManager_;
			ProfileController* profileController_;
			ShowProfileController* showProfileController_;
			ContactEditController* contactEditController_;
			ContactsFromXMPPRoster* contactsFromRosterProvider_;
			ContactSuggester* contactSuggesterWithoutRoster_;
			ContactSuggester* contactSuggesterWithRoster_;
			SystemTrayController* systemTrayController_;
			SoundEventController* soundEventController_;
			XMPPURIController* xmppURIController_;
			std::string vCardPhotoHash_;

			boost::shared_ptr<ErrorEvent> lastDisconnectError_;
			bool useDelayForLatency_;
			UserSearchController* userSearchControllerChat_;
			UserSearchController* userSearchControllerAdd_;
			UserSearchController* userSearchControllerInvite_;
			int timeBeforeNextReconnect_;
			Timer::ref reconnectTimer_;
			StatusTracker* statusTracker_;
			bool myStatusLooksOnline_;
			bool quitRequested_;
			bool offlineRequested_;
			static const int SecondsToWaitBeforeForceQuitting;
			FileTransferOverview* ftOverview_;
			WhiteboardManager* whiteboardManager_;
			HighlightManager* highlightManager_;
			HighlightEditorController* highlightEditorController_;
			std::map<std::string, std::string> emoticons_;
	};
}
