/*
 * Copyright (c) 2011 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#pragma once

#include <map>
#include <string>

#include <Swiften/Base/boost_bsignals.h>
#include <Swiften/Elements/Message.h>

#include <Swiftob/Users.h>
#include <Swiftob/MUCs.h>

namespace Swift {
	class Client;
}

class Storage;
class Commands {
	typedef boost::function<void(Swift::Message::ref)> ListenerCallback;
	public:
		enum RoleList {Anyone, Owner};
	public:
		class Command {
			public:
				Command(RoleList allowedBy, const std::string& description) : allowedBy_(allowedBy), description_(description) {
				}

				virtual ~Command() {};

				boost::signal<void (const std::string&, const std::string&, Swift::Message::ref)> onReceived;
				RoleList getAllowedBy() {return allowedBy_;}
				std::string getDescription() {return description_;}
			private:
				RoleList allowedBy_;
				std::string description_;
		};

	public:
		Commands(Users* users, Swift::Client* client, Storage* storage, MUCs* mucs);
		~Commands();
		bool hasCommand(const std::string&);
		bool runCommand(const std::string& command, const std::string& params, Swift::Message::ref message);
		void runListeners(Swift::Message::ref message);
		void replyTo(Swift::Message::ref source, std::string replyBody, bool outOfMUC = false);
		void registerCommand(const std::string& name, RoleList roles, const std::string& description, boost::function<void(const std::string& /*command*/, const std::string& /*params*/, Swift::Message::ref)> callback);
		void registerListener(ListenerCallback);
		void resetCommands();
		void setRehashError(const std::string& error);

	public:
		boost::signal<void ()> onReset;
		boost::signal<void ()> onRestartRequested;
	private:
		void clearCommands();
		bool roleIn(const Users::User::Role userRole, RoleList roles);
		void handleQuitCommand(const std::string& command, const std::string& params, Swift::Message::ref message);
		void handleHelpCommand(const std::string& command, const std::string& params, Swift::Message::ref message);
		void handleJoinCommand(const std::string& /*command*/, const std::string& params, Swift::Message::ref message);
		void handleJoinCommandSuccess(const Swift::JID& room, Swift::Message::ref message);
		void handleJoinCommandFailure(const Swift::JID& room, const std::string& error, Swift::Message::ref message);
		void handlePartCommand(const std::string& /*command*/, const std::string& params, Swift::Message::ref message);
		void handleRehashCommand(const std::string& command, const std::string& params, Swift::Message::ref message);
		void handleRestartCommand(const std::string& command, const std::string& params, Swift::Message::ref message);
		void handleChangeNick(const std::string& command, const std::string& params, Swift::Message::ref message);
		void handleChangeOwner(const std::string& command, const std::string& params, Swift::Message::ref message);
	private:
		std::map<std::string, Command*> commands_;
		std::vector<ListenerCallback> listeners_;
		Users* users_;
		Swift::Client* client_;
		Storage* storage_;
		MUCs* mucs_;
		std::string rehashError_;
};


