/*
 * Copyright (c) 2010-2015 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#include <Swift/QtUI/QtSwift.h>

#include <string>
#include <map>

#include <boost/bind.hpp>

#include <QFile>
#include <QMessageBox>
#include <QApplication>
#include <QMap>
#include <qdebug.h>

#include <Swiften/Base/Log.h>
#include <Swiften/Base/Path.h>
#include <Swiften/Base/Paths.h>
#include <Swiften/Base/Platform.h>
#include <Swiften/Base/String.h>
#include <Swiften/StringCodecs/Base64.h>
#include <Swiften/Elements/Presence.h>
#include <Swiften/Client/Client.h>
#include <Swiften/Base/Paths.h>
#include <Swiften/TLS/TLSContextFactory.h>

#include <SwifTools/Application/PlatformApplicationPathProvider.h>
#include <SwifTools/AutoUpdater/AutoUpdater.h>
#include <SwifTools/AutoUpdater/PlatformAutoUpdaterFactory.h>
#include <SwifTools/Notifier/TogglableNotifier.h>

#include <Swift/Controllers/Storages/CertificateFileStorageFactory.h>
#include <Swift/Controllers/Storages/FileStoragesFactory.h>
#include <Swift/Controllers/Settings/XMLSettingsProvider.h>
#include <Swift/Controllers/Settings/SettingsProviderHierachy.h>
#include <Swift/Controllers/XMPPEvents/EventController.h>
#include <Swift/Controllers/SettingConstants.h>
#include <Swift/Controllers/MainController.h>
#include <Swift/Controllers/ApplicationInfo.h>
#include <Swift/Controllers/BuildVersion.h>
#include <Swift/Controllers/StatusCache.h>

#include <Swift/QtUI/QtLoginWindow.h>
#include <Swift/QtUI/QtChatTabsBase.h>
#include <Swift/QtUI/QtChatTabs.h>
#include <Swift/QtUI/QtChatTabsShortcutOnlySubstitute.h>
#include <Swift/QtUI/QtSystemTray.h>
#include <Swift/QtUI/QtSoundPlayer.h>
#include <Swift/QtUI/QtSwiftUtil.h>
#include <Swift/QtUI/QtUIFactory.h>
#include <Swift/QtUI/QtChatWindowFactory.h>
#include <Swift/QtUI/QtSingleWindow.h>

#if defined(SWIFTEN_PLATFORM_WINDOWS)
#include <Swift/QtUI/WindowsNotifier.h>
#elif defined(HAVE_GROWL)
#include <SwifTools/Notifier/GrowlNotifier.h>
#elif defined(SWIFTEN_PLATFORM_LINUX)
#include <Swift/QtUI/FreeDesktopNotifier.h>
#else
#include <SwifTools/Notifier/NullNotifier.h>
#endif

#if defined(SWIFTEN_PLATFORM_MACOSX)
#include <SwifTools/Dock/MacOSXDock.h>
#else
#include <SwifTools/Dock/NullDock.h>
#endif

#if defined(SWIFTEN_PLATFORM_MACOSX)
#include <Swift/QtUI/QtURIHandler.h>
#elif defined(SWIFTEN_PLATFORM_WIN32)
#include <SwifTools/URIHandler/NullURIHandler.h>
#else
#include <Swift/QtUI/QtDBUSURIHandler.h>
#endif

namespace Swift{

#if defined(SWIFTEN_PLATFORM_MACOSX)
//#define SWIFT_APPCAST_URL "http://swift.im/appcast/swift-mac-dev.xml"
#else 
//#define SWIFT_APPCAST_URL ""
#endif

po::options_description QtSwift::getOptionsDescription() {
	po::options_description result("Options");
	result.add_options()
		("debug", "Turn on debug logging")
		("help", "Show this help message")
		("version", "Show version information")
		("netbook-mode", "Use netbook mode display (unsupported)")
		("no-tabs", "Don't manage chat windows in tabs (unsupported)")
		("latency-debug", "Use latency debugging (unsupported)")
		("multi-account", po::value<int>()->default_value(1), "Number of accounts to open windows for (unsupported)")
		("start-minimized", "Don't show the login/roster window at startup")
		("enable-jid-adhocs", "Enable AdHoc commands to custom JID's.")
		("trellis", "Enable support for trellis layout")
#if QT_VERSION >= 0x040800
		("language", po::value<std::string>(), "Use a specific language, instead of the system-wide one")
#endif
		;
	return result;
}

XMLSettingsProvider* QtSwift::loadSettingsFile(const QString& fileName) {
	QFile configFile(fileName);
	if (configFile.exists() && configFile.open(QIODevice::ReadOnly)) {
		QString xmlString;
		while (!configFile.atEnd()) {
			QByteArray line = configFile.readLine();
			xmlString += line + "\n";
		}
		return new XMLSettingsProvider(Q2PSTRING(xmlString));
	}
	return new XMLSettingsProvider("");
}

void QtSwift::loadEmoticonsFile(const QString& fileName, std::map<std::string, std::string>& emoticons)  {
	QFile file(fileName);
	if (file.exists() && file.open(QIODevice::ReadOnly)) {
		while (!file.atEnd()) {
			QString line = file.readLine();
			line.replace("\n", "");
			line.replace("\r", "");
			QStringList tokens = line.split(" ");
			if (tokens.size() == 2) {
				QString emoticonFile = tokens[1];
				if (!emoticonFile.startsWith(":/") && !emoticonFile.startsWith("qrc:/")) {
					emoticonFile = "file://" + emoticonFile;
				}
				emoticons[Q2PSTRING(tokens[0])] = Q2PSTRING(emoticonFile);
			}
		}
	}
}

QtSwift::QtSwift(const po::variables_map& options) : networkFactories_(&clientMainThreadCaller_), autoUpdater_(NULL), idleDetector_(&idleQuerier_, networkFactories_.getTimerFactory(), 1000) {
	QCoreApplication::setApplicationName(SWIFT_APPLICATION_NAME);
	QCoreApplication::setOrganizationName(SWIFT_ORGANIZATION_NAME);
	QCoreApplication::setOrganizationDomain(SWIFT_ORGANIZATION_DOMAIN);
	QCoreApplication::setApplicationVersion(buildVersion);

	qtSettings_ = new QtSettingsProvider();
	xmlSettings_ = loadSettingsFile(P2QSTRING(pathToString(Paths::getExecutablePath() / "system-settings.xml")));
	settingsHierachy_ = new SettingsProviderHierachy();
	settingsHierachy_->addProviderToTopOfStack(xmlSettings_);
	settingsHierachy_->addProviderToTopOfStack(qtSettings_);

	networkFactories_.getTLSContextFactory()->setDisconnectOnCardRemoval(settingsHierachy_->getSetting(SettingConstants::DISCONNECT_ON_CARD_REMOVAL));

	std::map<std::string, std::string> emoticons;
	loadEmoticonsFile(":/emoticons/emoticons.txt", emoticons);
	loadEmoticonsFile(P2QSTRING(pathToString(Paths::getExecutablePath() / "emoticons.txt")), emoticons);

	if (options.count("netbook-mode")) {
		splitter_ = new QtSingleWindow(qtSettings_);
	} else {
		splitter_ = NULL;
	}

	int numberOfAccounts = 1;
	try {
		numberOfAccounts = options["multi-account"].as<int>();
	} catch (...) {
		/* This seems to fail on a Mac when the .app is launched directly (the usual path).*/
		numberOfAccounts = 1;
	}

	if (options.count("debug")) {
		Log::setLogLevel(Swift::Log::debug);
	}

	bool enableAdHocCommandOnJID = options.count("enable-jid-adhocs") > 0;
	tabs_ = NULL;
	if (options.count("no-tabs") && !splitter_) {
		tabs_ = new QtChatTabsShortcutOnlySubstitute();
	}
	else {
		tabs_ = new QtChatTabs(splitter_ != NULL, settingsHierachy_, options.count("trellis"));
	}
	bool startMinimized = options.count("start-minimized") > 0;
	applicationPathProvider_ = new PlatformApplicationPathProvider(SWIFT_APPLICATION_NAME);
	storagesFactory_ = new FileStoragesFactory(applicationPathProvider_->getDataDir(), networkFactories_.getCryptoProvider());
	certificateStorageFactory_ = new CertificateFileStorageFactory(applicationPathProvider_->getDataDir(), tlsFactories_.getCertificateFactory(), networkFactories_.getCryptoProvider());
	chatWindowFactory_ = new QtChatWindowFactory(splitter_, settingsHierachy_, qtSettings_, tabs_, "", emoticons);
	soundPlayer_ = new QtSoundPlayer(applicationPathProvider_);

	// Ugly, because the dock depends on the tray, but the temporary
	// multi-account hack creates one tray per account.
	QtSystemTray* systemTray = new QtSystemTray();
	systemTrays_.push_back(systemTray);

