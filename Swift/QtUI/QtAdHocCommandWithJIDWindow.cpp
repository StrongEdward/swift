/*
 * Copyright (c) 2010-2014 Kevin Smith
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#include <boost/bind.hpp>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <Swiften/Elements/Command.h>
#include <Swift/Controllers/UIEvents/UIEventStream.h>
#include <Swift/Controllers/UIEvents/RequestAdHocWithJIDUIEvent.h>
#include <Swift/QtUI/QtAdHocCommandWithJIDWindow.h>
#include <Swift/QtUI/QtFormWidget.h>
#include <Swift/QtUI/QtSwiftUtil.h>

const int FormLayoutIndex = 1;

namespace Swift {
QtAdHocCommandWithJIDWindow::QtAdHocCommandWithJIDWindow(UIEventStream* uiEventStream) : uiEventStream_(uiEventStream) {
	QVBoxLayout* hlayout = new QVBoxLayout(this);

	QLabel* jidLabel = new QLabel("JID:", this);
	hlayout->addWidget(jidLabel);
	jid_ = new QLineEdit(this);
	hlayout->addWidget(jid_);

	QLabel* commandLabel = new QLabel("Command:", this);
	hlayout->addWidget(commandLabel);
	node_ = new QLineEdit(this);
	hlayout->addWidget(node_);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
	QPushButton* rejectButton = buttonBox->addButton("Cancel", QDialogButtonBox::RejectRole);
	connect(rejectButton, SIGNAL(clicked()), this, SLOT(handleRejectClick()));
	QPushButton* acceptButton = buttonBox->addButton("Complete", QDialogButtonBox::AcceptRole);
	connect(acceptButton, SIGNAL(clicked()), this, SLOT(handleAcceptClick()));
	hlayout->addWidget(buttonBox);

	setLayout(hlayout);
	show();
}

QtAdHocCommandWithJIDWindow::~QtAdHocCommandWithJIDWindow() {
}

void QtAdHocCommandWithJIDWindow::handleAcceptClick() {
	const JID jid = JID(Q2PSTRING(jid_->text()));
	const std::string node = Q2PSTRING(node_->text());
	boost::shared_ptr<UIEvent> event(new RequestAdHocWithJIDUIEvent(jid, node));
	uiEventStream_->send(event);
	accept();
}

void QtAdHocCommandWithJIDWindow::handleRejectClick() {
	reject();
}

}