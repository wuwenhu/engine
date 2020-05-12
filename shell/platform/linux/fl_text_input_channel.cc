// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_text_input_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_json_method_codec.h"

struct _FlTextInputChannel {
  FlMethodChannel parent_instance;

  FlTextInputChannelSetClientHandler set_client_handler;
  FlTextInputChannelShowHandler show_handler;
  FlTextInputChannelSetEditingStateHandler set_editing_state_handler;
  FlTextInputChannelClearClientHandler clear_client_handler;
  FlTextInputChannelHideHandler hide_handler;
  gpointer handler_data;
};

G_DEFINE_TYPE(FlTextInputChannel,
              fl_text_input_channel,
              fl_method_channel_get_type())

static void method_call_cb(FlMethodChannel* channel,
                           const gchar* method,
                           FlValue* args,
                           FlMethodChannelResponseHandle* response_handle,
                           gpointer user_data) {
  FlTextInputChannel* self = FL_TEXT_INPUT_CHANNEL(user_data);

  if (strcmp(method, "TextInput.setClient") == 0) {
    if (self->set_client_handler != nullptr)
      self->set_client_handler(self, 0, "", self->handler_data);
    fl_method_channel_respond(channel, response_handle, nullptr, nullptr);
  } else if (strcmp(method, "TextInput.show") == 0) {
    if (self->show_handler != nullptr)
      self->show_handler(self, self->handler_data);
    fl_method_channel_respond(channel, response_handle, nullptr, nullptr);
  } else if (strcmp(method, "TextInput.setEditableSizeAndTransform") == 0) {
    fl_method_channel_respond_not_implemented(channel, response_handle,
                                              nullptr);
  } else if (strcmp(method, "TextInput.requestAutofill") == 0) {
    fl_method_channel_respond_not_implemented(channel, response_handle,
                                              nullptr);
  } else if (strcmp(method, "TextInput.setStyle") == 0) {
    fl_method_channel_respond_not_implemented(channel, response_handle,
                                              nullptr);
  } else if (strcmp(method, "TextInput.setEditingState") == 0) {
    const gchar* text =
        fl_value_get_string(fl_value_lookup_string(args, "text"));
    int64_t selection_base =
        fl_value_get_int(fl_value_lookup_string(args, "selectionBase"));
    int64_t selection_extent =
        fl_value_get_int(fl_value_lookup_string(args, "selectionExtent"));
    const gchar* selection_affinity_name =
        fl_value_get_string(fl_value_lookup_string(args, "selectionAffinity"));
    FlTextAffinity selection_affinity;
    if (g_strcmp0(selection_affinity_name, "TextAffinity.downstream") == 0)
      selection_affinity = FL_TEXT_AFFINITY_DOWNSTREAM;
    else if (g_strcmp0(selection_affinity_name, "TextAffinity.upstream") == 0)
      selection_affinity = FL_TEXT_AFFINITY_UPSTREAM;
    gboolean selection_is_directional = fl_value_get_bool(
        fl_value_lookup_string(args, "selectionIsDirectional"));
    int64_t composing_base =
        fl_value_get_int(fl_value_lookup_string(args, "composingBase"));
    int64_t composing_extent =
        fl_value_get_int(fl_value_lookup_string(args, "composingExtent"));

    if (self->set_editing_state_handler != nullptr)
      self->set_editing_state_handler(self, text, selection_base,
                                      selection_extent, selection_affinity,
                                      selection_is_directional, composing_base,
                                      composing_extent, self->handler_data);

    fl_method_channel_respond(channel, response_handle, nullptr, nullptr);
  } else if (strcmp(method, "TextInput.clearClient") == 0) {
    if (self->clear_client_handler != nullptr)
      self->clear_client_handler(self, self->handler_data);
    fl_method_channel_respond(channel, response_handle, nullptr, nullptr);
  } else if (strcmp(method, "TextInput.hide") == 0) {
    if (self->hide_handler != nullptr)
      self->hide_handler(self, self->handler_data);
    fl_method_channel_respond(channel, response_handle, nullptr, nullptr);
  } else
    fl_method_channel_respond_not_implemented(channel, response_handle,
                                              nullptr);
}

