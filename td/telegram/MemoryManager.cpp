//
// Copyright Andrea Cavalli (nospam@warp.ovh) 2020
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "td/telegram/MemoryManager.h"

#include "td/telegram/td_api.h"

#include "td/telegram/AccessRights.h"
#include "td/telegram/Document.h"
#include "td/telegram/files/FileLocation.h"
#include "td/telegram/Td.h"

#include "td/actor/PromiseFuture.h"

#include "td/db/SqliteKeyValue.h"
#include "td/db/SqliteKeyValueAsync.h"

#include <algorithm>
#include <numeric>

#include "td/telegram/CallbackQueriesManager.h"
#include "td/telegram/CountryInfoManager.h"
#include "td/telegram/DownloadManager.h"
#include "td/telegram/GameManager.h"
#include "td/telegram/LinkManager.h"
#include "td/telegram/OptionManager.h"
#include "td/telegram/PrivacyManager.h"
#include "td/telegram/SponsoredMessageManager.h"
#include "td/telegram/ThemeManager.h"
#include "td/telegram/VoiceNotesManager.h"
#include "td/telegram/files/FileManager.h"
#include "td/telegram/AutosaveManager.h"
#include "td/telegram/TranslationManager.h"
#include "td/telegram/NotificationSettingsManager.h"
#include "td/telegram/ReactionManager.h"
#include "td/telegram/TopDialogManager.h"
#include "td/telegram/AttachMenuManager.h"
#include "td/telegram/AuthManager.h"
#include "td/telegram/BotInfoManager.h"
#include "td/telegram/ForumTopicManager.h"
#include "td/telegram/NotificationManager.h"
#include "td/telegram/AccountManager.h"
#include "td/telegram/StatisticsManager.h"
#include "td/telegram/BoostManager.h"
#include "td/telegram/CommonDialogManager.h"
#include "td/telegram/DialogActionManager.h"
#include "td/telegram/DialogInviteLinkManager.h"
#include "td/telegram/MessageImportManager.h"
#include "td/telegram/SavedMessagesManager.h"
#include "td/telegram/TranscriptionManager.h"
#include "td/telegram/BusinessManager.h"
#include "td/telegram/DialogFilterManager.h"
#include "td/telegram/DialogManager.h"
#include "td/telegram/DialogParticipantManager.h"
#include "td/telegram/PeopleNearbyManager.h"
#include "td/telegram/QuickReplyManager.h"
#include "td/telegram/StoryManager.h"
#include "td/telegram/TimeZoneManager.h"
#include "td/telegram/UpdatesManager.h"
#include "td/telegram/AnimationsManager.h"
#include "td/telegram/AudiosManager.h"
#include "td/telegram/BackgroundManager.h"
#include "td/telegram/ContactsManager.h"
#include "td/telegram/DocumentsManager.h"
#include "td/telegram/FileReferenceManager.h"
#include "td/telegram/GroupCallManager.h"
#include "td/telegram/InlineQueriesManager.h"
#include "td/telegram/MessagesManager.h"
#include "td/telegram/PollManager.h"
#include "td/telegram/StickersManager.h"
#include "td/telegram/VideoNotesManager.h"
#include "td/telegram/VideosManager.h"
#include "td/telegram/WebPagesManager.h"

namespace td {

tl_object_ptr<td_api::memoryStatistics> MemoryStats::get_memory_statistics_object() const {
  return make_tl_object<td_api::memoryStatistics>(debug);
}

MemoryManager::MemoryManager(Td *td, ActorShared<> parent) : td_(td), parent_(std::move(parent)) {
}

void MemoryManager::start_up() {
}

void MemoryManager::tear_down() {
  parent_.reset();
}

void MemoryManager::get_memory_stats(bool full, Promise<MemoryStats> promise) const {
  vector<string> output = {"{"};

  output.emplace_back("\"memory_stats\":{");

  this->print_managers_memory_stats(output);

  output.emplace_back("}");

  output.emplace_back("}");

  string s;
  s = accumulate(output.begin(), output.end(), s);
  auto value = MemoryStats(s);

  promise.set_value(std::move(value));
}

void MemoryManager::get_current_state(vector<td_api::object_ptr<td_api::Update>> &updates) const {
  if (td_->auth_manager_->is_bot()) {
    return;
  }

  // Never return updates
}

void MemoryManager::print_managers_memory_stats(vector<string> &output) const {
  output.emplace_back("\"callback_queries_manager_\":{"); td_->callback_queries_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"country_info_manager_\":{"); td_->country_info_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"download_manager_\":{"); td_->download_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"game_manager_\":{"); td_->game_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"link_manager_\":{"); td_->link_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"option_manager_\":{"); td_->option_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"privacy_manager_\":{"); td_->privacy_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"sponsored_message_manager_\":{"); td_->sponsored_message_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"theme_manager_\":{"); td_->theme_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"voice_notes_manager_\":{"); td_->voice_notes_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"file_manager_\":{"); td_->file_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"autosave_manager_\":{"); td_->autosave_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"translation_manager_\":{"); td_->translation_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"notification_settings_manager_\":{"); td_->notification_settings_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"reaction_manager_\":{"); td_->reaction_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"top_dialog_manager_\":{"); td_->top_dialog_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"attach_menu_manager_\":{"); td_->attach_menu_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"auth_manager_\":{"); td_->auth_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"bot_info_manager_\":{"); td_->bot_info_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"forum_topic_manager_\":{"); td_->forum_topic_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"notification_manager_\":{"); td_->notification_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"account_manager_\":{"); td_->account_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"statistics_manager_\":{"); td_->statistics_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"boost_manager_\":{"); td_->boost_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"common_dialog_manager_\":{"); td_->common_dialog_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"dialog_action_manager_\":{"); td_->dialog_action_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"dialog_invite_link_manager_\":{"); td_->dialog_invite_link_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"message_import_manager_\":{"); td_->message_import_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"saved_messages_manager_\":{"); td_->saved_messages_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"transcription_manager_\":{"); td_->transcription_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"business_manager_\":{"); td_->business_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"dialog_filter_manager_\":{"); td_->dialog_filter_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"dialog_manager_\":{"); td_->dialog_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"dialog_participant_manager_\":{"); td_->dialog_participant_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"people_nearby_manager_\":{"); td_->people_nearby_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"quick_reply_manager_\":{"); td_->quick_reply_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"story_manager_\":{"); td_->story_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"time_zone_manager_\":{"); td_->time_zone_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"updates_manager_\":{"); td_->updates_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"animations_manager_\":{"); td_->animations_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"audios_manager_\":{"); td_->audios_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"background_manager_\":{"); td_->background_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"contacts_manager_\":{"); td_->contacts_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"documents_manager_\":{"); td_->documents_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"file_reference_manager_\":{"); td_->file_reference_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"group_call_manager_\":{"); td_->group_call_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"inline_queries_manager_\":{"); td_->inline_queries_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"messages_manager_\":{"); td_->messages_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"poll_manager_\":{"); td_->poll_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"stickers_manager_\":{"); td_->stickers_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"video_notes_manager_\":{"); td_->video_notes_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"videos_manager_\":{"); td_->videos_manager_->memory_stats(output); output.emplace_back("}");
  output.emplace_back(",");
  output.emplace_back("\"web_pages_manager_\":{"); td_->web_pages_manager_->memory_stats(output); output.emplace_back("}");
}

}  // namespace td
