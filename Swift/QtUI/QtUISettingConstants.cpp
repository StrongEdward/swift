/*
 * Copyright (c) 2012-2013 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

/*
 * Copyright (c) 2015 Daniel Baczynski
 * Licensed under the Simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swift/QtUI/QtUISettingConstants.h>

namespace Swift {

const SettingsProvider::Setting<bool> QtUISettingConstants::COMPACT_ROSTER("compactRoster", false);
const SettingsProvider::Setting<std::string> QtUISettingConstants::CLICKTHROUGH_BANNER("clickthroughBanner", "");
const SettingsProvider::Setting<int> QtUISettingConstants::CURRENT_ROSTER_TAB("currentRosterTab", 0);
const SettingsProvider::Setting<bool> QtUISettingConstants::SHOW_NICK_IN_ROSTER_HEADER("showNickInRosterHeader", true);
const SettingsProvider::Setting<int> QtUISettingConstants::CHATWINDOW_FONT_SIZE("chatWindowFontSize", 0);
const SettingsProvider::Setting<int> QtUISettingConstants::HISTORYWINDOW_FONT_SIZE("historyWindowFontSize", 0);
const SettingsProvider::Setting<bool> QtUISettingConstants::SHOW_EMOTICONS("showEmoticons", true);
const SettingsProvider::Setting<bool> QtUISettingConstants::USE_PLAIN_CHATS("plainChats", false);
const SettingsProvider::Setting<bool> QtUISettingConstants::USE_SCREENREADER("screenreader", false);
const SettingsProvider::Setting<bool> QtUISettingConstants::LAST_VIEW_WAS_MULTIACCOUNT("multiaccountView", false);
}
