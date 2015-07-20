/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <Swiften/Base/Log.h>

#include <Swift/Controllers/Account.h>
#include <Swift/Controllers/Settings/DummySettingsProvider.h>

using namespace Swift;
using namespace std;

class AccountTest : public CppUnit::TestFixture {
		CPPUNIT_TEST_SUITE(AccountTest);
		CPPUNIT_TEST(testSavingPassword);
		CPPUNIT_TEST(testNotSavingPassword);
		CPPUNIT_TEST(testOneAccountSerialization);
		CPPUNIT_TEST(testThreeAccountsSerialization);
		CPPUNIT_TEST_SUITE_END();

	public:
		void setUp() {
			settings = new DummySettingsProvider();
			account = new Account(3, "ShrekAccount", "shrek@jabber.org", "greenskin", "/home/shrek/cert.cert", options, true, false, settings);
			account2 = new Account(1, "Some acc", "someacc@jabb.org", "passss", "path", options, true, true, settings);
			account3 = new Account(2, "AccountName", "somelogin@jabber.org", "", "/home/foo/cert.cert", options, false, false, settings);

			options.allowPLAINWithoutTLS = false;
			options.boshHTTPConnectProxyAuthID = SafeString("someStringID");
			options.boshHTTPConnectProxyAuthPassword = SafeString("someStringPass");
			options.boshHTTPConnectProxyURL = URL::fromString("http://ebay.com/");
			options.boshURL = URL::fromString("http://ebay2.com/");
			options.forgetPassword = false;
			options.manualHostname = "Manhostname";
			options.manualPort = 47864;
			options.manualProxyHostname = "ShrekProxyHostaname";
			options.manualProxyPort = 44448;
			options.proxyType = ClientOptions::HTTPConnectProxy;
			options.tlsOptions = TLSOptions();
			options.useAcks = true;
			options.useStreamCompression = true;
			options.useStreamResumption = false;
			options.useTLS = ClientOptions::RequireTLS;
		}


		void tearDown() {
			delete settings;
			delete account;
		}

		void testSavingPassword() {
			account->setPassword("secretpass");
			account->setRememberPassword(true);
			CPPUNIT_ASSERT_EQUAL(string("secretpass"), account->getPassword());
		}

		void testNotSavingPassword() {
			account->setPassword("secretpass");
			account->setRememberPassword(false);
			CPPUNIT_ASSERT_EQUAL(string(""), account->getPassword());
		}

		void testOneAccountSerialization() {
			stringstream stream;
			{
				boost::archive::text_oarchive outArchive(stream);
				outArchive << account;
			}

			Account* deserializedAccount = new Account(2, "NotShrekAccount", "notshrek@jabber.org", "notgreenskin", "/home/notshrek/cert.cert", options2, false, true, settings);

			try {
				boost::archive::text_iarchive inArchive(stream);
				inArchive >> deserializedAccount;
			}
			catch (boost::archive::archive_exception& ex) {
				SWIFT_LOG(error) << ex.what() << ", code: " << ex.code;
			}

			CPPUNIT_ASSERT(accountsAreEqual(account, deserializedAccount));
			delete deserializedAccount;
		}

		void testThreeAccountsSerialization() {
			std::stringstream stream;
			{
				boost::archive::text_oarchive outArchive(stream);
				outArchive << account;
				outArchive << account2;
				outArchive << account3;
			}

			Account* deserialized = new Account();
			Account* deserialized2 = new Account();
			Account* deserialized3 = new Account();


			try {
				boost::archive::text_iarchive inArchive(stream);
				inArchive >> deserialized;
				inArchive >> deserialized2;
				inArchive >> deserialized3;
			}
			catch (boost::archive::archive_exception& ex) {
				SWIFT_LOG(error) << ex.what() << ", code: " << ex.code;
			}

			CPPUNIT_ASSERT(accountsAreEqual(account, deserialized));
			CPPUNIT_ASSERT(accountsAreEqual(account2, deserialized2));
			CPPUNIT_ASSERT(accountsAreEqual(account3, deserialized3));

			delete deserialized;
			delete deserialized2;
			delete deserialized3;
		}

	private:
		DummySettingsProvider* settings;
		Account* account;
		Account* account2;
		Account* account3;

		ClientOptions options;
		ClientOptions options2;

		bool accountsAreEqual(const Account* a, const Account* b) {
			return a->getAccountName() == b->getAccountName() && a->getCertificatePath() == b->getCertificatePath() && a->getClientOptions() == b->getClientOptions() &&	a->getColor() == b->getColor() && a->getIndex() == b->getIndex() && a->getJID() == b->getJID() && a->getLoginAutomatically() == b->getLoginAutomatically() && a->getPassword() == b->getPassword() && a->getRememberPassword() == b->getRememberPassword() && a->getRecentChatsSerialized() == b->getRecentChatsSerialized();
		}
};

CPPUNIT_TEST_SUITE_REGISTRATION(AccountTest);
