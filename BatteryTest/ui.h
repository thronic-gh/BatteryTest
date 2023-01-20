#pragma once

//
//	Hjelpefunksjoner til projisering.
//
HFONT font;

void ProjectText(
	int Size, 
	bool Bold, 
	bool Italic, 
	bool Underline, 
	LPCWSTR Font, 
	int x, int y, 
	COLORREF c, 
	std::wstring s, 
	HDC* dc, 
	int Alignment, 
	bool Transparent
){
	font = CreateFont(
		Size,			// Høyde.
		0,			// Bredde.
		0, 
		0, 
		(Bold ? 700 : 400),		// 0-1000. 400=Normal, 700=bold.
		Italic,			// Italic.
		Underline,		// Underline.
		FALSE,			// Strikeout.
		DEFAULT_CHARSET,	// Charset.
		OUT_OUTLINE_PRECIS,	
		CLIP_DEFAULT_PRECIS, 
		CLEARTYPE_QUALITY,	// Kvalitet.
		VARIABLE_PITCH,	
		Font			// Font.
	);
	
	SetTextAlign(*dc, Alignment | TA_NOUPDATECP);
	SetTextColor(*dc, c);
	HGDIOBJ prevObj = SelectObject(*dc, font);
	SetBkColor(*dc, RGB(32,32,32));
	SetBkMode(*dc, (Transparent? TRANSPARENT : OPAQUE));
	TextOutW(*dc, x, y, s.c_str(), (int)s.length());
	SelectObject(*dc, prevObj);
	DeleteObject(font);
}

//
//	Oppdaterer projisering.
//
HWND* ProjectorHwnd;
RECT ClientRect;
RECT WindowRect;
HDC hdc, BufferDC;
HBRUSH BgBrush;
HBITMAP BufferBitmap;
COLORREF Red = RGB(255, 0, 0);
COLORREF Black = RGB(0, 0, 0);
COLORREF Blue = RGB(0, 0, 255);
COLORREF ActiveGreen = RGB(0, 128, 0);
COLORREF SuperGreen = RGB(0, 255, 0);
COLORREF CTBlue = RGB(102, 178, 255);
COLORREF White = RGB(224, 224, 224);
COLORREF TrueWhite = RGB(255,255,255);