#if defined(HAVE_GROWL)
	notifier_ = new GrowlNotifier(SWIFT_APPLICATION_NAME);
#elif defined(SWIFTEN_PLATFORM_WINDOWS)
	notifier_ = new WindowsNotifier(SWIFT_APPLICATION_NAME, applicationPathProvider_->getResourcePath("/images/logo-icon-32.png"), systemTray->getQSystemTrayIcon());
#elif defined(SWIFTEN_PLATFORM_LINUX)
	notifier_ = new FreeDesktopNotifier(SWIFT_APPLICATION_NAME);
#else
	notifier_ = new NullNotifier();
#endif

#if defined(SWIFTEN_PLATFORM_MACOSX)
	dock_ = new MacOSXDock(&cocoaApplication_);
#else
	dock_ = new NullDock();
#endif

#if defined(SWIFTEN_PLATFORM_MACOSX)
	uriHandler_ = new QtURIHandler();
#elif defined(SWIFTEN_PLATFORM_WIN32)
	uriHandler_ = new NullURIHandler();
#else
	uriHandler_ = new QtDBUSURIHandler();
#endif

	statusCache_ = new StatusCache(applicationPathProvider_);

	if (splitter_) {
		splitter_->show();
	}

	// Putting extracted things from MainController here:

	uiEventStream_ = new UIEventStream();
	QtUIFactory* uiFactory = new QtUIFactory(
				settingsHierachy_,
				qtSettings_,
				tabs_,
				splitter_,
				systemTrays_[0], // replacing i
				chatWindowFactory_,
				networkFactories_.getTimerFactory(),
				statusCache_,
				startMinimized,
				!emoticons.empty(),
				enableAdHocCommandOnJID);
	LoginWindow* loginWindow = uiFactory->createLoginWindow(uiEventStream_);

	togglableNotifier_ = new TogglableNotifier(notifier_);
	togglableNotifier_->setPersistentEnabled(settingsHierachy_->getSetting(SettingConstants::SHOW_NOTIFICATIONS));

	eventController_ = new EventController();

	uiFactories_.push_back(uiFactory);
	MainController* mainController = new MainController(
				&clientMainThreadCaller_,
				uiEventStream_,
				eventController_,
				&networkFactories_,
				uiFactory,
				loginWindow,
				settingsHierachy_,
				systemTrays_[0], // replacing i
				soundPlayer_,
				storagesFactory_,
				certificateStorageFactory_,
				dock_,
				notifier_,
				togglableNotifier_,
				uriHandler_,
				&idleDetector_,
				emoticons,
				options.count("latency-debug") > 0);
	mainControllers_.push_back(mainController);


	std::string selectedLoginJID = settingsHierachy_->getSetting(SettingConstants::LAST_LOGIN_JID);
	bool loginAutomatically = settingsHierachy_->getSetting(SettingConstants::LOGIN_AUTOMATICALLY);
	std::string cachedPassword;
	std::string cachedCertificate;
	ClientOptions cachedOptions;
	bool eagle = settingsHierachy_->getSetting(SettingConstants::FORGET_PASSWORDS);
	if (!eagle) {
		foreach (std::string profile, settingsHierachy_->getAvailableProfiles()) {
			ProfileSettingsProvider profileSettings(profile, settingsHierachy_);
			std::string password = profileSettings.getStringSetting("pass");
			std::string certificate = profileSettings.getStringSetting("certificate");
			std::string jid = profileSettings.getStringSetting("jid");
			ClientOptions clientOptions = parseClientOptions(profileSettings.getStringSetting("options"));
			loginWindow->addAvailableAccount(jid, password, certificate, clientOptions);
			if (jid == selectedLoginJID) {
				cachedPassword = password;
				cachedCertificate = certificate;
				cachedOptions = clientOptions;
			}
		}
		loginWindow->selectUser(selectedLoginJID);
		loginWindow->setLoginAutomatically(loginAutomatically);
	}

	if (loginAutomatically) {
		mainController->profileSettings_ = new ProfileSettingsProvider(selectedLoginJID, settingsHierachy_);
		/* FIXME: deal with autologin with a cert*/
		mainController->handleLoginRequest(selectedLoginJID, cachedPassword, cachedCertificate, CertificateWithKey::ref(), cachedOptions, true, true);
	} else {
		mainController->profileSettings_ = NULL;
	}


	/*for (int i = 0; i < numberOfAccounts; i++) {
		if (i > 0) {
			// Don't add the first tray (see note above)
			systemTrays_.push_back(new QtSystemTray());
		}
		QtUIFactory* uiFactory = new QtUIFactory(settingsHierachy_, qtSettings_, tabs_, splitter_, systemTrays_[i], chatWindowFactory_, networkFactories_.getTimerFactory(), statusCache_, startMinimized, !emoticons.empty(), enableAdHocCommandOnJID);
		uiFactories_.push_back(uiFactory);
		MainController* mainController = new MainController(
				&clientMainThreadCaller_,
				&networkFactories_,
				uiFactory,
				settingsHierachy_,
				systemTrays_[i],
				soundPlayer_,
				storagesFactory_,
				certificateStorageFactory_,
				dock_,
				notifier_,
				uriHandler_,
				&idleDetector_,
				emoticons,
				options.count("latency-debug") > 0);
		mainControllers_.push_back(mainController);
	}*/


	// PlatformAutoUpdaterFactory autoUpdaterFactory;
	// if (autoUpdaterFactory.isSupported()) {
	// 	autoUpdater_ = autoUpdaterFactory.createAutoUpdater(SWIFT_APPCAST_URL);
	// 	autoUpdater_->checkForUpdates();
	// }
}

