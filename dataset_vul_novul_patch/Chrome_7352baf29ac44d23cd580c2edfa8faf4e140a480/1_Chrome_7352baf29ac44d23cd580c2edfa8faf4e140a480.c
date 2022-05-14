void SignatureUtil::CheckSignature(
    const FilePath& file_path,
    ClientDownloadRequest_SignatureInfo* signature_info) {
  VLOG(2) << "Checking signature for " << file_path.value();

  WINTRUST_FILE_INFO file_info;
  file_info.cbStruct = sizeof(file_info);
  file_info.pcwszFilePath = file_path.value().c_str();
  file_info.hFile = NULL;
  file_info.pgKnownSubject = NULL;

  WINTRUST_DATA wintrust_data;
  wintrust_data.cbStruct = sizeof(wintrust_data);
  wintrust_data.pPolicyCallbackData = NULL;
  wintrust_data.pSIPClientData = NULL;
  wintrust_data.dwUIChoice = WTD_UI_NONE;
  wintrust_data.fdwRevocationChecks = WTD_REVOKE_NONE;
  wintrust_data.dwUnionChoice = WTD_CHOICE_FILE;
  wintrust_data.pFile = &file_info;
  wintrust_data.dwStateAction = WTD_STATEACTION_VERIFY;
  wintrust_data.hWVTStateData = NULL;
  wintrust_data.pwszURLReference = NULL;
  wintrust_data.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL;
  wintrust_data.dwUIContext = WTD_UICONTEXT_EXECUTE;

  GUID policy_guid = WINTRUST_ACTION_GENERIC_VERIFY_V2;

  LONG result = WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE),
                               &policy_guid,
                               &wintrust_data);

  CRYPT_PROVIDER_DATA* prov_data = WTHelperProvDataFromStateData(
      wintrust_data.hWVTStateData);
  if (prov_data) {
    if (prov_data->csSigners > 0) {
      signature_info->set_trusted(result == ERROR_SUCCESS);
    }
     for (DWORD i = 0; i < prov_data->csSigners; ++i) {
       const CERT_CHAIN_CONTEXT* cert_chain_context =
           prov_data->pasSigners[i].pChainContext;
       for (DWORD j = 0; j < cert_chain_context->cChain; ++j) {
         CERT_SIMPLE_CHAIN* simple_chain = cert_chain_context->rgpChain[j];
         ClientDownloadRequest_CertificateChain* chain =
             signature_info->add_certificate_chain();
         for (DWORD k = 0; k < simple_chain->cElement; ++k) {
           CERT_CHAIN_ELEMENT* element = simple_chain->rgpElement[k];
           chain->add_element()->set_certificate(
              element->pCertContext->pbCertEncoded,
              element->pCertContext->cbCertEncoded);
        }
      }
    }

    wintrust_data.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE),
                   &policy_guid, &wintrust_data);
  }
}
