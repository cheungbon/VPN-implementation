#include <Windows.h>
#include <Ras.h>
#include <iostream>

#pragma comment(lib, "Rasapi32.lib")

using namespace std;

void OutputString(char *lpFmt, ...)
{

	char buff[1024] = "";
	va_list    arglist;
	va_start(arglist, lpFmt);
	vsnprintf(buff, sizeof buff, lpFmt, arglist);
	va_end(arglist);
	//OutputDebugString((CString)buff);
	//AfxGetApp()->GetMainWnd()->GetDlgItem(IDC_STATIC)->SetWindowTextW((CString)buff);
	//AfxGetApp()->GetMainWnd()->RedrawWindow();
	cout << lpFmt << endl;
}

bool CreateVPN(const LPCTSTR pszEntryName, const LPCTSTR pszServerName,
	const LPCTSTR pszUserName, const LPCTSTR pszPassWord)
{
	RASENTRY rasEntry;
	DWORD rasEntrySize, dwResult;

	ZeroMemory(&rasEntry, sizeof(rasEntry));
	rasEntrySize = sizeof(rasEntry);
	rasEntry.dwCountryCode = 86;
	rasEntry.dwCountryID = 86;
	rasEntry.dwDialExtraPercent = 75;
	rasEntry.dwDialExtraSampleSeconds = 120;
	rasEntry.dwDialMode = RASEDM_DialAll;
	rasEntry.dwType = RASET_Vpn;
	rasEntry.dwRedialCount = 30;
	rasEntry.dwRedialPause = 60;
	rasEntry.dwSize = sizeof(rasEntry);
	rasEntry.dwfOptions = RASEO_SwCompression | RASEO_RequireEncryptedPw | RASEO_RequireDataEncryption |
		RASEO_PreviewUserPw | RASEO_RemoteDefaultGateway;
	rasEntry.dwFramingProtocol = RASFP_Ppp;
	rasEntry.dwVpnStrategy = VS_PptpOnly;
	rasEntry.dwfNetProtocols = RASNP_Ip;
	rasEntry.dwEncryptionType = ET_Optional;    //可选加密
	rasEntry.dwHangUpExtraPercent = 10;
	rasEntry.dwHangUpExtraSampleSeconds = 120;
	lstrcpy(rasEntry.szLocalPhoneNumber, pszServerName);
	lstrcpy(rasEntry.szDeviceType, RASDT_Vpn);
	lstrcpy(rasEntry.szDeviceName, L"VPN");

	dwResult = RasValidateEntryName(NULL, pszEntryName);

	if (dwResult == ERROR_ALREADY_EXISTS) {
		RasDeleteEntry(NULL, pszEntryName);
	}

	dwResult = RasSetEntryProperties(NULL, pszEntryName, &rasEntry, sizeof(rasEntry), NULL, 0);
	if (dwResult != 0)
	{
		//AfxMessageBox(_T("error : SetEntryProperties"));
		return false;
	}

	RASDIALPARAMS rdParams;
	ZeroMemory(&rdParams, sizeof(RASDIALPARAMS));
	rdParams.dwSize = sizeof(RASDIALPARAMS);
	lstrcpy(rdParams.szEntryName, pszEntryName);
	lstrcpy(rdParams.szUserName, pszUserName);
	lstrcpy(rdParams.szPassword, pszPassWord);
	DWORD dwRet = RasSetEntryDialParams(NULL, &rdParams, FALSE);

	if (dwRet == 0)
		return true;
	else
		return false;

}

