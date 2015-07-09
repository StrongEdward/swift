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

#include <Swift/QtUI/QtLoginWindow.h>

#include <algorithm>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QComboBox>
#include <QCursor>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolButton>
 
#include <Swiften/Base/Platform.h>
#include <Swiften/Base/Paths.h>

#include <Swift/Controllers/UIEvents/RequestFileTransferListUIEvent.h>
#include <Swift/Controllers/UIEvents/RequestHighlightEditorUIEvent.h>
#include <Swift/Controllers/UIEvents/RequestXMLConsoleUIEvent.h>
#include <Swift/Controllers/UIEvents/UIEventStream.h>
#include <Swift/Controllers/SettingConstants.h>
#include <Swift/Controllers/Settings/SettingsProvider.h>
#include <Swift/QtUI/CAPICertificateSelector.h>
#include <Swift/QtUI/QtAboutWidget.h>
#include <Swift/QtUI/QtAccountsListWidget.h>
#include <Swift/QtUI/QtConnectionSettingsWindow.h>
#include <Swift/QtUI/QtMainWindow.h>
#include <Swift/QtUI/QtSwiftUtil.h>
#include <Swift/QtUI/QtUISettingConstants.h>
#include <Swift/QtUI/QtUtilities.h>


namespace Swift{

QtLoginWindow::QtLoginWindow(UIEventStream* uiEventStream, SettingsProvider* settings, TimerFactory* timerFactory) : QMainWindow(), settings_(settings), timerFactory_(timerFactory) {
	uiEventStream_ = uiEventStream;

	setWindowTitle("Swift");
#ifndef Q_OS_MAC
	setWindowIcon(QIcon(":/logo-icon-16.png"));
#endif
	QtUtilities::setX11Resource(this, "Main");
	setAccessibleName(tr("Swift Login Window"));
	//setAccessibleDescription(tr("This window is used for providing credentials to log into your XMPP service"));

	resize(200, 500);
	setContentsMargins(0,0,0,0);
	QWidget *centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);
	QBoxLayout *topLayout = new QBoxLayout(QBoxLayout::TopToBottom, centralWidget);
	stack_ = new QStackedWidget(centralWidget);
	topLayout->addWidget(stack_);
	topLayout->setMargin(0);
	loginWidgetWrapper_ = new QWidget(this);
	loginWidgetWrapper_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, loginWidgetWrapper_);
	//layout->addStretch(2);

	QLabel* logo = new QLabel(this);
	logo->setPixmap(QPixmap(":/logo-shaded-text.256.png"));
	logo->setScaledContents(true);
	logo->setFixedSize(192,192);

	QWidget *logoWidget = new QWidget(this);
	QHBoxLayout *logoLayout = new QHBoxLayout();
	logoLayout->setMargin(0);
	//logoLayout->addStretch(0);
	logoLayout->addWidget(logo);
	//logoLayout->addStretch(0);
	logoWidget->setLayout(logoLayout);
	layout->addWidget(logoWidget);

	// Changing views label, TODO: find better place for it maybe?
	viewLabel_ = new QLabel(this);
	viewLabel_->setTextFormat(Qt::RichText);
	viewLabel_->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	viewLabel_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
	layout->addWidget(viewLabel_);
	connect(viewLabel_, SIGNAL(linkActivated(const QString&)), SLOT(handleChangeView()));

	bool multiaccountEnabled = settings_->getSetting(SettingConstants::MULTIACCOUNT_ENABLED);
	if (!multiaccountEnabled) {
		viewLabel_->hide();
	}

	// Accounts list
	accountsListWrapper_ = new QWidget(this);
	accountsListWrapper_->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
	QBoxLayout *accountsListLayout = new QBoxLayout(QBoxLayout::TopToBottom, accountsListWrapper_);
	accountsListLayout->setContentsMargins(0,0,0,0);
	accountsListLayout->setSpacing(2);

	accountsList_ = new QtAccountsListWidget;

	accountsListLayout->addWidget(static_cast<QtAccountsListWidget*>(accountsList_));

	QWidget* underList = new QWidget(this);
	QLayout* underListLayout = new QHBoxLayout();
	underListLayout->setContentsMargins(4,4,0,4);
	underListLayout->setSpacing(2);

	QPushButton* addAccountButton = new QPushButton(QIcon(":/icons/add.ico"), "");
	addAccountButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	underListLayout->addWidget(addAccountButton);
	underListLayout->setAlignment(addAccountButton, Qt::AlignRight);
	connect(addAccountButton, SIGNAL(clicked()), this, SLOT(handleAddAccountClicked()));
	addAccountButton->setEnabled(false);

	underList->setLayout(underListLayout);
	accountsListLayout->addWidget(underList);

	//accountsListLayout->addStretch(2);
	//accountsListLayout->addStretch(2);

	okButton_ = new QPushButton(this);
	okButton_->setText(tr("OK"));
	okButton_->setAutoDefault(true);
	okButton_->setDefault(true);
	okButton_->setAccessibleName(tr("Done. Go to roster."));
	accountsListLayout->addWidget(okButton_);
	okButton_->hide(); // For now: when enabling/connecting to one account it automatically goes to roster so no need to show it. Then it will be useful when we have connected all accounts that we want and clicking it would get us to roster.

	layout->addWidget(accountsListWrapper_);
	accountsListWrapper_->hide();


	// Single account
	singleAccountWrapper_ = new QWidget(this);
	singleAccountWrapper_->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum));
	QBoxLayout *singleAccountLayout = new QBoxLayout(QBoxLayout::TopToBottom, singleAccountWrapper_);
	singleAccountLayout->setContentsMargins(0,0,0,0);
	//singleAccountLayout->setSpacing(2);

	QLabel* jidLabel = new QLabel(this);
	jidLabel->setText("<font size='-1'>" + tr("User address:") + "</font>");
	singleAccountLayout->addWidget(jidLabel);

	username_ = new QComboBox(this);
	username_->setMinimumWidth(215);
	username_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	username_->setEditable(true);
	username_->setWhatsThis(tr("User address - looks like someuser@someserver.com"));
	username_->setToolTip(tr("User address - looks like someuser@someserver.com"));
	username_->view()->installEventFilter(this);
	username_->setAccessibleName(tr("User address (of the form someuser@someserver.com)"));
	username_->setAccessibleDescription(tr("This is the user address that you'll be using to log in with"));
	singleAccountLayout->addWidget(username_);
	QLabel* jidHintLabel = new QLabel(this);
	jidHintLabel->setText("<font size='-1' color='grey' >" + tr("Example: alice@wonderland.lit") + "</font>");
	jidHintLabel->setAlignment(Qt::AlignRight);
	singleAccountLayout->addWidget(jidHintLabel);

	QLabel* passwordLabel = new QLabel();
	passwordLabel->setText("<font size='-1'>" + tr("Password:") + "</font>");
	passwordLabel->setAccessibleName(tr("User password"));
	passwordLabel->setAccessibleDescription(tr("This is the password you'll use to log in to the XMPP service"));
	singleAccountLayout->addWidget(passwordLabel);

	QWidget* w = new QWidget(this);
	w->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	singleAccountLayout->addWidget(w);

	QHBoxLayout* credentialsLayout = new QHBoxLayout(w);
	credentialsLayout->setMargin(0);
	credentialsLayout->setSpacing(3);
	password_ = new QLineEdit(this);
	password_->setEchoMode(QLineEdit::Password);
	connect(password_, SIGNAL(returnPressed()), this, SLOT(loginClicked()));
	connect(username_->lineEdit(), SIGNAL(returnPressed()), password_, SLOT(setFocus()));
	connect(username_, SIGNAL(editTextChanged(const QString&)), this, SLOT(handleUsernameTextChanged()));
	credentialsLayout->addWidget(password_);

	certificateButton_ = new QToolButton(this);
	certificateButton_->setCheckable(true);
	certificateButton_->setIcon(QIcon(":/icons/certificate.png"));
	certificateButton_->setToolTip(tr("Click if you have a personal certificate used for login to the service."));
	certificateButton_->setWhatsThis(tr("Click if you have a personal certificate used for login to the service."));
	certificateButton_->setAccessibleName(tr("Login with certificate"));
	certificateButton_->setAccessibleDescription(tr("Click if you have a personal certificate used for login to the service."));

	credentialsLayout->addWidget(certificateButton_);
	connect(certificateButton_, SIGNAL(clicked(bool)), SLOT(handleCertficateChecked(bool)));

	loginButton_ = new QPushButton(this);
	loginButton_->setText(tr("Connect"));
	loginButton_->setAutoDefault(true);
	loginButton_->setDefault(true);
	loginButton_->setAccessibleName(tr("Connect now"));
	singleAccountLayout->addWidget(loginButton_);

	QLabel* connectionOptionsLabel = new QLabel(this);
	connectionOptionsLabel->setText("<a href=\"#\"><font size='-1'>" + QObject::tr("Connection Options") + "</font></a>");
	connectionOptionsLabel->setTextFormat(Qt::RichText);
	connectionOptionsLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	singleAccountLayout->addWidget(connectionOptionsLabel);
	connect(connectionOptionsLabel, SIGNAL(linkActivated(const QString&)), SLOT(handleOpenConnectionOptions()));

	message_ = new QLabel(this);
	message_->setTextFormat(Qt::RichText);
	message_->setWordWrap(true);
	setMessage("");
	singleAccountLayout->addStretch(2);
	singleAccountLayout->addWidget(message_);
	singleAccountLayout->addStretch(2);

	remember_ = new QCheckBox(tr("Remember Password?"), this);
	singleAccountLayout->addWidget(remember_);
	loginAutomatically_ = new QCheckBox(tr("Login Automatically?"), this);
	singleAccountLayout->addWidget(loginAutomatically_);

	layout->addWidget(singleAccountWrapper_);

	connect(loginButton_, SIGNAL(clicked()), SLOT(loginClicked()));
	stack_->addWidget(loginWidgetWrapper_);