// Helper function to just check if a method returned an error
static gboolean finish_method(GObject* object,
                              GAsyncResult* result,
                              GError** error) {
  g_autoptr(FlMethodResponse) response = fl_method_channel_invoke_method_finish(
      FL_METHOD_CHANNEL(object), result, error);
  if (response == nullptr)
    return FALSE;
  return fl_method_response_get_result(response, error) != nullptr;
}

static void update_editing_state_response_cb(GObject* object,
                                             GAsyncResult* result,
                                             gpointer user_data) {
  g_autoptr(GError) error = nullptr;
  if (!finish_method(object, result, &error))
    g_warning("Failed to call TextInputClient.updateEditingState: %s",
              error->message);
}

static void perform_action_response_cb(GObject* object,
                                       GAsyncResult* result,
                                       gpointer user_data) {
  g_autoptr(GError) error = nullptr;
  if (!finish_method(object, result, &error))
    g_warning("Failed to call TextInputClient.performAction: %s",
              error->message);
}

static void fl_text_input_channel_class_init(FlTextInputChannelClass* klass) {}

static void fl_text_input_channel_init(FlTextInputChannel* self) {}

FlTextInputChannel* fl_text_input_channel_new(
    FlBinaryMessenger* messenger,
    FlTextInputChannelSetClientHandler set_client_handler,
    FlTextInputChannelShowHandler show_handler,
    FlTextInputChannelSetEditingStateHandler set_editing_state_handler,
    FlTextInputChannelClearClientHandler clear_client_handler,
    FlTextInputChannelHideHandler hide_handler,
    gpointer user_data) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);

  g_autoptr(FlJsonMethodCodec) codec = fl_json_method_codec_new();
  FlTextInputChannel* self = FL_TEXT_INPUT_CHANNEL(
      g_object_new(fl_text_input_channel_get_type(), "messenger", messenger,
                   "name", "flutter/textinput", "codec", codec, nullptr));
  fl_method_channel_set_method_call_handler(FL_METHOD_CHANNEL(self),
                                            method_call_cb, self);

  self->set_client_handler = set_client_handler;
  self->show_handler = show_handler;
  self->set_editing_state_handler = set_editing_state_handler;
  self->clear_client_handler = clear_client_handler;
  self->hide_handler = hide_handler;
  self->handler_data = user_data;

  return self;
}

void text_input_channel_request_existing_input_state(
    FlTextInputChannel* self,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data) {
  fl_method_channel_invoke_method(FL_METHOD_CHANNEL(self),
                                  "TextInputClient.requestExistingInputState",
                                  nullptr, cancellable, callback, user_data);
}

FlValue* text_input_channel_request_existing_input_state_finish(
    FlTextInputChannel* self,
    GAsyncResult* result,
    GError** error) {
  g_autoptr(FlMethodResponse) response = fl_method_channel_invoke_method_finish(
      FL_METHOD_CHANNEL(self), result, error);
  if (response == nullptr)
    return nullptr;
  return fl_method_response_get_result(response, error);
}

