#include "main.h"



LRESULT __stdcall WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) 
{
	switch (Msg) {
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN:
		LbuttonIsDown = true;
		GetCursorPos(&WindowClick);
		GetWindowRect(*ProjectorHwnd, &PrevWindowPos);
		break;

	case WM_LBUTTONUP:
		LbuttonIsDown = false;
		NewLbuttonUpEvent = true;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case ID_REPORT_KNAPP:

				// Skriv ut rapport til HTML fil.
				std::string rapportsti = getenv("USERPROFILE");
				rapportsti.append("\\desktop\\BatteryTestResult.html");
				std::ofstream rapportfil(rapportsti);
					
				rapportfil << 
					"<!doctype html><html><head>" << 
					"<title>BatteryTest - Report</title>" << 
					"</head><body>" << 
					"<style>" << 
					"BODY { font-family:sans-serif; font-size:16px; }" <<
					"</style>" <<
					BatteriRapport() << 
					"</body></html>";
					
				rapportfil.close();

				break;
		}
		break;

	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	return 0;
}



int __stdcall wWinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	PWSTR lpCmdLine, 
	int nCmdShow
) {
	WNDCLASSEX wc;
	HWND hWnd;
	MSG Msg;

	wc.cbSize = sizeof(WNDCLASSEX);	
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON));
	wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;	
	wc.lpszClassName = L"MainClass";

	if (!RegisterClassEx(&wc)) {
		MessageBox(
			NULL, 
			L"Error registering window.", 
			L"Oisann!", 
			MB_OK | MB_ICONERROR
		);	
		exit(EXIT_FAILURE);
	}

	hWnd = CreateWindowEx(
		WS_EX_LAYERED,
		L"MainClass",
		L"BatteryTest BETA",
		WS_POPUPWINDOW | WS_CAPTION,
		(GetSystemMetrics(SM_CXSCREEN) - WinX)/2, /* int X position. */
		(GetSystemMetrics(SM_CYSCREEN) - WinY)/2, /* int Y position. */
		WinX, /* int Width. */
		WinY, /* int Height. */
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd) {
		MessageBox(
			NULL, 
			L"Error during creating window.", 
			L"Oisann!", 
			MB_OK | MB_ICONERROR
		);
		exit(EXIT_FAILURE);
	}

	// Knapp for å sende rapport.
	ReportButtonHwnd = CreateWindowExW(
		0,
		L"STATIC",
		L"",
		SS_BITMAP | WS_VISIBLE | WS_CHILD | SS_NOTIFY,
		650, 390, 41, 41,
		hWnd,
		(HMENU)ID_REPORT_KNAPP,
		hInstance,
		0
	);
	/*ReportButtonImg = LoadImageW(
		GetModuleHandle(0),
		MAKEINTRESOURCE(ID_REPORTBUTTON_IMG),
		IMAGE_BITMAP,
		0,
		0,
		LR_DEFAULTCOLOR
	);*/
	ReportButtonImg = LoadBitmapW(hInstance, MAKEINTRESOURCE(ID_REPORTBUTTON_IMG));

	// Trenger kun sette hånd-peker 1 gang for alle knapper,
	// da det ser ut som den påvirker standardklassen globalt.
	// GCL_HCURSOR hvis 32-bit, GCLP_ hvis 64-bit.
	SetClassLongPtrW(ReportButtonHwnd, GCL_HCURSOR, (long long)LoadCursorW(0, IDC_HAND));

	// Tooltip for rapportknapp.
	ReportButtonTooltip = CreateWindowExW(
		WS_EX_TOPMOST, 
		TOOLTIPS_CLASSW, 
		NULL,
		WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hWnd, 
		NULL, 
		hInstance,
		NULL
	);
	TOOLINFOW tf = {0};
	tf.cbSize = TTTOOLINFOW_V1_SIZE;
	tf.hwnd = hWnd;			// dialogboks eller hovedvindu.
	tf.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	tf.uId = (UINT_PTR)ReportButtonHwnd;	// HWND til kontroll tooltip skal knyttes til.
	tf.lpszText = (wchar_t*)L"Save current results to desktop (test continues).";
	if (SendMessage(ReportButtonTooltip, TTM_ADDTOOLW, 0, (LPARAM)&tf) == 1)
		SendMessage(ReportButtonTooltip, TTM_ACTIVATE, 1, 0);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	ProjectorHwnd = &hWnd;

	//
	// Forsyner prosjektoren vår med gjennomsiktigheten den trenger.
	// Vindustiler satt via CreateWindowEx() tar seg av det funksjonelle.
	// WS_POPUPWINDOW forsyner en viss ramme.
	// WS_EX_LAYERED er krevd av funksjonen under.
	// WS_EX_TRANSPARENT gjør at vi kan samhandle med spillet.
	//
	SetLayeredWindowAttributes(hWnd, RGB(238, 238, 238), NULL, LWA_COLORKEY);

	// Start cpu stressetråd som sjekker mot RunningOnBattery.
    std::thread cpustress_t1(CpuStress);
    std::thread cpustress_t2(CpuStress);
    std::thread cpustress_t3(CpuStress);
    std::thread cpustress_t4(CpuStress);
	std::thread cpustress_t5(CpuStress);
    std::thread cpustress_t6(CpuStress);
    std::thread cpustress_t7(CpuStress);
    std::thread cpustress_t8(CpuStress);
    cpustress_t1.detach();
    cpustress_t2.detach();
    cpustress_t3.detach();
    cpustress_t4.detach();
	cpustress_t5.detach();
    cpustress_t6.detach();
    cpustress_t7.detach();
    cpustress_t8.detach();

	// Starter batterioppdateringstråd.
	std::thread batterymain(BatteryUpdateMain);
	batterymain.detach();

	// Starter hovedloop.
	std::thread _mainloop(mainloop);
	_mainloop.detach();

	// Message loop for vindu.
	while (GetMessage(&Msg, nullptr, 0, 0)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return (int)Msg.wParam;
}