#ifdef SWIFTEN_PLATFORM_MACOSX
	menuBar_ = new QMenuBar(NULL);
#else
	menuBar_ = menuBar();
#endif
	QApplication::setQuitOnLastWindowClosed(false);

	swiftMenu_ = new QMenu(tr("&Swift"), this);
#ifdef SWIFTEN_PLATFORM_MACOSX
	generalMenu_ = new QMenu(tr("&General"), this);
#else
	generalMenu_ = swiftMenu_;
#endif

#ifdef SWIFTEN_PLATFORM_MACOSX
	QAction* aboutAction = new QAction(QString("&About %1").arg("Swift"), this);
#else
	QAction* aboutAction = new QAction(QString(tr("&About %1")).arg("Swift"), this);
#endif
	connect(aboutAction, SIGNAL(triggered()), SLOT(handleAbout()));
	swiftMenu_->addAction(aboutAction);

	xmlConsoleAction_ = new QAction(tr("&Show Debug Console"), this);
	connect(xmlConsoleAction_, SIGNAL(triggered()), SLOT(handleShowXMLConsole()));
	generalMenu_->addAction(xmlConsoleAction_);

#ifdef SWIFT_EXPERIMENTAL_FT
	fileTransferOverviewAction_ = new QAction(tr("Show &File Transfer Overview"), this);
	connect(fileTransferOverviewAction_, SIGNAL(triggered()), SLOT(handleShowFileTransferOverview()));
	generalMenu_->addAction(fileTransferOverviewAction_);