void UpdateProjection()
{
	try {
	//
	//	Hent kontekstkoordinater rett fra spillvindu.
	//	HUSK: GetClientRect() = Vinduoppløsning. 
	//	      GetWindowRect() = Vinduplassering.
	//	
	//	Derfor bruker vi client til å oppdatere bakgrunn med.
	//
	GetClientRect(*ProjectorHwnd, &ClientRect);
	int cx = ClientRect.right;
	int cy = ClientRect.bottom;

	//
	//	Opprett mellomlager for tegning.
	//	Vi lar front-HDC være i fred mens vi tegner på et mellomlager
	//	og flipper over dette når vi er ferdig - kun én oppdatering. 
	//
	if ((hdc = GetDC(*ProjectorHwnd)) == NULL) {
		GetError(L"GetDC failed.");
		exit(EXIT_FAILURE);
	}

	if ((BufferDC = CreateCompatibleDC(hdc)) == NULL) {
		GetError(L"CreateCompatibleDC failed.");
		exit(EXIT_FAILURE);
	}

	/*
	if ((BufferBitmap = CreateCompatibleBitmap(hdc, cx, cy)) == NULL) {
		GetError(L"CreateCompatibleBitmap failed.");
		exit(EXIT_FAILURE);
	}
	*/
	
	if ((BufferBitmap = (HBITMAP)LoadImage(
		GetModuleHandle(NULL), 
		MAKEINTRESOURCE(BG_IMAGE),
		IMAGE_BITMAP,
		0,
		0,
		LR_DEFAULTCOLOR
	)) == NULL) {
		GetError(L"BufferBitmap failed.");
		exit(EXIT_FAILURE);
	}

	if (SelectObject(BufferDC, BufferBitmap) == NULL) {
		GetError(L"SelectObject failed.");
		exit(EXIT_FAILURE);
	}

	//
	//	FOR OVERLEGG (Ref. CSS Prosjektor/ESP).
	//	Overskriv gjeldende bakgrunn for å unngå artifakter fra 
	//	forrige bilde. Vi oppnår gj.siktighet med samme farge 
	//	vi brukte som parameter i SetLayeredWindowAttributes (WinMain).
	//
	//BgBrush = CreateSolidBrush(TrueWhite);
	//FillRect(BufferDC, &ClientRect, BgBrush);
	//DeleteObject(BgBrush);

	// Finnes det noe statustekst å skrive?
	int _TextStart = 50;
	int _TextStep = 15;
	 
	for (size_t a=0; a<statustekster.size(); a++) {
		if (
			statustekster[a].find(L"Battery SHOULD") != std::wstring::npos || 
			statustekster[a].find(L"Battery seems to ACTUALLY") != std::wstring::npos || 
			statustekster[a].find(L"Test ended") != std::wstring::npos || 
			statustekster[a].find(L"Charge battery and wait") != std::wstring::npos || 
			statustekster[a].find(L"Disconnect charger") != std::wstring::npos || 
			statustekster[a].find(L"Charge until") != std::wstring::npos
		) {
			ProjectText(
				14, true, false, false, L"Verdana", 30, _TextStart + (a * _TextStep), Blue, 
				statustekster[a], 
				&BufferDC, TA_LEFT, true
			);
		} else if (
			statustekster[a].find(L"Avoid temporary usage") != std::wstring::npos || 
			statustekster[a].find(L"If the discharge rate spike often") != std::wstring::npos
		) {
			ProjectText(
				14, true, true, false, L"Verdana", 30, _TextStart + (a * _TextStep), Black, 
				statustekster[a], 
				&BufferDC, TA_LEFT, true
			);
		} else {
			ProjectText(
				14, false, false, false, L"Verdana", 30, _TextStart + (a * _TextStep), Black, 
				statustekster[a], 
				&BufferDC, TA_LEFT, true
			);
		}
	}

	// Tegn kapasitetssøylene.
	RECT MaxByDesign, MaxCurrently;
	
	MaxByDesign.left = 560;
	MaxByDesign.right = 650;
	MaxByDesign.bottom = 300;
	MaxByDesign.top = 300 - ((MaxCapByDesignPerc * 2) + 1);

	MaxCurrently.left = 690;
	MaxCurrently.right = 780;
	MaxCurrently.bottom = 300;
	MaxCurrently.top = 300 - ((MaxCapCurrentlyPerc * 2) + 1);;

	BgBrush = CreateSolidBrush(SuperGreen);
	FillRect(BufferDC, &MaxByDesign, BgBrush);
	FillRect(BufferDC, &MaxCurrently, BgBrush);
	DeleteObject(BgBrush);

	// Skriv tekst til søylene.
	ProjectText(
		14, true, false, false, L"Verdana", 560, 320, CTBlue, 
		std::to_wstring(MaxCapByDesignPerc) + L" %", 
		&BufferDC, TA_LEFT, true
	);
	ProjectText(
		14, true, false, false, L"Verdana", 560, 340, CTBlue, 
		std::to_wstring(MaxCapByDesignVal) + L" mWh", 
		&BufferDC, TA_LEFT, true
	);
	ProjectText(
		14, true, false, false, L"Verdana", 690, 320, CTBlue, 
		std::to_wstring(MaxCapCurrentlyPerc) + L" %", 
		&BufferDC, TA_LEFT, true
	);
	ProjectText(
		14, true, false, false, L"Verdana", 690, 340, CTBlue, 
		std::to_wstring(MaxCapCurrentlyVal) + L" mWh", 
		&BufferDC, TA_LEFT, true
	);

	// Skriv infotekst til rapportknapp.
	ProjectText(
		12, false, false, false, L"Verdana", 570, 435, Black, 
		L"Automatic test-stop w/report at 15%.", 
		&BufferDC, TA_LEFT, true
	);

	//
	//	Skriv mellomlager til prosjektoren og rydd opp.
	//
	BitBlt(hdc, 0, 0, cx, cy, BufferDC, 0, 0, SRCCOPY);
	DeleteObject(BufferBitmap);
	DeleteDC(BufferDC);
	ReleaseDC(*ProjectorHwnd, hdc);

	} catch(...) { 
		GetError(L"UI error."); 
		exit(EXIT_FAILURE);
	}
}

//
//	Klikkhåndterer - Posisjonering og aksjoner.
//
bool LbuttonIsDown = false;
bool NewLbuttonUpEvent = false;
POINT WindowClick;
RECT PrevWindowPos;
int WinX = 816;
int WinY = 489;

void HandleMousePositioning()
{
	POINT MusPunkt;

	GetCursorPos(&MusPunkt);
	GetWindowRect(*ProjectorHwnd, &WindowRect);
	int AksjonX = MusPunkt.x - WindowRect.left;
	int AksjonY = MusPunkt.y - WindowRect.top;

	//
	//	Vindu kan beveges så lenge venstre museknapp er nede.
	//
	if (LbuttonIsDown) {
	
		// Debug for å finne ca knappeposisjoner.
		// Deaktivér SetWindowPos mens denne brukes.
		//StatusText = std::to_string(AksjonX) +" x "+ std::to_string(AksjonY);

		SetWindowPos(
			*ProjectorHwnd, 
			HWND_TOP,
			MusPunkt.x - (WindowClick.x - PrevWindowPos.left),
			MusPunkt.y - (WindowClick.y - PrevWindowPos.top),
			WinX,
			WinY,
			SWP_SHOWWINDOW
		);
	}
}