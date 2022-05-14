void PrintPreviewDataSource::Init() {
  AddLocalizedString("title", IDS_PRINT_PREVIEW_TITLE);
  AddLocalizedString("loading", IDS_PRINT_PREVIEW_LOADING);
  AddLocalizedString("noPlugin", IDS_PRINT_PREVIEW_NO_PLUGIN);
  AddLocalizedString("launchNativeDialog", IDS_PRINT_PREVIEW_NATIVE_DIALOG);
  AddLocalizedString("previewFailed", IDS_PRINT_PREVIEW_FAILED);
  AddLocalizedString("invalidPrinterSettings",
                     IDS_PRINT_PREVIEW_INVALID_PRINTER_SETTINGS);
  AddLocalizedString("printButton", IDS_PRINT_PREVIEW_PRINT_BUTTON);
  AddLocalizedString("saveButton", IDS_PRINT_PREVIEW_SAVE_BUTTON);
  AddLocalizedString("cancelButton", IDS_PRINT_PREVIEW_CANCEL_BUTTON);
  AddLocalizedString("printing", IDS_PRINT_PREVIEW_PRINTING);
  AddLocalizedString("printingToPDFInProgress",
                     IDS_PRINT_PREVIEW_PRINTING_TO_PDF_IN_PROGRESS);
#if defined(OS_MACOSX)
  AddLocalizedString("openingPDFInPreview",
                     IDS_PRINT_PREVIEW_OPENING_PDF_IN_PREVIEW);
#endif
  AddLocalizedString("destinationLabel", IDS_PRINT_PREVIEW_DESTINATION_LABEL);
  AddLocalizedString("copiesLabel", IDS_PRINT_PREVIEW_COPIES_LABEL);
  AddLocalizedString("examplePageRangeText",
                     IDS_PRINT_PREVIEW_EXAMPLE_PAGE_RANGE_TEXT);
  AddLocalizedString("layoutLabel", IDS_PRINT_PREVIEW_LAYOUT_LABEL);
  AddLocalizedString("optionAllPages", IDS_PRINT_PREVIEW_OPTION_ALL_PAGES);
  AddLocalizedString("optionBw", IDS_PRINT_PREVIEW_OPTION_BW);
  AddLocalizedString("optionCollate", IDS_PRINT_PREVIEW_OPTION_COLLATE);
  AddLocalizedString("optionColor", IDS_PRINT_PREVIEW_OPTION_COLOR);
  AddLocalizedString("optionLandscape", IDS_PRINT_PREVIEW_OPTION_LANDSCAPE);
  AddLocalizedString("optionPortrait", IDS_PRINT_PREVIEW_OPTION_PORTRAIT);
  AddLocalizedString("optionTwoSided", IDS_PRINT_PREVIEW_OPTION_TWO_SIDED);
  AddLocalizedString("pagesLabel", IDS_PRINT_PREVIEW_PAGES_LABEL);
  AddLocalizedString("pageRangeTextBox", IDS_PRINT_PREVIEW_PAGE_RANGE_TEXT);
  AddLocalizedString("pageRangeRadio", IDS_PRINT_PREVIEW_PAGE_RANGE_RADIO);
  AddLocalizedString("printToPDF", IDS_PRINT_PREVIEW_PRINT_TO_PDF);
  AddLocalizedString("printPreviewTitleFormat", IDS_PRINT_PREVIEW_TITLE_FORMAT);
  AddLocalizedString("printPreviewSummaryFormatShort",
                     IDS_PRINT_PREVIEW_SUMMARY_FORMAT_SHORT);
  AddLocalizedString("printPreviewSummaryFormatLong",
                     IDS_PRINT_PREVIEW_SUMMARY_FORMAT_LONG);
  AddLocalizedString("printPreviewSheetsLabelSingular",
                     IDS_PRINT_PREVIEW_SHEETS_LABEL_SINGULAR);
  AddLocalizedString("printPreviewSheetsLabelPlural",
                     IDS_PRINT_PREVIEW_SHEETS_LABEL_PLURAL);
  AddLocalizedString("printPreviewPageLabelSingular",
                     IDS_PRINT_PREVIEW_PAGE_LABEL_SINGULAR);
  AddLocalizedString("printPreviewPageLabelPlural",
                     IDS_PRINT_PREVIEW_PAGE_LABEL_PLURAL);
  const string16 shortcut_text(UTF8ToUTF16(kAdvancedPrintShortcut));
#if defined(OS_CHROMEOS)
  AddString("cloudPrintDialogOption", l10n_util::GetStringFUTF16(
      IDS_PRINT_PREVIEW_CLOUD_DIALOG_OPTION,
      l10n_util::GetStringUTF16(IDS_GOOGLE_CLOUD_PRINT),
      shortcut_text));
  AddLocalizedString("printWithCloudPrint",
                     IDS_PRINT_PREVIEW_MORE_PRINTERS);
#else
  AddString("systemDialogOption", l10n_util::GetStringFUTF16(
      IDS_PRINT_PREVIEW_SYSTEM_DIALOG_OPTION,
      shortcut_text));
  AddString("printWithCloudPrint", l10n_util::GetStringFUTF16(
      IDS_PRINT_PREVIEW_PRINT_WITH_CLOUD_PRINT,
      l10n_util::GetStringUTF16(IDS_GOOGLE_CLOUD_PRINT)));
#endif
#if defined(OS_MACOSX)
  AddLocalizedString("openPdfInPreviewOption",
                     IDS_PRINT_PREVIEW_OPEN_PDF_IN_PREVIEW_APP);
#endif
  AddString("printWithCloudPrintWait", l10n_util::GetStringFUTF16(
      IDS_PRINT_PREVIEW_PRINT_WITH_CLOUD_PRINT_WAIT,
      l10n_util::GetStringUTF16(IDS_GOOGLE_CLOUD_PRINT)));
  AddLocalizedString("pageRangeInstruction",
                     IDS_PRINT_PREVIEW_PAGE_RANGE_INSTRUCTION);
  AddLocalizedString("copiesInstruction", IDS_PRINT_PREVIEW_COPIES_INSTRUCTION);
  AddLocalizedString("signIn", IDS_PRINT_PREVIEW_SIGN_IN);
  AddLocalizedString("managePrinters", IDS_PRINT_PREVIEW_MANAGE_PRINTERS);
  AddLocalizedString("incrementTitle", IDS_PRINT_PREVIEW_INCREMENT_TITLE);
  AddLocalizedString("decrementTitle", IDS_PRINT_PREVIEW_DECREMENT_TITLE);
  AddLocalizedString("printPagesLabel", IDS_PRINT_PREVIEW_PRINT_PAGES_LABEL);
   AddLocalizedString("optionsLabel", IDS_PRINT_PREVIEW_OPTIONS_LABEL);
   AddLocalizedString("optionHeaderFooter",
                      IDS_PRINT_PREVIEW_OPTION_HEADER_FOOTER);
  AddLocalizedString("optionFitToPage",
                     IDS_PRINT_PREVIEW_OPTION_FIT_TO_PAGE);
   AddLocalizedString("marginsLabel", IDS_PRINT_PREVIEW_MARGINS_LABEL);
   AddLocalizedString("defaultMargins", IDS_PRINT_PREVIEW_DEFAULT_MARGINS);
   AddLocalizedString("noMargins", IDS_PRINT_PREVIEW_NO_MARGINS);
  AddLocalizedString("customMargins", IDS_PRINT_PREVIEW_CUSTOM_MARGINS);
  AddLocalizedString("minimumMargins", IDS_PRINT_PREVIEW_MINIMUM_MARGINS);
  AddLocalizedString("top", IDS_PRINT_PREVIEW_TOP_MARGIN_LABEL);
  AddLocalizedString("bottom", IDS_PRINT_PREVIEW_BOTTOM_MARGIN_LABEL);
  AddLocalizedString("left", IDS_PRINT_PREVIEW_LEFT_MARGIN_LABEL);
  AddLocalizedString("right", IDS_PRINT_PREVIEW_RIGHT_MARGIN_LABEL);

  set_json_path("strings.js");
  add_resource_path("print_preview.js", IDR_PRINT_PREVIEW_JS);
  set_default_resource(IDR_PRINT_PREVIEW_HTML);
}