#endif

	highlightEditorAction_ = new QAction(tr("&Edit Highlight Rules"), this);
	connect(highlightEditorAction_, SIGNAL(triggered()), SLOT(handleShowHighlightEditor()));
	generalMenu_->addAction(highlightEditorAction_);

	toggleSoundsAction_ = new QAction(tr("&Play Sounds"), this);
	toggleSoundsAction_->setCheckable(true);
	toggleSoundsAction_->setChecked(settings_->getSetting(SettingConstants::PLAY_SOUNDS));
	connect(toggleSoundsAction_, SIGNAL(toggled(bool)), SLOT(handleToggleSounds(bool)));
	generalMenu_->addAction(toggleSoundsAction_);

	toggleNotificationsAction_ = new QAction(tr("Display Pop-up &Notifications"), this);
	toggleNotificationsAction_->setCheckable(true);
	toggleNotificationsAction_->setChecked(settings_->getSetting(SettingConstants::SHOW_NOTIFICATIONS));
	connect(toggleNotificationsAction_, SIGNAL(toggled(bool)), SLOT(handleToggleNotifications(bool)));

#ifndef SWIFTEN_PLATFORM_MACOSX
	swiftMenu_->addSeparator();
#endif

#ifdef SWIFTEN_PLATFORM_MACOSX
	QAction* quitAction = new QAction("&Quit", this);
