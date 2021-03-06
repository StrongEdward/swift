/*
 * Copyright (c) 2013 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#include <Sluift/ElementConvertors/PubSubCreateConvertor.h>

#include <lua.hpp>
#include <boost/smart_ptr/make_shared.hpp>


#include <Sluift/LuaElementConvertors.h>

#pragma clang diagnostic ignored "-Wunused-private-field"

using namespace Swift;

PubSubCreateConvertor::PubSubCreateConvertor(LuaElementConvertors* convertors) : 
		GenericLuaElementConvertor<PubSubCreate>("pubsub_create"),
		convertors(convertors) {
}

PubSubCreateConvertor::~PubSubCreateConvertor() {
}

boost::shared_ptr<PubSubCreate> PubSubCreateConvertor::doConvertFromLua(lua_State* L) {
	boost::shared_ptr<PubSubCreate> result = boost::make_shared<PubSubCreate>();
	lua_getfield(L, -1, "node");
	if (lua_isstring(L, -1)) {
		result->setNode(std::string(lua_tostring(L, -1)));
	}
	lua_pop(L, 1);
	lua_getfield(L, -1, "configure");
	if (!lua_isnil(L, -1)) {
		if (boost::shared_ptr<PubSubConfigure> payload = boost::dynamic_pointer_cast<PubSubConfigure>(convertors->convertFromLuaUntyped(L, -1, "pubsub_configure"))) {
			result->setConfigure(payload);
		}
	}
	lua_pop(L, 1);
	return result;
}

void PubSubCreateConvertor::doConvertToLua(lua_State* L, boost::shared_ptr<PubSubCreate> payload) {
	lua_createtable(L, 0, 0);
	lua_pushstring(L, payload->getNode().c_str());
	lua_setfield(L, -2, "node");
	if (convertors->convertToLuaUntyped(L, payload->getConfigure()) > 0) {
		lua_setfield(L, -2, "configure");
	}
}

boost::optional<LuaElementConvertor::Documentation> PubSubCreateConvertor::getDocumentation() const {
	return Documentation(
		"PubSubCreate",
		"This table has the following fields:\n\n"
		"- `node`: string\n"
		"- `configure`: @{PubSubConfigure}\n"
	);
}