void text_input_channel_update_editing_state(FlTextInputChannel* self,
                                             int64_t client_id,
                                             const gchar* text,
                                             int64_t selection_base,
                                             int64_t selection_extent,
                                             FlTextAffinity selection_affinity,
                                             gboolean selection_is_directional,
                                             int64_t composing_base,
                                             int64_t composing_extent) {
  g_printerr("TextInput.updateEditingState(%" G_GINT64_FORMAT
             ", \"%s\", %" G_GINT64_FORMAT ", %" G_GINT64_FORMAT
             ", %d, %s, %" G_GINT64_FORMAT ", %" G_GINT64_FORMAT ")\n",
             client_id, text, selection_base, selection_extent,
             selection_affinity, selection_is_directional ? "true" : "false",
             composing_base, composing_extent);

  g_autoptr(FlValue) args = fl_value_new_list();
  fl_value_append_take(args, fl_value_new_int(client_id));
  g_autoptr(FlValue) value = fl_value_new_map();
  fl_value_set_string_take(value, "text", fl_value_new_string(text));
  fl_value_set_string_take(value, "selectionBase",
                           fl_value_new_int(selection_base));
  fl_value_set_string_take(value, "selectionExtent",
                           fl_value_new_int(selection_extent));
  const gchar* selection_affinity_name = "";
  switch (selection_affinity) {
    case FL_TEXT_AFFINITY_DOWNSTREAM:
      selection_affinity_name = "TextAffinity.downstream";
      break;
    case FL_TEXT_AFFINITY_UPSTREAM:
      selection_affinity_name = "TextAffinity.upstream";
      break;
  }
  fl_value_set_string_take(value, "selectionAffinity",
                           fl_value_new_string(selection_affinity_name));
  fl_value_set_string_take(value, "selectionIsDirectional",
                           fl_value_new_bool(selection_is_directional));
  fl_value_set_string_take(value, "composingBase",
                           fl_value_new_int(composing_base));
  fl_value_set_string_take(value, "composingExtent",
                           fl_value_new_int(composing_extent));
  fl_value_append(args, value);

  fl_method_channel_invoke_method(
      FL_METHOD_CHANNEL(self), "TextInputClient.updateEditingState", args,
      nullptr, update_editing_state_response_cb, self);
}

void text_input_channel_perform_action(FlTextInputChannel* self,
                                       int64_t client_id,
                                       FlTextInputAction action) {
  g_autoptr(FlValue) args = fl_value_new_list();
  fl_value_append_take(args, fl_value_new_int(client_id));
  const gchar* action_name = "";
  switch (action) {
    case FL_TEXT_INPUT_ACTION_CONTINUE:
      action_name = "TextInputAction.continueAction";
      break;
    case FL_TEXT_INPUT_ACTION_DONE:
      action_name = "TextInputAction.done";
      break;
    case FL_TEXT_INPUT_ACTION_EMERGENCY_CALL:
      action_name = "TextInputAction.emergencyCall";
      break;
    case FL_TEXT_INPUT_ACTION_GO:
      action_name = "TextInputAction.go";
      break;
    case FL_TEXT_INPUT_ACTION_JOIN:
      action_name = "TextInputAction.join";
      break;
    case FL_TEXT_INPUT_ACTION_NEWLINE:
      action_name = "TextInputAction.newline";
      break;
    case FL_TEXT_INPUT_ACTION_NEXT:
      action_name = "TextInputAction.next";
      break;
    case FL_TEXT_INPUT_ACTION_PREVIOUS:
      action_name = "TextInputAction.previous";
      break;
    case FL_TEXT_INPUT_ACTION_ROUTE:
      action_name = "TextInputAction.route";
      break;
    case FL_TEXT_INPUT_ACTION_SEARCH:
      action_name = "TextInputAction.search";
      break;
    case FL_TEXT_INPUT_ACTION_SEND:
      action_name = "TextInputAction.send";
      break;
    case FL_TEXT_INPUT_ACTION_UNSPECIFIED:
      action_name = "TextInputAction.unspecified";
      break;
  }
  fl_value_append_take(args, fl_value_new_string(action_name));

  fl_method_channel_invoke_method(FL_METHOD_CHANNEL(self),
                                  "TextInputClient.performAction", args,
                                  nullptr, perform_action_response_cb, self);
}

void text_input_channel_on_connection_closed(FlTextInputChannel* self,
                                             int64_t client_id) {
  g_autoptr(FlValue) args = fl_value_new_list();
  fl_value_append_take(args, fl_value_new_int(client_id));
  fl_method_channel_invoke_method(FL_METHOD_CHANNEL(self),
                                  "TextInputClient.onConnectionClosed", args,
                                  nullptr, nullptr, nullptr);
}
