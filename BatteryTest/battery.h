#pragma once

class BatteriKlasse
{
    private:
    // Datastruktur for batterier som blir funnet.
    struct BatteriData {

        // Fra BATTERY_STATUS.
        // https://docs.microsoft.com/en-us/windows/win32/power/battery-status-str
        long CurBatteryRate = 0;                // mW (milliwatt)
        unsigned long CurBatteryCapacity = 0;   // mWh (milliwatt-timer)
        unsigned long CurBatteryVoltage = 0;    // mV (millivolt)
        unsigned long CurBatteryState = 0;

        // Fra BATTERY_INFORMATION.
        // https://docs.microsoft.com/en-us/windows/win32/power/battery-information-str
        unsigned long DesBatteryCapacity = 0;
        unsigned long MaxBatteryCapacity = 0; // Sammenligne med Designed for slitasje.
    };



    public: 
    // Liste over batteridata per batteri som blir funnet.
    std::vector<BatteriData> BatteriListe;



    // Funksjon som henter batterier i systemet og spør dem om informasjon.
    // Gjør regelmessige kall til denne via en sløyfe i main e.l.
    void GetBatteryState()
    {
        HDEVINFO hdev = SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if (hdev == INVALID_HANDLE_VALUE)
            GetError(L"Failed at SetupDiGetClassDevs().");

        // Nullstill foreløpig batteridata.
        BatteriListe.clear();

        // Limit search to 100 batteries max
        for (int batnum = 0; batnum < 100; batnum++) {

            SP_DEVICE_INTERFACE_DATA batifdata = { 0 };
            batifdata.cbSize = sizeof(batifdata);

            // Spesifiser at det er batteridetaljer vi ønsker i batifdata (battery interface data).
            if (!SetupDiEnumDeviceInterfaces(hdev, 0, &GUID_DEVCLASS_BATTERY, batnum, &batifdata)) {

                // Sjekk om det bare ikke var noe data tilgjengelig.
                if (GetLastError() == 259)
                    return;

                // Vis andre typer feil.
                GetError(L"Failed at SetupDiEnumDeviceInterfaces().");
            }

            // Hent størrelse som trengs på buffer til cbRequired.
            DWORD cbRequired = 0;
            if (!SetupDiGetDeviceInterfaceDetail(hdev, &batifdata, 0, 0, &cbRequired, 0) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                GetError(L"Failed at SetupDiGetDeviceInterfaceDetail(1).");

            // Forbered DID minne som er stor nok for detaljer.
            SP_DEVICE_INTERFACE_DETAIL_DATA* pbatifdata = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, cbRequired);
            if (pbatifdata == 0)
                GetError(L"Failed at allokering av minne til pbatifdata.");

            // Hent batteri interface data.
            pbatifdata->cbSize = sizeof(*pbatifdata);
            if (!SetupDiGetDeviceInterfaceDetail(hdev, &batifdata, pbatifdata, cbRequired, &cbRequired, 0)) {
                LocalFree(pbatifdata);
                GetError(L"Failed at SetupDiGetDeviceInterfaceDetail(2).");
            }

            // Opprett handle til batteriet.
            HANDLE hBattery = CreateFile(pbatifdata->DevicePath,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (hBattery == INVALID_HANDLE_VALUE) {
                LocalFree(pbatifdata);
                GetError(L"Failed at HANDLE hBattery = CreateFile().");
            }

            // Spør batteriet om en tag.
            BATTERY_QUERY_INFORMATION bqi = { 0 };
            DWORD dwWait = 0;
            DWORD dwOut;
            if (DeviceIoControl(hBattery,
                IOCTL_BATTERY_QUERY_TAG,
                &dwWait,
                sizeof(dwWait),
                &bqi.BatteryTag,
                sizeof(bqi.BatteryTag),
                &dwOut,
                NULL) == 0
                || bqi.BatteryTag == 0
                ) {
                CloseHandle(hBattery);
                LocalFree(pbatifdata);
                GetError(L"Failed at IOCTL_BATTERY_QUERY_TAG.");
            }

            // Med en tag, kan man spørre om mer informasjon.
            BATTERY_INFORMATION bi = { 0 };
            bqi.InformationLevel = BatteryInformation;

            if (DeviceIoControl(hBattery,
                IOCTL_BATTERY_QUERY_INFORMATION,
                &bqi,
                sizeof(bqi),
                &bi,
                sizeof(bi),
                &dwOut,
                NULL) == 0
                ) {
                CloseHandle(hBattery);
                LocalFree(pbatifdata);
                GetError(L"Failed at IOCTL_BATTERY_QUERY_INFORMATION.");
            }

            // Sjekk at batteriet ikke er en UPS / fail-safe.
            if (!(bi.Capabilities & BATTERY_SYSTEM_BATTERY)) {
                CloseHandle(hBattery);
                LocalFree(pbatifdata);
                GetError(L"Battery is not a system battery.");
            }

            if (bi.Capabilities & BATTERY_IS_SHORT_TERM) {
                CloseHandle(hBattery);
                LocalFree(pbatifdata);
                GetError(L"Battery seems to be a fail-safe / UPS.");
            }

            // Sjekk status til batteriet.
            BATTERY_WAIT_STATUS bws = { 0 };
            bws.BatteryTag = bqi.BatteryTag;

            BATTERY_STATUS bs;
            if (DeviceIoControl(hBattery,
                IOCTL_BATTERY_QUERY_STATUS,
                &bws,
                sizeof(bws),
                &bs,
                sizeof(bs),
                &dwOut,
                NULL) == 0
                ) {
                CloseHandle(hBattery);
                LocalFree(pbatifdata);
                GetError(L"Feilet på IOCTL_BATTERY_QUERY_STATUS.");
            }
			
            // Verdier jeg er interessert i.
            BatteriData bidata;
            bidata.CurBatteryCapacity = bs.Capacity;
            bidata.CurBatteryRate = bs.Rate;
            bidata.CurBatteryState = bs.PowerState;
            bidata.CurBatteryVoltage = bs.Voltage;
            bidata.DesBatteryCapacity = bi.DesignedCapacity;
            bidata.MaxBatteryCapacity = bi.FullChargedCapacity;
            BatteriListe.push_back(bidata);

            CloseHandle(hBattery);
            LocalFree(pbatifdata);
        }

        SetupDiDestroyDeviceInfoList(hdev);
    }

    static std::wstring OversettPowerstate(unsigned long state)
    {
        std::wstring status;

        if (state & BATTERY_CRITICAL)
            status.append(L"Critical health. ");

        if (state & BATTERY_CHARGING)
            status.append(L"Charging. ");

        if (state & BATTERY_DISCHARGING)
            status.append(L"Discharging. ");

        if (state & BATTERY_POWER_ON_LINE)
            status.append(L"AC Power. ");

        if (status.length() == 0) {
            status.append(L"Unknown.");
        }

        return status.c_str();
    }

	//
	// Husk: Ekstra ANSI versjon av denne under for rapportbruk.
	// libcurl mail ser ut til å ville ha const char* format.
	//
    static std::wstring HentTidspunkt(float HoursRemaining, bool fra_start)
    {
        // Hent timer og minutter via cast til int (ta heltall fra desimal).
		int timer, minutter;
		tm* lokaltid;
		if (fra_start) {
			lokaltid = localtime(&tid_fra_start);
	        timer = lokaltid->tm_hour + (int)HoursRemaining;
		    minutter = lokaltid->tm_min + (int)((HoursRemaining - (int)HoursRemaining) * 60);
		} else {
			time_t tid = time(0);
			lokaltid = localtime(&tid);
			timer = lokaltid->tm_hour + (int)HoursRemaining;
		    minutter = lokaltid->tm_min + (int)((HoursRemaining - (int)HoursRemaining) * 60);
		}

        // Juster tid etter tillegg.
        if (timer >= 24)
            timer -= 24;
        
        if (minutter >= 60) {
            minutter -= 60;
            timer += 1;
        }

        // Juster format, prepend 0 til tall under 10.
        std::wstring tid_frem;
        if (timer < 10)
            tid_frem = L"0" + std::to_wstring(timer);
        else
            tid_frem = std::to_wstring(timer);
        
        if (minutter < 10)
            tid_frem += L":0" + std::to_wstring(minutter);
        else
            tid_frem += L":" + std::to_wstring(minutter);        

        // Returner klokkeslett i timer:minutter.
        return tid_frem;
    }

	static std::string HentTidspunktA(float HoursRemaining, bool fra_start)
    {
		int timer, minutter;
		tm* lokaltid;
		if (fra_start) {
			lokaltid = localtime(&tid_fra_start);
	        timer = lokaltid->tm_hour + (int)HoursRemaining;
		    minutter = lokaltid->tm_min + (int)((HoursRemaining - (int)HoursRemaining) * 60);
		} else {
			time_t tid = time(0);
			lokaltid = localtime(&tid);
			timer = lokaltid->tm_hour + (int)HoursRemaining;
		    minutter = lokaltid->tm_min + (int)((HoursRemaining - (int)HoursRemaining) * 60);
		}

        // Juster tid etter tillegg.
        if (timer >= 24)
            timer -= 24;
        
        if (minutter >= 60) {
            minutter -= 60;
            timer += 1;
        }

        // Juster format, prepend 0 til tall under 10.
        std::string tid_frem;
        if (timer < 10)
            tid_frem = u8"0" + std::to_string(timer);
        else
            tid_frem = std::to_string(timer);
        
        if (minutter < 10)
            tid_frem += u8":0" + std::to_string(minutter);
        else
            tid_frem += u8":" + std::to_string(minutter);        

        // Returner klokkeslett i timer:minutter.
        return tid_frem;
    }

	static std::string HentTestVarighetA()
	{
		std::string varighet;
		int timer_start, minutter_start;
		int timer_stopp, minutter_stopp;
		tm* lokaltid;

		// Starttid.
		lokaltid = localtime(&tid_fra_start);
	    timer_start = lokaltid->tm_hour;
		minutter_start = lokaltid->tm_min;

		// Stopptid (nå).
		time_t tid = time(0);
		lokaltid = localtime(&tid);
		timer_stopp = lokaltid->tm_hour;
		minutter_stopp = lokaltid->tm_min;

		// Normaliser tider til antall minutter.
		int totalt_minutter_fra_start = ((timer_stopp*60)+minutter_stopp) - ((timer_start*60)+minutter_start);
		int timer_fra_start = (int)(totalt_minutter_fra_start/60);
		int minutter_fra_start = totalt_minutter_fra_start % 60;

		varighet = std::to_string(timer_fra_start) + u8" hour(s) og "+ 
					std::to_string(minutter_fra_start) + u8" minute(s)";

        return varighet;
	}
};


void BatteryUpdateMain()
{
    // Gi hovedvinduet tid til å opprette seg skikkelig.
    Sleep(200);

    BatteriKlasse Batterier;
    statustekster.push_back(L"Starting ...");

    while(1) {

        // Oppdater UI.
		UpdateProjection();
        SendMessage(ReportButtonHwnd, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)ReportButtonImg);

        // Intervall mot batteri.
        Sleep(5000);
        statustekster.clear();

        // Hent batteridata.
        Batterier.GetBatteryState();

        // Oppdater tekst som skal vises.
        ++rundeteller;
		statustekster.push_back(
			std::to_wstring(rundeteller) + 
			L" battery queries performed against battery driver (stage " +
			std::to_wstring(BatteriTestStatus) +L")."
		);

        // Fant jeg batteri?
        if (Batterier.BatteriListe.size() == 0) {
            statustekster.push_back(L"No battery found...");
            mutexlock.lock();
            RunningOnBattery = false;
            mutexlock.unlock();
            continue;

        } else {
            statustekster.push_back(L"Battery found.");
        }

		if (Batterier.BatteriListe[0].CurBatteryRate == INT_MIN) {
			statustekster.push_back(L"Charge rate returned INT_MIN, battery not supported for testing.");
			statustekster.push_back(L"Charge rate is the only reliable way to check the end of charge.");
			continue;
		}

        // Hent detaljer dersom det fantes ett batteri, velg det første som ble funnet.
        MaxCapCurrentlyPerc = (int)round(
			((float)Batterier.BatteriListe[0].MaxBatteryCapacity * 100) / (float)Batterier.BatteriListe[0].DesBatteryCapacity
		);
        MaxCapByDesignPerc = 100;
        MaxCapCurrentlyVal = Batterier.BatteriListe[0].MaxBatteryCapacity;
        MaxCapByDesignVal = Batterier.BatteriListe[0].DesBatteryCapacity;

        statustekster.push_back(L"Status: " + BatteriKlasse::OversettPowerstate(Batterier.BatteriListe[0].CurBatteryState));
        statustekster.push_back(L"Capacity: " + std::to_wstring(Batterier.BatteriListe[0].CurBatteryCapacity) + L" mWh.");
        statustekster.push_back(L"Charge rate: " + std::to_wstring(Batterier.BatteriListe[0].CurBatteryRate) + L" mW.");
        statustekster.push_back(L"Voltage: " + std::to_wstring(Batterier.BatteriListe[0].CurBatteryVoltage) + L" mV.");

		if (BatteriTestStatus == 4) {
			statustekster.push_back(L"Test ended after discharge round 2/2.");
			continue;
		}

        // Kjører PC på batteri?
        // Cpu tråden sjekker RunningOnBattery for å velge om den skal stresse.
        if (!(Batterier.BatteriListe[0].CurBatteryState & BATTERY_POWER_ON_LINE)) {

			// Gå rundt hvis batteri ikke har hatt en full opplading enda.
			// Det unngår at rapport ikke sendes for tidlig.
			if (BatteriTestStatus == 0 || BatteriTestStatus == 2)  {
				statustekster.push_back(L"Charge battery and wait for message about max capacity.");
				continue;
			}

			// Sjekk om det er lite batteri igjen. Stopp og send rapport.
			if (((Batterier.BatteriListe[0].CurBatteryCapacity * 100) / MaxCapCurrentlyVal) <= 15) {

				// Gi tilbakemelding om lavt batteri.
				PlaySound(
					MAKEINTRESOURCE(LOW_POWER_SND),
					GetModuleHandle(NULL),
					SND_RESOURCE | SND_ASYNC | SND_NODEFAULT
				);

				// Har batteriet vært gjennom en oppladingsrunde?
				if (BatteriTestStatus == 1) {
					BatteriTestStatus = 2;

					// Send stoppsignal til CPU stress.
					mutexlock.lock();
					RunningOnBattery = false;
					mutexlock.unlock();
					continue;
				}
				
				// Send kun automatisk rapport etter 2 utladinger.
				if (BatteriTestStatus == 3) {
					BatteriTestStatus = 4;
					
					// Send stoppsignal til CPU stress.
					mutexlock.lock();
					RunningOnBattery = false;
					mutexlock.unlock();

					// Send rapport (kun elon verksted).
					/*
					CurlMailer cm;
					cm.AddRecipient(RapportMottaker.c_str());
					cm.To = RapportMottaker;
					cm.Cc = "";
					cm.Message = u8"<b>Rapport sendt ved automatisk grense på 15%</b>" + BatteriRapport();
					cm.SendMail();
					continue;
					*/

					// Skriv ut rapport til HTML fil i stedet for alle andre.
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
				}
			}

            // Engangsting ved frakobling av lader før en test begynner.
            if (mWhStartCapacity == 0) {
                mutexlock.lock();
                RunningOnBattery = true;
                mutexlock.unlock();
                mWhStartCapacity = Batterier.BatteriListe[0].CurBatteryCapacity;
                tid_fra_start = time(0);
            }

        // Vekselstrøm.
        } else {           
			mutexlock.lock();
            RunningOnBattery = false;
            mutexlock.unlock();
            mWhStartCapacity = 0;
			ratecounter = 0;
            AvgRate.clear();

			// Sjekk status ved første fulle batteri-tilstand.
			if (Batterier.BatteriListe[0].CurBatteryRate == 0) {

				// Når man kobler til lader kan det ta litt tid med 0 
				// før rate faktisk starter. Vent litt på dette...
				Sleep(3000);
				if (Batterier.BatteriListe[0].CurBatteryRate != 0)
					continue;

				// Har batteriet vært gjennom en utladingsrunde?
				if (BatteriTestStatus == 2 || BatteriTestStatus == 3) {
					statustekster.push_back(L"Disconnect charger for discharge round 2 of 2.");
					BatteriTestStatus = 3;
				} else {
					statustekster.push_back(L"Disconnect charger for discharge round 1 of 2.");
					BatteriTestStatus = 1;
				}
			} else {
				// Gi beskjed om opplading som aller første steg.
				if (BatteriTestStatus == 0 || BatteriTestStatus == 2)
					statustekster.push_back(L"Charger connected. Charge until you're asked to disconnect.");
			}

            continue;
        }


        //
        // Vis diagnoseinfo dersom det kjøres på batteri og cpu blir stresset for å lade ut strøm.
        //

        // Regn ut rate gj.snitt fra starten av test etter 3 rates for å unngå spike-verdier.
		ratecounter += 1;
		if (Batterier.BatteriListe[0].CurBatteryRate != INT_MIN && ratecounter > 3)
	        AvgRate.push_back(Batterier.BatteriListe[0].CurBatteryRate);

        // Ikke start visning før det finnes minst 3 trekk-registreringer, for et gj.snitt.
        if (ratecounter < 5) {
            statustekster.push_back(L"Calculating average discharge rate, waiting for at least 5 values ... "+ std::to_wstring(ratecounter) +L"/5.");
            continue;
        }

        long rate = 0;
        for (size_t a=0; a<AvgRate.size(); a++)
            rate += AvgRate[a];
        rate = rate / (long)AvgRate.size();

        // Beregn ny estimering per runde.
        float TilgjengeligTrekkBurde = ((float)mWhStartCapacity / (float)(rate - (rate * 2)));
        float TilgjengeligTrekkEgentlig = ((float)Batterier.BatteriListe[0].CurBatteryCapacity / (float)(rate - (rate * 2)));

        // Registrer tekst.
        statustekster.push_back(L"");
        statustekster.push_back(L"Starting CPU load for smooth discharge ("+ std::to_wstring(Batterier.BatteriListe[0].CurBatteryCapacity) +L" / "+ std::to_wstring(mWhStartCapacity) +L" mWh).");
        statustekster.push_back(L"");
        statustekster.push_back(L"Start time: "+ BatteriKlasse::HentTidspunkt(0.0f, true));
        statustekster.push_back(L"Battery SHOULD last until ~ " + BatteriKlasse::HentTidspunkt(TilgjengeligTrekkBurde, true) + L".");
        statustekster.push_back(L"Battery seems to ACTUALLY last until ~ "+ BatteriKlasse::HentTidspunkt(TilgjengeligTrekkEgentlig, false) +L".");
        statustekster.push_back(L"Time factors: "+ std::to_wstring(TilgjengeligTrekkBurde) +L" (SHOULD), "+ std::to_wstring(TilgjengeligTrekkEgentlig) +L" (ACTUALLY).");
        statustekster.push_back(L"Discharge rate average from start: "+ std::to_wstring(rate) +L" mW.");
        statustekster.push_back(L"");
        statustekster.push_back(L"Avoid temporary usage of PC for stable discharge and calculations.");
        statustekster.push_back(L"If the discharge rate spike often, it can shorten the ACTUALLY time.");

		// Oppdater rapportdetaljer.
		RapportDetaljer.TestBurdeVareTil = BatteriKlasse::HentTidspunktA(TilgjengeligTrekkBurde, true);
		RapportDetaljer.TestVarerFaktiskTil = BatteriKlasse::HentTidspunktA(TilgjengeligTrekkEgentlig, false);
		RapportDetaljer.TestStartet = BatteriKlasse::HentTidspunktA(0.0f, true);
		RapportDetaljer.TestStoppet = BatteriKlasse::HentTidspunktA(0.0f, false);
		RapportDetaljer.TestUtladningsSnitt = std::to_string(rate) +u8" mW.";
		RapportDetaljer.TestKapasitetVedStart = std::to_string(mWhStartCapacity) +u8" mWh.";
		RapportDetaljer.TestKapasitetVedStopp = std::to_string(Batterier.BatteriListe[0].CurBatteryCapacity) +u8" / "+ std::to_string(mWhStartCapacity) +u8" mWh.";
		RapportDetaljer.TestMaksKapasitetHelse = std::to_string(MaxCapCurrentlyPerc) + u8" %.";
		RapportDetaljer.TestProsentResterende = std::to_string((int)round(((float)Batterier.BatteriListe[0].CurBatteryCapacity * 100) / (float)MaxCapCurrentlyVal)) + u8" %";
		RapportDetaljer.TestVarighet = BatteriKlasse::HentTestVarighetA();
    }
}