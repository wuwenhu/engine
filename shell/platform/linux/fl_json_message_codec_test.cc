// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_json_message_codec.h"
#include "gtest/gtest.h"

#include <math.h>

// FIXME: Test decoding integers that can't fit into 64 bits

// Encode a message using FlJsonMessageCodec to a UTF-8 string.
static gchar* encode_message(FlValue* value) {
  g_autoptr(FlJsonMessageCodec) codec = fl_json_message_codec_new();
  g_autoptr(GError) error = nullptr;
  g_autofree gchar* result = fl_json_message_codec_encode(codec, value, &error);
  EXPECT_EQ(error, nullptr);
  return static_cast<gchar*>(g_steal_pointer(&result));
}

// Decode a message using FlJsonMessageCodec from UTF-8 string.
static FlValue* decode_message(const char* text) {
  g_autoptr(FlJsonMessageCodec) codec = fl_json_message_codec_new();
  g_autoptr(GError) error = nullptr;
  g_autoptr(FlValue) value = fl_json_message_codec_decode(codec, text, &error);
  EXPECT_EQ(error, nullptr);
  EXPECT_NE(value, nullptr);
  return fl_value_ref(value);
}

// Decode a message using FlJsonMessageCodec from UTF-8 string. Expect the given
// error.
static void decode_error_value(const char* text, GQuark domain, gint code) {
  g_autoptr(FlJsonMessageCodec) codec = fl_json_message_codec_new();
  g_autoptr(GError) error = nullptr;
  g_autoptr(FlValue) value = fl_json_message_codec_decode(codec, text, &error);
  EXPECT_TRUE(g_error_matches(error, domain, code));
  EXPECT_EQ(value, nullptr);
}

TEST(FlJsonMessageCodecTest, EncodeNULL) {
  g_autofree gchar* text = encode_message(NULL);
  EXPECT_STREQ(text, "null");
}

TEST(FlJsonMessageCodecTest, EncodeNull) {
  g_autoptr(FlValue) value = fl_value_new_null();
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "null");
}

TEST(FlJsonMessageCodecTest, DecodeNull) {
  g_autoptr(FlValue) value = decode_message("null");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_NULL);
}

static gchar* encode_bool(gboolean value) {
  g_autoptr(FlValue) v = fl_value_new_bool(value);
  return encode_message(v);
}

TEST(FlJsonMessageCodecTest, EncodeBoolFalse) {
  g_autofree gchar* text = encode_bool(FALSE);
  EXPECT_STREQ(text, "false");
}

TEST(FlJsonMessageCodecTest, EncodeBoolTrue) {
  g_autofree gchar* text = encode_bool(TRUE);
  EXPECT_STREQ(text, "true");
}

TEST(FlJsonMessageCodecTest, DecodeBoolFalse) {
  g_autoptr(FlValue) value = decode_message("false");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_BOOL);
  EXPECT_FALSE(fl_value_get_bool(value));
}

TEST(FlJsonMessageCodecTest, DecodeBoolTrue) {
  g_autoptr(FlValue) value = decode_message("true");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_BOOL);
  EXPECT_TRUE(fl_value_get_bool(value));
}

static gchar* encode_int(int64_t value) {
  g_autoptr(FlValue) v = fl_value_new_int(value);
  return encode_message(v);
}

TEST(FlJsonMessageCodecTest, EncodeIntZero) {
  g_autofree gchar* text = encode_int(0);
  EXPECT_STREQ(text, "0");
}

TEST(FlJsonMessageCodecTest, EncodeIntOne) {
  g_autofree gchar* text = encode_int(1);
  EXPECT_STREQ(text, "1");
}

TEST(FlJsonMessageCodecTest, EncodeInt12345) {
  g_autofree gchar* text = encode_int(12345);
  EXPECT_STREQ(text, "12345");
}

