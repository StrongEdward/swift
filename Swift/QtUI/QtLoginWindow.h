/*
 * Copyright (c) 2010-2012 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QPointer>
#include <QPushButton>
#include <QStackedWidget>

#include <Swift/Controllers/UIEvents/UIEventStream.h>
#include <Swift/Controllers/UIInterfaces/AccountsList.h>
#include <Swift/Controllers/UIInterfaces/LoginWindow.h>
#include <Swift/Controllers/UIInterfaces/MainWindow.h>

class QLabel;
class QToolButton;
class QComboBox;

namespace Swift {
	class QtAboutWidget;
	class SettingsProvider;
	class TimerFactory;
	class AccountsList;

	class QtLoginWindow : public QMainWindow, public LoginWindow {
		Q_OBJECT
		public:
			struct QtMenus {
				QtMenus(QMenu* swiftMenu, QMenu* generalMenu) : swiftMenu(swiftMenu), generalMenu(generalMenu) {}
				QMenu* swiftMenu;
				QMenu* generalMenu;
			};

		public:
			QtLoginWindow(UIEventStream* uiEventStream, SettingsProvider* settings, TimerFactory* timerFactory);

			virtual bool isInMultiaccountView();
			void morphInto(MainWindow *mainWindow);
			virtual void loggedOut();
			virtual void setShowNotificationToggle(bool);
			virtual void setMessage(const std::string& message);
			virtual void setAccountsManager(AccountsManager* manager);
			virtual void addAvailableAccount(boost::shared_ptr<Account> account);
			virtual void removeAvailableAccount(int index);
			virtual void clearPassword();
			virtual void setLoginAutomatically(bool loginAutomatically);
			virtual void setIsLoggingIn(bool loggingIn);
			void selectUser(const std::string& user);
			bool askUserToTrustCertificatePermanently(const std::string& message, const std::vector<Certificate::ref>& certificate);
			void hide();
			QtMenus getMenus() const;
			virtual void quit();

		signals:
			void geometryChanged();

		private slots:
			void loginClicked();
			void handleCertficateChecked(bool);
			void handleQuit();
			void handleShowXMLConsole();
			void handleShowFileTransferOverview();
			void handleShowHighlightEditor();
			void handleToggleSounds(bool enabled);
			void handleToggleNotifications(bool enabled);
			void handleAccountWantsToBeDefault(int index);
			void handleAbout();
			void bringToFront();
			void toggleBringToFront();
			void handleUsernameTextChanged();
			void resizeEvent(QResizeEvent* event);
			void moveEvent(QMoveEvent* event);
			void handleSettingChanged(const std::string& settingPath);
			void handleOpenConnectionOptions();
			void handleChangeView();
			void handleAddAccountClicked();

		protected:
			bool eventFilter(QObject *obj, QEvent *event);

		private:
			void setInitialMenus();
			void updateViewLabelText();
			QWidget* loginWidgetWrapper_;
			QWidget* singleAccountWrapper_;
			QWidget* accountsListWrapper_;
			QComboBox* username_;
			QLineEdit* password_;
			QPushButton* loginButton_;
			QPushButton* okButton_;
			/* If you add a widget here, change setLoggingIn as well.*/
			QCheckBox* remember_;
			QCheckBox* loginAutomatically_;
			QStackedWidget* stack_;
			QLabel* message_;
			QLabel* viewLabel_;
			QString certificateFile_;
			QToolButton* certificateButton_;
			QMenuBar* menuBar_;
			QMenu* swiftMenu_;
			QMenu* generalMenu_;
			QMenu* viewMenu_;
			QAction* toggleSoundsAction_;
			QAction* toggleNotificationsAction_;
			UIEventStream* uiEventStream_;
			QPointer<QtAboutWidget> aboutDialog_;
			SettingsProvider* settings_;
			QAction* xmlConsoleAction_;
			QAction* fileTransferOverviewAction_;
			QAction* highlightEditorAction_;
			TimerFactory* timerFactory_;
			ClientOptions currentOptions_;
			AccountsList* accountsList_;
			AccountsManager* accountsManager_;
	};
}