void WINAPI RasDialFunc(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError)
{
	char szRasString[256]; // Buffer for storing the error string

	if (dwError)  // Error occurred
	{
		RasGetErrorString((UINT)dwError, (LPWSTR)szRasString, 256);
		OutputString("Error: %d - %s\n", dwError, szRasString);
		//SetEvent(g_TerminalEvent);
		return;
	}

	// Map each of the states of RasDial() and display on the screen
	// the next state that RasDial() is entering
	switch (rasconnstate)
	{
		// Running States
	case RASCS_OpenPort:
		OutputString("Opening port...\n");
		break;
	case RASCS_PortOpened:
		OutputString("Port opened.\n");
		break;
	case RASCS_ConnectDevice:
		OutputString("Connecting device...\n");
		break;
	case RASCS_DeviceConnected:
		OutputString("Device connected.\n");
		break;
	case RASCS_AllDevicesConnected:
		OutputString("All devices connected.\n");
		break;
	case RASCS_Authenticate:
		OutputString("Authenticating...\n");
		break;
	case RASCS_AuthNotify:
		OutputString("Authentication notify.\n");
		break;
	case RASCS_AuthRetry:
		OutputString("Retrying authentication...\n");
		break;
	case RASCS_AuthCallback:
		OutputString("Authentication callback...\n");
		break;
	case RASCS_AuthChangePassword:
		OutputString("Change password...\n");
		break;
	case RASCS_AuthProject:
		OutputString("Projection phase started...\n");
		break;
	case RASCS_AuthLinkSpeed:
		OutputString("Negotiating speed...\n");
		break;
	case RASCS_AuthAck:
		OutputString("Authentication acknowledge...\n");
		break;
	case RASCS_ReAuthenticate:
		OutputString("Retrying Authentication...\n");
		break;
	case RASCS_Authenticated:
		OutputString("Authentication complete.\n");
		break;
	case RASCS_PrepareForCallback:
		OutputString("Preparing for callback...\n");
		break;
	case RASCS_WaitForModemReset:
		OutputString("Waiting for modem reset...\n");
		break;
	case RASCS_WaitForCallback:
		OutputString("Waiting for callback...\n");
		break;
	case RASCS_Projected:
		OutputString("Projection completed.\n");
		break;
#if (WINVER >= 0x400) 
	case RASCS_StartAuthentication:
		OutputString("Starting authentication...\n");
		break;
	case RASCS_CallbackComplete:
		OutputString("Callback complete.\n");
		break;
	case RASCS_LogonNetwork:
		OutputString("Logon to the network.\n");
		break;
#endif 
	case RASCS_SubEntryConnected:
		OutputString("Subentry connected.\n");
		break;
	case RASCS_SubEntryDisconnected:
		OutputString("Subentry disconnected.\n");
		break;

		// The RAS Paused States will not occur because
		// we did not use the RASDIALEXTENSIONS structure
		// to set the RDEOPT_PausedState option flag.

		// The Paused States are:

		// RASCS_RetryAuthentication:
		// RASCS_CallbackSetByCaller:
		// RASCS_PasswordExpired:

		// Terminal States
	case RASCS_Connected:
		OutputString("Connection completed.\n");
		OutputString("连接成功\n");

		//StartApp();
		//SetEvent(g_TerminalEvent);
		break;
	case RASCS_Disconnected:
		OutputString("Disconnecting...\n");
		//SetEvent(g_TerminalEvent);
		break;
	default:
		OutputString("Unknown Status = %d\n", rasconnstate);
		break;
	}
	//AfxGetApp()->GetMainWnd()->Invalidate();
}

bool ConnectVPN()
{
	RASDIALPARAMS    RasDialParams;
	HRASCONN        hRasConn;
	DWORD            Ret;
	WCHAR			c_VPNDescription[256];
	WCHAR			c_VPNServer[256];
	WCHAR			c_VPNUserName[256];
	WCHAR			c_VPNPassword[256];

	lstrcpy(c_VPNDescription, L"VPN2");
	lstrcpy(c_VPNServer, L"xxx");
	lstrcpy(c_VPNUserName, L"vpn3");
	lstrcpy(c_VPNPassword, L"xxx");

	if (!CreateVPN(c_VPNDescription, c_VPNServer, c_VPNUserName, c_VPNPassword))
	{
		//AfxMessageBox(_T("VPN创建失败"));
		return false;
	}

	ZeroMemory(&RasDialParams, sizeof(RASDIALPARAMS));
	RasDialParams.dwSize = sizeof(RASDIALPARAMS);
	lstrcpy(RasDialParams.szEntryName, c_VPNDescription);
	lstrcpy(RasDialParams.szPhoneNumber, c_VPNServer);
	lstrcpy(RasDialParams.szUserName, c_VPNUserName);
	lstrcpy(RasDialParams.szPassword, c_VPNPassword);

	hRasConn = NULL;
	while (hRasConn == NULL) {
		Ret = RasDial(NULL, NULL, &RasDialParams, 0, &RasDialFunc, &hRasConn);
		if (Ret != 0)
		{
			OutputString("RasDial %s failed with error %d\n", RasDialParams.szPhoneNumber, Ret);
			RasDeleteEntry(NULL, RasDialParams.szEntryName);
			return false;
		}
		Sleep(2000);
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (ConnectVPN()) {
		cout << "";
	}
}