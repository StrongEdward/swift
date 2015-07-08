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

#include <string>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <Swiften/Base/boost_bsignals.h>
#include <Swiften/Client/ClientOptions.h>
#include <Swiften/JID/JID.h>

namespace Swift {
	class SettingsProvider;
	class ProfileSettingsProvider;

	struct RGBColor {
			RGBColor() : red(200), green(50), blue(100){} // TODO: random color
			RGBColor(short int r, short int g, short int b) : red(r), green(g), blue(b) {}
			bool isValid() {
				return red >= 0 && red <= 255 && green >= 0 && green <= 255 && blue >= 0 && blue <= 255;
			}
			bool operator==(const RGBColor& other) const {
				return	red == other.red && green == other.green && blue == other.blue;
			}

			short int red;
			short int green;
			short int blue;
	};

	class Account {
		public:
			Account();
			Account(const std::string& profile, SettingsProvider* settings, int index = -1);
			Account(int index, const std::string& accountName, const std::string& jid, const std::string& password, const std::string& certificatePath, const ClientOptions& options, bool rememberPassword, bool autoLogin, /*bool enabled,*/ /*bool isDefault,*/ SettingsProvider* settings);
			~Account();

			void clearPassword();

			int getIndex() const;
			std::string getAccountName() const;
			const JID& getJID() const;
			const std::string& getPassword() const;
			const std::string& getCertificatePath() const;
			const ClientOptions& getClientOptions() const;
			//bool isDefault();
			bool getRememberPassword() const;
			bool getLoginAutomatically() const;
			bool isEnabled() const;
			RGBColor getColor() const;
			ProfileSettingsProvider* getProfileSettings() const;

			void setIndex(int newIndex);
			void setAccountName(const std::string& newName);
			void setJID(const std::string& newJID);
			void setPassword(const std::string& newPassword);
			void setCertificatePath(const std::string& newPath);
			void setClientOptions(const ClientOptions& newOptions);
			//void setDefault(bool isDefault);
			void setRememberPassword(bool remember);
			void setLoginAutomatically(bool autoLogin);
			void setEnabled(bool enabled);
			void setColor(RGBColor color);

		public:
			boost::signal<void (bool)> onEnabledChanged;

		private:
			friend class boost::serialization::access;
			template<class Archive> void serialize(Archive& ar, const unsigned int version);

			void storeAllSettings();
			void determineColor();
			std::string serializeClientOptions(const ClientOptions& options);
			ClientOptions parseClientOptions(const std::string& optionString);

		private:
			int index_;
			std::string accountName_;
			JID jid_;
			std::string password_;
			std::string certificatePath_;
			ClientOptions clientOptions_;
			bool rememberPassword_;
			bool autoLogin_;
			bool enabled_;
			//bool isDefault_;
			RGBColor color_;

			SettingsProvider* settings_;
			ProfileSettingsProvider* profileSettings_;
	};

	template<class Archive>
	void Account::serialize(Archive& ar, const unsigned int /*version*/) {
		ar & index_;
		ar & accountName_;
		ar & jid_;
		ar & password_;
		ar & certificatePath_;
		ar & clientOptions_;
		ar & rememberPassword_;
		ar & autoLogin_;
		ar & enabled_;
		ar & color_.red;
		ar & color_.green;
		ar & color_.blue;
	}
}