#else
	QAction* quitAction = new QAction(tr("&Quit"), this);
#endif
	connect(quitAction, SIGNAL(triggered()), SLOT(handleQuit()));
	swiftMenu_->addAction(quitAction);

	setInitialMenus();
	settings_->onSettingChanged.connect(boost::bind(&QtLoginWindow::handleSettingChanged, this, _1));

	bool eagle = settings_->getSetting(SettingConstants::FORGET_PASSWORDS);
	remember_->setEnabled(!eagle);
	loginAutomatically_->setEnabled(!eagle);
	xmlConsoleAction_->setEnabled(!eagle);
	if (eagle) {
		remember_->setChecked(false);
		loginAutomatically_->setChecked(false);
	}

#ifdef SWIFTEN_PLATFORM_MACOSX
	// Temporary workaround for case 501. Could be that this code is still
	// needed when Qt provides a proper fix
	qApp->installEventFilter(this);
#endif

	if (multiaccountEnabled) {
		updateViewLabelText();
		if (settings_->getSetting(QtUISettingConstants::LAST_VIEW_WAS_MULTIACCOUNT)) {
			handleChangeView();
		}
	}

	this->show();
}

void QtLoginWindow::setShowNotificationToggle(bool toggle) {
	if (toggle) {
		QList< QAction* > generalMenuActions = generalMenu_->actions();
		generalMenu_->insertAction(generalMenuActions.at(generalMenuActions.count()-2), toggleNotificationsAction_);
	}
	else {
		generalMenu_->removeAction(toggleNotificationsAction_);
	}
}

bool QtLoginWindow::eventFilter(QObject *obj, QEvent *event) {
	if (obj == username_->view() && event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace) {
			QString jid(username_->view()->currentIndex().data().toString());
			int result = QMessageBox::question(this, tr("Remove profile"), tr("Remove the profile '%1'?").arg(jid), QMessageBox::Yes | QMessageBox::No);
			if (result == QMessageBox::Yes) {
				onPurgeSavedLoginRequest(Q2PSTRING(jid)); // accountsManager->removeAccount(jid); maybe?
			}
			return true;
		}
	}
#ifdef SWIFTEN_PLATFORM_MACOSX
	// Dock clicked
	// Temporary workaround for case 501. Could be that this code is still
	// needed when Qt provides a proper fix
	if (obj == qApp && event->type() == QEvent::ApplicationActivate && !isVisible()) {
		bringToFront();
	}
#endif

	return QObject::eventFilter(obj, event);
}

