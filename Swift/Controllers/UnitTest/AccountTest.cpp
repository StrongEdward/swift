/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <Swift/Controllers/Account.h>
#include <Swift/Controllers/Settings/DummySettingsProvider.h>

using namespace Swift;
using namespace std;

class AccountTest : public CppUnit::TestFixture {
		CPPUNIT_TEST_SUITE(AccountTest);
		CPPUNIT_TEST(testSavingPassword);
		CPPUNIT_TEST(testNotSavingPassword);
		CPPUNIT_TEST(testOneAccountSerialization);
		CPPUNIT_TEST_SUITE_END();

	public:
		void setUp() {
			settings = new DummySettingsProvider();
			account = new Account("shrek", settings);
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
			ClientOptions options;
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
			delete account;
			account = new Account (3, "ShrekAccount", "shrek@jabber.org", "greenskin", "/home/shrek/cert.cert", options, true, false, settings);

			stringstream stream;
			boost::archive::text_oarchive outArchive(stream);
			outArchive << account;

			ClientOptions options2;
			Account* desAccount = new Account(2, "NotShrekAccount", "notshrek@jabber.org", "notgreenskin", "/home/notshrek/cert.cert", options2, false, true, settings);

			boost::archive::text_iarchive inArchive(stream);
			inArchive >> desAccount;

			CPPUNIT_ASSERT(accountsAreEqual(account, desAccount));
			delete desAccount;
		}

	private:
		DummySettingsProvider* settings;
		Account* account;

		bool accountsAreEqual(const Account* a, const Account* b) {
			return a->getAccountName() == b->getAccountName() && a->getCertificatePath() == b->getCertificatePath() && a->getClientOptions() == b->getClientOptions() &&	a->getColor() == b->getColor() && a->getIndex() == b->getIndex() &&	a->getJID() == b->getJID() && a->getLoginAutomatically() == b->getLoginAutomatically() && a->getPassword() == b->getPassword() && a->getRememberPassword() == b->getRememberPassword();
		}
};

CPPUNIT_TEST_SUITE_REGISTRATION(AccountTest);