TEST(FlJsonMessageCodecTest, EncodeIntMin) {
  g_autofree gchar* text = encode_int(G_MININT64);
  EXPECT_STREQ(text, "-9223372036854775808");
}

TEST(FlJsonMessageCodecTest, EncodeIntMax) {
  g_autofree gchar* text = encode_int(G_MAXINT64);
  EXPECT_STREQ(text, "9223372036854775807");
}

TEST(FlJsonMessageCodecTest, DecodeIntZero) {
  g_autoptr(FlValue) value = decode_message("0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 0);
}

TEST(FlJsonMessageCodecTest, DecodeIntOne) {
  g_autoptr(FlValue) value = decode_message("1");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 1);
}

TEST(FlJsonMessageCodecTest, DecodeInt12345) {
  g_autoptr(FlValue) value = decode_message("12345");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), 12345);
}

TEST(FlJsonMessageCodecTest, DecodeIntMin) {
  g_autoptr(FlValue) value = decode_message("-9223372036854775808");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), G_MININT64);
}

TEST(FlJsonMessageCodecTest, DecodeIntMax) {
  g_autoptr(FlValue) value = decode_message("9223372036854775807");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_INT);
  EXPECT_EQ(fl_value_get_int(value), G_MAXINT64);
}

TEST(FlJsonMessageCodecTest, DecodeIntLeadingZero1) {
  decode_error_value("00", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_ADDITIONAL_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeIntLeadingZero2) {
  decode_error_value("01", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_ADDITIONAL_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeIntDoubleNegative) {
  decode_error_value("--1", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_NUMBER);
}

TEST(FlJsonMessageCodecTest, DecodeIntPositiveSign) {
  decode_error_value("+1", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_FAILED);
}

TEST(FlJsonMessageCodecTest, DecodeIntHexChar) {
  decode_error_value("0a", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_ADDITIONAL_DATA);
}

static gchar* encode_float(double value) {
  g_autoptr(FlValue) v = fl_value_new_float(value);
  return encode_message(v);
}

TEST(FlJsonMessageCodecTest, EncodeFloatZero) {
  g_autofree gchar* text = encode_float(0);
  EXPECT_STREQ(text, "0.0");
}

TEST(FlJsonMessageCodecTest, EncodeFloatOne) {
  g_autofree gchar* text = encode_float(1);
  EXPECT_STREQ(text, "1.0");
}

TEST(FlJsonMessageCodecTest, EncodeFloatMinusOne) {
  g_autofree gchar* text = encode_float(-1);
  EXPECT_STREQ(text, "-1.0");
}

TEST(FlJsonMessageCodecTest, EncodeFloatHalf) {
  g_autofree gchar* text = encode_float(0.5);
  EXPECT_STREQ(text, "0.5");
}

TEST(FlJsonMessageCodecTest, EncodeFloatPi) {
  g_autofree gchar* text = encode_float(M_PI);
  EXPECT_STREQ(text, "3.1415926535897931");
}

TEST(FlJsonMessageCodecTest, EncodeFloatMinusZero) {
  g_autofree gchar* text = encode_float(-0.0);
  EXPECT_STREQ(text, "-0.0");
}

TEST(FlJsonMessageCodecTest, EncodeFloatNaN) {
  g_autoptr(FlValue) value = fl_value_new_float(NAN);
  g_autoptr(FlJsonMessageCodec) codec = fl_json_message_codec_new();
  g_autoptr(GError) error = nullptr;
  g_autofree gchar* result = fl_json_message_codec_encode(codec, value, &error);
  EXPECT_TRUE(g_error_matches(error, FL_JSON_MESSAGE_CODEC_ERROR,
                              FL_JSON_MESSAGE_CODEC_ERROR_INVALID_NUMBER));
  EXPECT_EQ(result, nullptr);
}

TEST(FlJsonMessageCodecTest, EncodeFloatInfinity) {
  g_autoptr(FlValue) value = fl_value_new_float(INFINITY);
  g_autoptr(FlJsonMessageCodec) codec = fl_json_message_codec_new();
  g_autoptr(GError) error = nullptr;
  g_autofree gchar* result = fl_json_message_codec_encode(codec, value, &error);
  EXPECT_TRUE(g_error_matches(error, FL_JSON_MESSAGE_CODEC_ERROR,
                              FL_JSON_MESSAGE_CODEC_ERROR_INVALID_NUMBER));
  EXPECT_EQ(result, nullptr);
}

TEST(FlJsonMessageCodecTest, DecodeFloatZero) {
  g_autoptr(FlValue) value = decode_message("0.0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), 0.0);
}

TEST(FlJsonMessageCodecTest, DecodeFloatOne) {
  g_autoptr(FlValue) value = decode_message("1.0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), 1.0);
}

TEST(FlJsonMessageCodecTest, DecodeFloatMinusOne) {
  g_autoptr(FlValue) value = decode_message("-1.0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), -1.0);
}

TEST(FlJsonMessageCodecTest, DecodeFloatHalf) {
  g_autoptr(FlValue) value = decode_message("0.5");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), 0.5);
}

TEST(FlJsonMessageCodecTest, DecodeFloatPi) {
  g_autoptr(FlValue) value = decode_message("3.1415926535897931");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), M_PI);
}

TEST(FlJsonMessageCodecTest, DecodeFloatMinusZero) {
  g_autoptr(FlValue) value = decode_message("-0.0");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_FLOAT);
  EXPECT_EQ(fl_value_get_float(value), -0.0);
}