QtSwift::~QtSwift() {
	delete autoUpdater_;
	foreach (QtUIFactory* factory, uiFactories_) {
		delete factory;
	}
	foreach (MainController* controller, mainControllers_) {
		delete controller;
	}
	delete notifier_;
	delete togglableNotifier_;
	foreach (QtSystemTray* tray, systemTrays_) {
		delete tray;
	}
	delete tabs_;
	delete splitter_;
	delete settingsHierachy_;
	delete qtSettings_;
	delete xmlSettings_;
	delete statusCache_;
	delete uriHandler_;
	delete dock_;
	delete soundPlayer_;
	delete chatWindowFactory_;
	delete certificateStorageFactory_;
	delete storagesFactory_;
	delete applicationPathProvider_;
	delete uiEventStream_;

	eventController_->disconnectAll();
	delete eventController_;
}

#define CHECK_PARSE_LENGTH if (i >= segments.size()) {return result;}
#define PARSE_INT_RAW(defaultValue) CHECK_PARSE_LENGTH intVal = defaultValue; try {intVal = boost::lexical_cast<int>(segments[i]);} catch(const boost::bad_lexical_cast&) {};i++;
#define PARSE_STRING_RAW CHECK_PARSE_LENGTH stringVal = byteArrayToString(Base64::decode(segments[i]));i++;