void QtLoginWindow::handleSettingChanged(const std::string& settingPath) {
	if (settingPath == SettingConstants::PLAY_SOUNDS.getKey()) {
		toggleSoundsAction_->setChecked(settings_->getSetting(SettingConstants::PLAY_SOUNDS));
	}
	if (settingPath == SettingConstants::SHOW_NOTIFICATIONS.getKey()) {
		toggleNotificationsAction_->setChecked(settings_->getSetting(SettingConstants::SHOW_NOTIFICATIONS));
	}
}

void QtLoginWindow::setAccountsManager(AccountsManager* manager) {
	accountsManager_ = manager;

	// Multi account
	accountsList_->setManager(manager);
	if (accountsManager_->getDefaultAccount()) {
		accountsList_->setDefaultAccount(accountsManager_->getDefaultAccount()->getIndex());
	}
	accountsList_->onAccountWantsToBeDefault.connect(boost::bind(&QtLoginWindow::handleAccountWantsToBeDefault, this, _1));

	// Single account
	for(int i = 0; i < manager->accountsCount(); i++) {
		username_->addItem(P2QSTRING(manager->getAccountAt(i)->getJID().toString()));
	}
}

void QtLoginWindow::selectUser(const std::string& username) {
	int index = username_->findText(P2QSTRING(username));
	if (index != -1) {
		username_->setCurrentIndex(index);
		password_->setFocus();
	}
	else {
		username_->lineEdit()->setText("");
		username_->setFocus();
	}
}

void QtLoginWindow::addAvailableAccount(boost::shared_ptr<Account> account) {
	QString username = P2QSTRING(account->getJID().toString());
	if (username_->findText(username) == -1) {
		username_->addItem(username);
	}
	accountsList_->addAccountToList(account);
}

void QtLoginWindow::removeAvailableAccount(int index) {
	if (index >= 0 && index < username_->count()) {
		username_->removeItem(index);
		accountsList_->removeAccountFromList(index);
	}
	if (username_->count() == 0) {
		selectUser("");
	}
}

void QtLoginWindow::clearPassword() {
	password_->clear();
	// TODO: clear passwords in accounts list also
}

void QtLoginWindow::handleUsernameTextChanged() {
	boost::shared_ptr<Account> account = accountsManager_->getAccountByJIDString(Q2PSTRING(username_->currentText()));
	if (account) {
		certificateFile_ = P2QSTRING(account->getCertificatePath());
		password_->setText(P2QSTRING(account->getPassword()));
		remember_->setChecked(account->getRememberPassword());
		loginAutomatically_->setChecked(account->getLoginAutomatically());
		currentOptions_ = account->getClientOptions();
	}
	else {
		certificateFile_ = "";
		password_->setText("");
		remember_->setChecked(false);
		loginAutomatically_->setChecked(false);
		currentOptions_ = ClientOptions();
	}
	certificateButton_->setChecked(!certificateFile_.isEmpty());
}

void QtLoginWindow::loggedOut() {
	stack_->removeWidget(stack_->currentWidget());
	stack_->addWidget(loginWidgetWrapper_);
	stack_->setCurrentWidget(loginWidgetWrapper_);
	setInitialMenus();
	setIsLoggingIn(false);
}

void QtLoginWindow::setIsLoggingIn(bool loggingIn) {
	/* Change the for loop as well if you add to this.*/
	QWidget* widgets[7] = {username_, password_, remember_, loginAutomatically_, certificateButton_, viewLabel_, accountsListWrapper_};
	loginButton_->setText(loggingIn ? tr("Cancel") : tr("Connect"));
	for (int i = 0; i < 7; i++) {
		widgets[i]->setEnabled(!loggingIn);
	}
	bool eagle = settings_->getSetting(SettingConstants::FORGET_PASSWORDS);
	remember_->setEnabled(!eagle);
	loginAutomatically_->setEnabled(!eagle);
}

