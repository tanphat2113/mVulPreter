void CreateSession::ExecutePost(Response* const response) {
  DictionaryValue *capabilities = NULL;
  if (!GetDictionaryParameter("desiredCapabilities", &capabilities)) {
    response->SetError(new Error(
        kBadRequest, "Missing or invalid 'desiredCapabilities'"));
    return;
  }

  CommandLine command_line_options(CommandLine::NO_PROGRAM);
  ListValue* switches = NULL;
  const char* kCustomSwitchesKey = "chrome.switches";
  if (capabilities->GetListWithoutPathExpansion(kCustomSwitchesKey,
                                                &switches)) {
    for (size_t i = 0; i < switches->GetSize(); ++i) {
      std::string switch_string;
      if (!switches->GetString(i, &switch_string)) {
        response->SetError(new Error(
            kBadRequest, "Custom switch is not a string"));
        return;
      }
      size_t separator_index = switch_string.find("=");
      if (separator_index != std::string::npos) {
        CommandLine::StringType switch_string_native;
        if (!switches->GetString(i, &switch_string_native)) {
          response->SetError(new Error(
              kBadRequest, "Custom switch is not a string"));
          return;
        }
        command_line_options.AppendSwitchNative(
            switch_string.substr(0, separator_index),
            switch_string_native.substr(separator_index + 1));
      } else {
        command_line_options.AppendSwitch(switch_string);
      }
    }
  } else if (capabilities->HasKey(kCustomSwitchesKey)) {
    response->SetError(new Error(
         kBadRequest, "Custom switches must be a list"));
     return;
   }
  Value* verbose_value;
  if (capabilities->GetWithoutPathExpansion("chrome.verbose", &verbose_value)) {
    bool verbose;
    if (verbose_value->GetAsBoolean(&verbose) && verbose) {
      // Since logging is shared among sessions, if any session requests verbose
      // logging, verbose logging will be enabled for all sessions. It is not
      // possible to turn it off.
      logging::SetMinLogLevel(logging::LOG_INFO);
    } else {
      response->SetError(new Error(
          kBadRequest, "verbose must be a boolean true or false"));
      return;
    }
  }
 
   FilePath browser_exe;
   FilePath::StringType path;
  if (capabilities->GetStringWithoutPathExpansion("chrome.binary", &path))
    browser_exe = FilePath(path);

  Session* session = new Session();
  Error* error = session->Init(browser_exe, command_line_options);
  if (error) {
    response->SetError(error);
    return;
  }

  bool native_events_required = false;
  Value* native_events_value = NULL;
  if (capabilities->GetWithoutPathExpansion(
          "chrome.nativeEvents", &native_events_value)) {
    if (native_events_value->GetAsBoolean(&native_events_required)) {
      session->set_use_native_events(native_events_required);
    }
  }
  bool screenshot_on_error = false;
  if (capabilities->GetBoolean(
          "takeScreenshotOnError", &screenshot_on_error)) {
     session->set_screenshot_on_error(screenshot_on_error);
   }
 
  LOG(INFO) << "Created session " << session->id();
   std::ostringstream stream;
   SessionManager* session_manager = SessionManager::GetInstance();
   stream << "http://" << session_manager->GetAddress() << "/session/"
         << session->id();
  response->SetStatus(kSeeOther);
  response->SetValue(Value::CreateStringValue(stream.str()));
}