#define PARSE_BOOL(option, defaultValue) PARSE_INT_RAW(defaultValue); result.option = (intVal == 1);
#define PARSE_INT(option, defaultValue) PARSE_INT_RAW(defaultValue); result.option = intVal;
#define PARSE_STRING(option) PARSE_STRING_RAW; result.option = stringVal;
#define PARSE_SAFE_STRING(option) PARSE_STRING_RAW; result.option = SafeString(createSafeByteArray(stringVal));
#define PARSE_URL(option) {PARSE_STRING_RAW; result.option = URL::fromString(stringVal);}


ClientOptions QtSwift::parseClientOptions(const std::string& optionString) {
	ClientOptions result;
	size_t i = 0;
	int intVal = 0;
	std::string stringVal;
	std::vector<std::string> segments = String::split(optionString, ',');

	PARSE_BOOL(useStreamCompression, 1);
	PARSE_INT_RAW(-1);
	switch (intVal) {
		case 1: result.useTLS = ClientOptions::NeverUseTLS;break;
		case 2: result.useTLS = ClientOptions::UseTLSWhenAvailable;break;
		case 3: result.useTLS = ClientOptions::RequireTLS;break;
		default:;
	}
	PARSE_BOOL(allowPLAINWithoutTLS, 0);
	PARSE_BOOL(useStreamResumption, 0);
	PARSE_BOOL(useAcks, 1);
	PARSE_STRING(manualHostname);
	PARSE_INT(manualPort, -1);
	PARSE_INT_RAW(-1);
	switch (intVal) {
		case 1: result.proxyType = ClientOptions::NoProxy;break;
		case 2: result.proxyType = ClientOptions::SystemConfiguredProxy;break;
		case 3: result.proxyType = ClientOptions::SOCKS5Proxy;break;
		case 4: result.proxyType = ClientOptions::HTTPConnectProxy;break;
	}
	PARSE_STRING(manualProxyHostname);
	PARSE_INT(manualProxyPort, -1);
	PARSE_URL(boshURL);
	PARSE_URL(boshHTTPConnectProxyURL);
	PARSE_SAFE_STRING(boshHTTPConnectProxyAuthID);
	PARSE_SAFE_STRING(boshHTTPConnectProxyAuthPassword);

	return result;
}

}
