/*
 * TestSecurityBlock.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "security/TestSecurityBlock.h"
#include <cppunit/extensions/HelperMacros.h>

#include <ibrdtn/data/PayloadBlock.h>
#include <ibrdtn/security/BundleAuthenticationBlock.h>
#include <ibrdtn/security/SecurityKey.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/Serializer.h>
#include <ibrcommon/data/BLOB.h>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION (TestSecurityBlock);

class SecurityStringKey : public dtn::security::SecurityKey
{
public:
	SecurityStringKey(const std::string &data) : _data(data) {};
	virtual ~SecurityStringKey() {};

	virtual const std::string getData() const
	{
		return _data;
	}

private:
	const std::string _data;
};

void TestSecurityBlock::setUp(void)
{
}

void TestSecurityBlock::tearDown(void)
{
}

void TestSecurityBlock::localBABTest(void)
{
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://source/app");
	b._destination = dtn::data::EID("dtn://destination/app");
	b._procflags |= dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON;
	b._lifetime = 3600;

	const dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();
	ibrcommon::BLOB::Reference ref = p.getBLOB();

	// write some data
	{
		ibrcommon::BLOB::iostream io = ref.iostream();
		(*io) << "Hallo Welt";
	}

	// create a new key
	SecurityStringKey key("0123456789");
	key.reference = dtn::data::EID("dtn://source");

	// sign the bundle
	dtn::security::BundleAuthenticationBlock::auth(b, key);

	// sign the bundle
	dtn::security::BundleAuthenticationBlock::verify(b, key);
}

void TestSecurityBlock::serializeBABTest(void)
{
	// create a temporary buffer to the serialization tests
	std::stringstream ss;
	dtn::data::DefaultSerializer ds(ss);

	// we create an serialize a new bundle with SecurityBlock appended
	{
		dtn::data::Bundle b;
		b._source = dtn::data::EID("dtn://source/app");
		b._destination = dtn::data::EID("dtn://destination/app");
		b._procflags |= dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON;
		b._lifetime = 3600;

		const dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();
		ibrcommon::BLOB::Reference ref = p.getBLOB();

		// write some data
		{
			ibrcommon::BLOB::iostream io = ref.iostream();
			(*io) << "Hallo Welt";
		}

		// create a new key
		SecurityStringKey key("0123456789");
		key.reference = dtn::data::EID("dtn://source");

		// sign the bundle
		dtn::security::BundleAuthenticationBlock::auth(b, key);

		ds << b;
	}

	dtn::data::DefaultDeserializer dd(ss);

	// deserialize the bundle
	{
		dtn::data::Bundle b;
		dd >> b;

		// create a new key
		SecurityStringKey key("0123456789");
		key.reference = dtn::data::EID("dtn://source");

		// sign the bundle
		dtn::security::BundleAuthenticationBlock::verify(b, key);
	}
}
