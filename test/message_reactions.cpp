//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "td/telegram/MessageReaction.hpp"
#include "td/telegram/MessageReplyInfo.hpp"
#include "td/telegram/Version.h"
#include "td/telegram/logevent/LogEvent.h"

#include "td/utils/tests.h"

namespace {

auto reaction_snapshot(td::int32 count, bool is_bot, bool is_min = false) {
  auto server_reactions = td::telegram_api::make_object<td::telegram_api::messageReactions>();
  server_reactions->can_see_list_ = true;
  server_reactions->min_ = is_min;
  if (count > 0) {
    server_reactions->results_.push_back(td::telegram_api::make_object<td::telegram_api::reactionCount>(
        0, 0, td::telegram_api::make_object<td::telegram_api::reactionEmoji>("👍"), count));
  }
  return td::MessageReactions::get_message_reactions(nullptr, std::move(server_reactions), is_bot);
}

// Exercise the same store/parse methods as persisted messages without a live Td actor.
// These snapshots contain no file references or sender dependencies.
template <class T>
T reload_snapshot(const T &snapshot) {
  td::log_event::WithContext<td::TlStorerCalcLength, td::Global *> calc_length;
  td::store(snapshot, calc_length);
  td::string data(calc_length.get_length(), '\0');
  td::log_event::WithContext<td::TlStorerUnsafe, td::Global *> storer(td::MutableSlice(data).ubegin());
  td::store(snapshot, storer);
  td::log_event::WithVersion<td::log_event::WithContext<td::TlParser, td::Global *>> parser(data);
  parser.set_version(static_cast<td::int32>(td::Version::Next) - 1);
  T result;
  td::parse(result, parser);
  parser.fetch_end();
  parser.get_status().ensure();
  return result;
}

void check_count(const td::MessageReactions &snapshot, td::int32 count, bool can_get_added_reactions) {
  auto exported = snapshot.get_message_reactions_object(nullptr, td::UserId(), td::UserId());
  ASSERT_EQ(1u, exported->reactions_.size());
  ASSERT_EQ(count, exported->reactions_[0]->total_count_);
  ASSERT_EQ(can_get_added_reactions, exported->can_get_added_reactions_);
}

}  // namespace

TEST(MessageReactions, bot_snapshot_roundtrip) {
  auto snapshot = reaction_snapshot(7, true);
  ASSERT_TRUE(snapshot != nullptr);
  ASSERT_TRUE(!snapshot->need_polling_);
  check_count(*snapshot, 7, false);
  auto reloaded = reload_snapshot(*snapshot);
  check_count(reloaded, 7, false);
  ASSERT_TRUE(!reloaded.need_polling_);
  ASSERT_TRUE(!td::MessageReactions::need_update_message_reactions(snapshot.get(), &reloaded));
}

TEST(MessageReactions, bot_snapshot_refresh_and_removal) {
  auto old_snapshot = reaction_snapshot(7, true);
  auto refreshed = reaction_snapshot(11, true, true);
  refreshed->update_from(*old_snapshot, td::DialogId());
  ASSERT_TRUE(td::MessageReactions::need_update_message_reactions(old_snapshot.get(), refreshed.get()));
  check_count(*refreshed, 11, false);
  auto reloaded = reload_snapshot(*refreshed);
  check_count(reloaded, 11, false);

  auto empty_snapshot = reaction_snapshot(0, true);
  ASSERT_TRUE(empty_snapshot != nullptr);
  empty_snapshot->update_from(reloaded, td::DialogId());
  ASSERT_TRUE(td::MessageReactions::need_update_message_reactions(&reloaded, empty_snapshot.get()));
  ASSERT_TRUE(empty_snapshot->are_empty());
  auto exported = reload_snapshot(*empty_snapshot).get_message_reactions_object(nullptr, td::UserId(), td::UserId());
  ASSERT_TRUE(exported->reactions_.empty());
  ASSERT_TRUE(!exported->can_get_added_reactions_);
  ASSERT_TRUE(td::MessageReactions::get_message_reactions(nullptr, nullptr, true) == nullptr);
}

TEST(MessageReactions, user_snapshot_capabilities_unchanged) {
  auto snapshot = reaction_snapshot(7, false);
  ASSERT_TRUE(snapshot->need_polling_);
  check_count(*snapshot, 7, true);
}

TEST(MessageReactions, bot_reply_snapshot_roundtrip) {
  auto replies = td::telegram_api::make_object<td::telegram_api::messageReplies>();
  replies->replies_ = 3;
  replies->replies_pts_ = 10;
  replies->max_id_ = 42;
  td::MessageReplyInfo snapshot(nullptr, std::move(replies), true);
  ASSERT_TRUE(!snapshot.is_empty());
  auto reloaded = reload_snapshot(snapshot);
  auto exported = reloaded.get_message_reply_info_object(nullptr, td::MessageId());
  ASSERT_TRUE(exported != nullptr);
  ASSERT_EQ(3, exported->reply_count_);
  ASSERT_TRUE(!snapshot.need_update_to(reloaded));
}