void QtLoginWindow::loginClicked() {
	if (username_->isEnabled()) {
		std::string banner = settings_->getSetting(QtUISettingConstants::CLICKTHROUGH_BANNER);
		if (!banner.empty()) {
			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Confirm terms of use"));
			msgBox.setText("");
			msgBox.setInformativeText(P2QSTRING(banner));
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			if (msgBox.exec() != QMessageBox::Yes) {
				return;
			}
		}

		onLoginRequest(Q2PSTRING(username_->currentText()), Q2PSTRING(password_->text()), Q2PSTRING(certificateFile_), currentOptions_, remember_->isChecked(), loginAutomatically_->isChecked());
		if (settings_->getSetting(SettingConstants::FORGET_PASSWORDS)) { /* Mustn't remember logins */
			username_->clearEditText();
			password_->setText("");
		}
	}
	else {
		onCancelLoginRequest(Q2PSTRING(username_->lineEdit()->text()));
	}
}

// To be deleted? We set 'login automatically' after username_ text changed
void QtLoginWindow::setLoginAutomatically(bool loginAutomatically) {
	loginAutomatically_->setChecked(loginAutomatically);
}

void QtLoginWindow::handleCertficateChecked(bool checked) {
	if (checked) {
#ifdef HAVE_SCHANNEL
		certificateFile_ = P2QSTRING(selectCAPICertificate());
		if (certificateFile_.isEmpty()) {
			certificateButton_->setChecked(false);
		}
#else
		certificateFile_ = QFileDialog::getOpenFileName(this, tr("Select an authentication certificate"), QString(), tr("P12 files (*.cert *.p12 *.pfx);;All files (*.*)"));
		if (certificateFile_.isEmpty()) {
			certificateButton_->setChecked(false);
		}
#endif
	}
	else {
		certificateFile_ = "";
	}
}

void QtLoginWindow::handleAbout() {
	if (!aboutDialog_) {
		aboutDialog_ = new QtAboutWidget();
		aboutDialog_->show();
	}
	else {
		aboutDialog_->show();
		aboutDialog_->raise();
		aboutDialog_->activateWindow();
	}
}

void QtLoginWindow::handleShowXMLConsole() {
	uiEventStream_->send(boost::make_shared<RequestXMLConsoleUIEvent>());
}

void QtLoginWindow::handleShowFileTransferOverview() {
	uiEventStream_->send(boost::make_shared<RequestFileTransferListUIEvent>());
}

void QtLoginWindow::handleShowHighlightEditor() {
	uiEventStream_->send(boost::make_shared<RequestHighlightEditorUIEvent>());
}

void QtLoginWindow::handleToggleSounds(bool enabled) {
	settings_->storeSetting(SettingConstants::PLAY_SOUNDS, enabled);
}

void QtLoginWindow::handleToggleNotifications(bool enabled) {
	settings_->storeSetting(SettingConstants::SHOW_NOTIFICATIONS, enabled);
}

void QtLoginWindow::handleAccountWantsToBeDefault(int index) {
	if (accountsManager_->getDefaultAccount()->getIndex() != index) {
		onDefaultAccountChanged(index);
	}
}

void QtLoginWindow::handleQuit() {
	onQuitRequest();
}

void QtLoginWindow::quit() {
	QApplication::quit();
}

void QtLoginWindow::setInitialMenus() {
	menuBar_->clear();
	menuBar_->addMenu(swiftMenu_);
#ifdef SWIFTEN_PLATFORM_MACOSX
	menuBar_->addMenu(generalMenu_);
#endif
}

bool QtLoginWindow::isInMultiaccountView() {
	return accountsListWrapper_->isVisibleTo(this) && !singleAccountWrapper_->isVisibleTo(this);
}

