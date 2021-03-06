/*
 * Copyright (c) 2013 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#pragma once

#include <Swiften/Base/Override.h>

#include <Sluift/GenericLuaElementConvertor.h>
#include <Swiften/Elements/DiscoItems.h>

namespace Swift {
	class DiscoItemsConvertor : public GenericLuaElementConvertor<DiscoItems> {
		public:
			DiscoItemsConvertor();
			virtual ~DiscoItemsConvertor();

			virtual boost::shared_ptr<DiscoItems> doConvertFromLua(lua_State*) SWIFTEN_OVERRIDE;
			virtual void doConvertToLua(lua_State*, boost::shared_ptr<DiscoItems>) SWIFTEN_OVERRIDE;
	};
}