TEST(FlJsonMessageCodecTest, DecodeFloatMissingFraction) {
  decode_error_value("0.", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_NUMBER);
}

TEST(FlJsonMessageCodecTest, DecodeFloatInvalidFraction) {
  decode_error_value("0.a", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_NUMBER);
}

static gchar* encode_string(const gchar* value) {
  g_autoptr(FlValue) v = fl_value_new_string(value);
  return encode_message(v);
}

TEST(FlJsonMessageCodecTest, EncodeStringEmpty) {
  g_autofree gchar* text = encode_string("");
  EXPECT_STREQ(text, "\"\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringHello) {
  g_autofree gchar* text = encode_string("hello");
  EXPECT_STREQ(text, "\"hello\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringEmptySized) {
  g_autoptr(FlValue) value = fl_value_new_string_sized(nullptr, 0);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "\"\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringHelloSized) {
  g_autoptr(FlValue) value = fl_value_new_string_sized("Hello World", 5);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "\"Hello\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringEscapeQuote) {
  g_autofree gchar* text = encode_string("\"");
  EXPECT_STREQ(text, "\"\\\"\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringEscapeBackslash) {
  g_autofree gchar* text = encode_string("\\");
  EXPECT_STREQ(text, "\"\\\\\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringEscapeBackspace) {
  g_autofree gchar* text = encode_string("\b");
  EXPECT_STREQ(text, "\"\\b\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringEscapeFormFeed) {
  g_autofree gchar* text = encode_string("\f");
  EXPECT_STREQ(text, "\"\\f\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringEscapeNewline) {
  g_autofree gchar* text = encode_string("\n");
  EXPECT_STREQ(text, "\"\\n\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringEscapeCarriageReturn) {
  g_autofree gchar* text = encode_string("\r");
  EXPECT_STREQ(text, "\"\\r\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringEscapeTab) {
  g_autofree gchar* text = encode_string("\t");
  EXPECT_STREQ(text, "\"\\t\"");
}

TEST(FlJsonMessageCodecTest, EncodeStringEscapeUnicode) {
  g_autofree gchar* text = encode_string("\u0001");
  EXPECT_STREQ(text, "\"\\u0001\"");
}

TEST(FlJsonMessageCodecTest, DecodeStringEmpty) {
  g_autoptr(FlValue) value = decode_message("\"\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "");
}

TEST(FlJsonMessageCodecTest, DecodeStringHello) {
  g_autoptr(FlValue) value = decode_message("\"hello\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "hello");
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapeQuote) {
  g_autoptr(FlValue) value = decode_message("\"\\\"\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\"");
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapeBackslash) {
  g_autoptr(FlValue) value = decode_message("\"\\\\\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\\");
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapeSlash) {
  g_autoptr(FlValue) value = decode_message("\"\\/\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "/");
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapeBackspace) {
  g_autoptr(FlValue) value = decode_message("\"\\b\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\b");
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapeFormFeed) {
  g_autoptr(FlValue) value = decode_message("\"\\f\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\f");
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapeNewline) {
  g_autoptr(FlValue) value = decode_message("\"\\n\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\n");
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapeCarriageReturn) {
  g_autoptr(FlValue) value = decode_message("\"\\r\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\r");
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapeTab) {
  g_autoptr(FlValue) value = decode_message("\"\\t\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\t");
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapeUnicode) {
  g_autoptr(FlValue) value = decode_message("\"\\u0001\"");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(value), "\u0001");
}

TEST(FlJsonMessageCodecTest, DecodeStringBinary) {
  decode_error_value("\"Hello\x01World\"", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_STRING_CHARACTER);
}

TEST(FlJsonMessageCodecTest, DecodeStringNewline) {
  decode_error_value("\"Hello\nWorld\"", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_STRING_CHARACTER);
}

TEST(FlJsonMessageCodecTest, DecodeStringCarriageReturn) {
  decode_error_value("\"Hello\rWorld\"", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_STRING_CHARACTER);
}

TEST(FlJsonMessageCodecTest, DecodeStringTab) {
  decode_error_value("\"Hello\tWorld\"", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_STRING_CHARACTER);
}

TEST(FlJsonMessageCodecTest, DecodeStringUnterminatedEmpty) {
  decode_error_value("\"", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeStringExtraQuote) {
  decode_error_value("\"\"\"", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_ADDITIONAL_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeStringEscapedClosingQuote) {
  decode_error_value("\"\\\"", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeStringUnknownEscape) {
  decode_error_value(
      "\"\\z\"", FL_JSON_MESSAGE_CODEC_ERROR,
      FL_JSON_MESSAGE_CODEC_ERROR_INVALID_STRING_ESCAPE_SEQUENCE);
}

TEST(FlJsonMessageCodecTest, DecodeStringInvalidUnicode) {
  decode_error_value("\"\\uxxxx\"", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_STRING_UNICODE_ESCAPE);
}

TEST(FlJsonMessageCodecTest, DecodeStringUnicodeNoData) {
  decode_error_value("\"\\u\"", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_STRING_UNICODE_ESCAPE);
}

TEST(FlJsonMessageCodecTest, DecodeStringUnicodeShortData) {
  decode_error_value("\"\\uxx\"", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_INVALID_STRING_UNICODE_ESCAPE);
}

TEST(FlJsonMessageCodecTest, EncodeUint8ListEmpty) {
  g_autoptr(FlValue) value = fl_value_new_uint8_list(nullptr, 0);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonMessageCodecTest, EncodeUint8List) {
  uint8_t data[] = {0, 1, 2, 3, 4};
  g_autoptr(FlValue) value = fl_value_new_uint8_list(data, 5);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[0,1,2,3,4]");
}

TEST(FlJsonMessageCodecTest, EncodeInt32ListEmpty) {
  g_autoptr(FlValue) value = fl_value_new_int32_list(nullptr, 0);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonMessageCodecTest, EncodeInt32List) {
  int32_t data[] = {0, -1, 2, -3, 4};
  g_autoptr(FlValue) value = fl_value_new_int32_list(data, 5);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[0,-1,2,-3,4]");
}

TEST(FlJsonMessageCodecTest, EncodeInt64ListEmpty) {
  g_autoptr(FlValue) value = fl_value_new_int64_list(nullptr, 0);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonMessageCodecTest, EncodeInt64List) {
  int64_t data[] = {0, -1, 2, -3, 4};
  g_autoptr(FlValue) value = fl_value_new_int64_list(data, 5);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[0,-1,2,-3,4]");
}

TEST(FlJsonMessageCodecTest, EncodeFloatListEmpty) {
  g_autoptr(FlValue) value = fl_value_new_float_list(nullptr, 0);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonMessageCodecTest, EncodeFloatList) {
  double data[] = {0, -0.5, 0.25, -0.125, 0.0625};
  g_autoptr(FlValue) value = fl_value_new_float_list(data, 5);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[0.0,-0.5,0.25,-0.125,0.0625]");
}

TEST(FlJsonMessageCodecTest, EncodeListEmpty) {
  g_autoptr(FlValue) value = fl_value_new_list();
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[]");
}

TEST(FlJsonMessageCodecTest, EncodeListTypes) {
  g_autoptr(FlValue) value = fl_value_new_list();
  fl_value_append_take(value, fl_value_new_null());
  fl_value_append_take(value, fl_value_new_bool(TRUE));
  fl_value_append_take(value, fl_value_new_int(42));
  fl_value_append_take(value, fl_value_new_float(-1.5));
  fl_value_append_take(value, fl_value_new_string("hello"));
  fl_value_append_take(value, fl_value_new_list());
  fl_value_append_take(value, fl_value_new_map());
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[null,true,42,-1.5,\"hello\",[],{}]");
}

TEST(FlJsonMessageCodecTest, EncodeListNested) {
  g_autoptr(FlValue) even_numbers = fl_value_new_list();
  g_autoptr(FlValue) odd_numbers = fl_value_new_list();
  for (int i = 0; i < 10; i++) {
    if (i % 2 == 0)
      fl_value_append_take(even_numbers, fl_value_new_int(i));
    else
      fl_value_append_take(odd_numbers, fl_value_new_int(i));
  }
  g_autoptr(FlValue) value = fl_value_new_list();
  fl_value_append(value, even_numbers);
  fl_value_append(value, odd_numbers);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "[[0,2,4,6,8],[1,3,5,7,9]]");
}

TEST(FlJsonMessageCodecTest, DecodeListEmpty) {
  g_autoptr(FlValue) value = decode_message("[]");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_LIST);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlJsonMessageCodecTest, DecodeListNoComma) {
  decode_error_value("[0,1,2,3 4]", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_MISSING_COMMA);
}

TEST(FlJsonMessageCodecTest, DecodeListUnterminatedEmpty) {
  decode_error_value("[", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeListStartUnterminate) {
  decode_error_value("]", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_FAILED);
}

TEST(FlJsonMessageCodecTest, DecodeListUnterminated) {
  decode_error_value("[0,1,2,3,4", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeListDoubleTerminated) {
  decode_error_value("[0,1,2,3,4]]", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_ADDITIONAL_DATA);
}

TEST(FlJsonMessageCodecTest, EncodeMapEmpty) {
  g_autoptr(FlValue) value = fl_value_new_map();
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text, "{}");
}

TEST(FlJsonMessageCodecTest, EncodeMapKeyTypes) {
  g_autoptr(FlValue) value = fl_value_new_map();
  fl_value_set_take(value, fl_value_new_null(), fl_value_new_string("null"));
  fl_value_set_take(value, fl_value_new_bool(TRUE),
                    fl_value_new_string("bool"));
  fl_value_set_take(value, fl_value_new_int(42), fl_value_new_string("int"));
  fl_value_set_take(value, fl_value_new_float(-1.5),
                    fl_value_new_string("float"));
  fl_value_set_take(value, fl_value_new_string("hello"),
                    fl_value_new_string("string"));
  fl_value_set_take(value, fl_value_new_list(), fl_value_new_string("list"));
  fl_value_set_take(value, fl_value_new_map(), fl_value_new_string("map"));
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text,
               "{null:\"null\",true:\"bool\",42:\"int\",-1.5:\"float\","
               "\"hello\":\"string\",[]:\"list\",{}:\"map\"}");
}

TEST(FlJsonMessageCodecTest, EncodeMapValueTypes) {
  g_autoptr(FlValue) value = fl_value_new_map();
  fl_value_set_take(value, fl_value_new_string("null"), fl_value_new_null());
  fl_value_set_take(value, fl_value_new_string("bool"),
                    fl_value_new_bool(TRUE));
  fl_value_set_take(value, fl_value_new_string("int"), fl_value_new_int(42));
  fl_value_set_take(value, fl_value_new_string("float"),
                    fl_value_new_float(-1.5));
  fl_value_set_take(value, fl_value_new_string("string"),
                    fl_value_new_string("hello"));
  fl_value_set_take(value, fl_value_new_string("list"), fl_value_new_list());
  fl_value_set_take(value, fl_value_new_string("map"), fl_value_new_map());
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text,
               "{\"null\":null,\"bool\":true,\"int\":42,\"float\":-"
               "1.5,\"string\":\"hello\",\"list\":[],\"map\":{}}");
}

TEST(FlJsonMessageCodecTest, EncodeMapNested) {
  g_autoptr(FlValue) str_to_int = fl_value_new_map();
  g_autoptr(FlValue) int_to_str = fl_value_new_map();
  const char* numbers[] = {"zero", "one", "two", "three", NULL};
  for (int i = 0; numbers[i] != nullptr; i++) {
    fl_value_set_take(str_to_int, fl_value_new_string(numbers[i]),
                      fl_value_new_int(i));
    fl_value_set_take(int_to_str, fl_value_new_int(i),
                      fl_value_new_string(numbers[i]));
  }
  g_autoptr(FlValue) value = fl_value_new_map();
  fl_value_set_string(value, "str-to-int", str_to_int);
  fl_value_set_string(value, "int-to-str", int_to_str);
  g_autofree gchar* text = encode_message(value);
  EXPECT_STREQ(text,
               "{\"str-to-int\":{\"zero\":0,\"one\":1,\"two\":2,\"three\":3},"
               "\"int-to-str\":{0:\"zero\",1:\"one\",2:\"two\",3:\"three\"}}");
}

TEST(FlJsonMessageCodecTest, DecodeMapEmpty) {
  g_autoptr(FlValue) value = decode_message("{}");
  ASSERT_EQ(fl_value_get_type(value), FL_VALUE_TYPE_MAP);
  EXPECT_EQ(fl_value_get_length(value), static_cast<size_t>(0));
}

TEST(FlJsonMessageCodecTest, DecodeMapUnterminatedEmpty) {
  decode_error_value("{", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeMapStartUnterminate) {
  decode_error_value("}", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_FAILED);
}

TEST(FlJsonMessageCodecTest, DecodeMapNoComma) {
  decode_error_value("{\"zero\":0 \"one\":1}", FL_JSON_MESSAGE_CODEC_ERROR,
                     FL_JSON_MESSAGE_CODEC_ERROR_MISSING_COMMA);
}

TEST(FlJsonMessageCodecTest, DecodeMapNoColon) {
  decode_error_value("{\"zero\" 0,\"one\":1}", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_FAILED);
}

TEST(FlJsonMessageCodecTest, DecodeMapUnterminated) {
  decode_error_value("{\"zero\":0,\"one\":1", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_OUT_OF_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeMapDoubleTerminated) {
  decode_error_value("{\"zero\":0,\"one\":1}}", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_ADDITIONAL_DATA);
}

TEST(FlJsonMessageCodecTest, DecodeUnknownWord) {
  decode_error_value("foo", FL_MESSAGE_CODEC_ERROR,
                     FL_MESSAGE_CODEC_ERROR_FAILED);
}