void QtLoginWindow::morphInto(MainWindow *mainWindow) {
	setEnabled(false);
	QtMainWindow *qtMainWindow = dynamic_cast<QtMainWindow*>(mainWindow);
	assert(qtMainWindow);
	stack_->removeWidget(loginWidgetWrapper_);
	stack_->addWidget(qtMainWindow);
	stack_->setCurrentWidget(qtMainWindow);
	setEnabled(true);
	setInitialMenus();
	std::vector<QMenu*> mainWindowMenus = qtMainWindow->getMenus();
	viewMenu_ = mainWindowMenus[0];
	foreach (QMenu* menu, mainWindowMenus) {
		menuBar_->addMenu(menu);
	}
	setFocus();
}

void QtLoginWindow::setMessage(const std::string& message) {
	if (!message.empty()) {
		message_->setText("<center><font color=\"red\">" + P2QSTRING(message) + "</font></center>");
	}
	else {
		message_->setText("");
	}
}

void QtLoginWindow::toggleBringToFront() {
	if (!isVisible()) {
		bringToFront();
	}
	else {
		window()->hide();
	}
}

void QtLoginWindow::bringToFront() {
	window()->showNormal();
	window()->raise();
	window()->activateWindow();
}

void QtLoginWindow::hide() {
	window()->hide();
}

QtLoginWindow::QtMenus QtLoginWindow::getMenus() const {
	return QtMenus(swiftMenu_, generalMenu_);
}

void QtLoginWindow::resizeEvent(QResizeEvent*) {
	emit geometryChanged();
}

void QtLoginWindow::moveEvent(QMoveEvent*) {
	emit geometryChanged();
}

bool QtLoginWindow::askUserToTrustCertificatePermanently(const std::string& message, const std::vector<Certificate::ref>& certificates) {
	QMessageBox dialog(this);

	dialog.setText(tr("The certificate presented by the server is not valid."));
	dialog.setInformativeText(P2QSTRING(message) + "\n\n" + tr("Would you like to permanently trust this certificate? This must only be done if you know it is correct."));

	dialog.addButton(tr("Show Certificate"), QMessageBox::HelpRole);
	dialog.addButton(QMessageBox::Yes);
	dialog.addButton(QMessageBox::No);
	dialog.setDefaultButton(QMessageBox::No);
	while (true) {
		int result = dialog.exec();
		if (result == QMessageBox::Yes || result == QMessageBox::No) {
			return result == QMessageBox::Yes;
		}
		// FIXME: This isn't very nice, because the dialog disappears every time. We actually need a real
		// dialog with a custom button.
		QtMainWindow::openCertificateDialog(certificates, &dialog);
	}
}

void QtLoginWindow::handleOpenConnectionOptions() {
	QtConnectionSettingsWindow connectionSettings(currentOptions_);
	if (connectionSettings.exec() == QDialog::Accepted) {
		currentOptions_ = connectionSettings.getOptions();
	}
}

void QtLoginWindow::handleChangeView() {
	if (singleAccountWrapper_->isVisibleTo(this)) {
		singleAccountWrapper_->hide();
		(static_cast<QBoxLayout*>(accountsListWrapper_->layout()))->insertWidget(2, message_);
		accountsListWrapper_->show();
	}
	else {
		accountsListWrapper_->hide();
		(static_cast<QBoxLayout*>(singleAccountWrapper_->layout()))->insertWidget(8, message_);
		singleAccountWrapper_->show();
	}
	updateViewLabelText();
	settings_->storeSetting(QtUISettingConstants::LAST_VIEW_WAS_MULTIACCOUNT, accountsListWrapper_->isVisibleTo(this));
}

void QtLoginWindow::updateViewLabelText() {
	if (singleAccountWrapper_->isVisibleTo(this)) {
		viewLabel_->setText("<a href=\"#\"><font size='-1'>" + QObject::tr("Use multiple accounts") + "</font></a>");
	}
	else {
		viewLabel_->setText("<a href=\"#\"><font size='-1'>" + QObject::tr("Use one account") + "</font></a>");
	}
}

void QtLoginWindow::handleAddAccountClicked() {
	accountsManager_->addAccount();
}

}
