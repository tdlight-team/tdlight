//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2024
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "base64.h"
#include "td/utils/common.h"
#include "td/utils/SharedSlice.h"
#include "td/utils/Slice.h"
#include "td/utils/StackAllocator.h"
#include "td/utils/StringBuilder.h"
#include "td/utils/UInt.h"

namespace td {

class TlStorerToJsonString {
  decltype(StackAllocator::alloc(0)) buffer_ = StackAllocator::alloc(1 << 14);
  StringBuilder sb_ = StringBuilder(buffer_.as_slice(), true);
  size_t shift_ = 0;
  bool has_previously_appended_field_ = false;

  void store_field_begin(Slice name) {
    sb_.append_char(shift_, ' ');
    if (!name.empty()) {
      if (!has_previously_appended_field_) {
        has_previously_appended_field_ = true;
      } else {
        sb_.push_back(',');
      }
      sb_ << "\"" << name << "\": ";
    }
  }

  void store_field_end() {
    sb_.push_back('\n');
  }

  void store_binary(Slice data) {
    sb_ << "\"" << base64_encode(data) << "\"";
  }

 public:
  TlStorerToJsonString() = default;
  TlStorerToJsonString(const TlStorerToJsonString &) = delete;
  TlStorerToJsonString &operator=(const TlStorerToJsonString &) = delete;
  TlStorerToJsonString(TlStorerToJsonString &&) = delete;
  TlStorerToJsonString &operator=(TlStorerToJsonString &&) = delete;

  void store_field(Slice name, const string &value) {
    store_field_begin(name);
    sb_.push_back('"');
    sb_ << value;
    sb_.push_back('"');
    store_field_end();
  }

  void store_field(Slice name, const SecureString &value) {
    store_field_begin(name);
    sb_ << "\"<secret>\"";
    store_field_end();
  }

  template <class T>
  void store_field(Slice name, const T &value) {
    store_field_begin(name);
    sb_ << value;
    store_field_end();
  }

  void store_bytes_field(Slice name, const SecureString &value) {
    store_field_begin(name);
    sb_ << "\"<secret>\"";
    store_field_end();
  }

  template <class BytesT>
  void store_bytes_field(Slice name, const BytesT &value) {
    store_field_begin(name);
    store_binary(as_slice(value));
    store_field_end();
  }

  template <class ObjectT>
  void store_object_field(CSlice name, const ObjectT *value) {
    if (value == nullptr) {
      store_field(name, Slice("null"));
    } else {
      value->store(*this, name.c_str());
    }
  }

  void store_field(Slice name, const UInt128 &value) {
    store_field_begin(name);
    store_binary(as_slice(value));
    store_field_end();
  }

  void store_field(Slice name, const UInt256 &value) {
    store_field_begin(name);
    store_binary(as_slice(value));
    store_field_end();
  }

  void store_vector_begin(Slice field_name, size_t vector_size) {
    store_field_begin(field_name);
    sb_ << "[\n";
    shift_ += 2;
  }

  void store_vector_end() {
    CHECK(shift_ >= 2);
    shift_ -= 2;
    has_previously_appended_field_ = true;
    sb_.append_char(shift_, ' ');
    sb_ << "]\n";
  }

  void store_class_begin(const char *field_name, Slice class_name) {
    store_field_begin(Slice(field_name));
    sb_ << "{\n";
    shift_ += 2;
    store_field("@type", class_name);
    has_previously_appended_field_ = false;
  }

  void store_class_end() {
    CHECK(shift_ >= 2);
    shift_ -= 2;
    has_previously_appended_field_ = true;
    sb_.append_char(shift_, ' ');
    sb_ << "}\n";
  }

  string move_as_string() {
    return sb_.as_cslice().str();
  }
};

}  // namespace td
