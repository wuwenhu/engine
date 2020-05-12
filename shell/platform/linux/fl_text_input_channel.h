// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_TEXT_INPUT_LINUX_FL_TEXT_INPUT_CHANNEL_H_
#define FLUTTER_SHELL_TEXT_INPUT_LINUX_FL_TEXT_INPUT_CHANNEL_H_

#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_channel.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlTextInputChannel,
                     fl_text_input_channel,
                     FL,
                     TEXT_INPUT_CHANNEL,
                     FlMethodChannel);

/**
 * FlTextInputChannel:
 *
 * #FlTextInputChannel is a text_input channel that implements the shell side
 * of TextInputChannels.textInput from the Flutter services library.
 */

typedef enum {
  FL_TEXT_AFFINITY_DOWNSTREAM,
  FL_TEXT_AFFINITY_UPSTREAM,
} FlTextAffinity;

typedef enum {
  FL_TEXT_INPUT_ACTION_CONTINUE,
  FL_TEXT_INPUT_ACTION_DONE,
  FL_TEXT_INPUT_ACTION_EMERGENCY_CALL,
  FL_TEXT_INPUT_ACTION_GO,
  FL_TEXT_INPUT_ACTION_JOIN,
  FL_TEXT_INPUT_ACTION_NEWLINE,
  FL_TEXT_INPUT_ACTION_NEXT,
  FL_TEXT_INPUT_ACTION_PREVIOUS,
  FL_TEXT_INPUT_ACTION_ROUTE,
  FL_TEXT_INPUT_ACTION_SEARCH,
  FL_TEXT_INPUT_ACTION_SEND,
  FL_TEXT_INPUT_ACTION_UNSPECIFIED,
} FlTextInputAction;

typedef void (*FlTextInputChannelSetClientHandler)(FlTextInputChannel* channel,
                                                   int64_t client_id,
                                                   const gchar*,
                                                   gpointer user_data);
typedef void (*FlTextInputChannelShowHandler)(FlTextInputChannel* channel,
                                              gpointer user_data);
typedef void (*FlTextInputChannelSetEditingStateHandler)(
    FlTextInputChannel* channel,
    const gchar* text,
    int64_t selection_base,
    int64_t selection_extent,
    FlTextAffinity selection_affinity,
    gboolean selection_is_directional,
    int64_t composing_base,
    int64_t composing_extent,
    gpointer user_data);
typedef void (*FlTextInputChannelClearClientHandler)(
    FlTextInputChannel* channel,
    gpointer user_data);
typedef void (*FlTextInputChannelHideHandler)(FlTextInputChannel* channel,
                                              gpointer user_data);

/**
 * fl_text_input_channel_new:
 * @messenger: an #FlBinaryMessenger
 *
 * Creates a new text_input channel.
 *
 * Returns: a new #FlTextInputChannel
 */
FlTextInputChannel* fl_text_input_channel_new(
    FlBinaryMessenger* messenger,
    FlTextInputChannelSetClientHandler set_client_handler,
    FlTextInputChannelShowHandler show_handler,
    FlTextInputChannelSetEditingStateHandler set_editing_state_handler,
    FlTextInputChannelClearClientHandler clear_client_handler,
    FlTextInputChannelHideHandler hide_handler,
    gpointer user_data);

void text_input_channel_request_existing_input_state(
    FlTextInputChannel* self,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

FlValue* text_input_channel_request_existing_input_state_finish(
    FlTextInputChannel* self,
    GAsyncResult* result,
    GError** error);

void text_input_channel_update_editing_state(FlTextInputChannel* self,
                                             int64_t client_id,
                                             const gchar* text,
                                             int64_t selection_base,
                                             int64_t selection_extent,
                                             FlTextAffinity selection_affinity,
                                             gboolean selection_is_directional,
                                             int64_t composing_base,
                                             int64_t composing_extent);

void text_input_channel_perform_action(FlTextInputChannel* self,
                                       int64_t client_id,
                                       FlTextInputAction action);

void text_input_channel_on_connection_closed(FlTextInputChannel* self,
                                             int64_t client_id);

G_END_DECLS

#endif  // FLUTTER_SHELL_TEXT_INPUT_LINUX_FL_TEXT_INPUT_CHANNEL_H_
