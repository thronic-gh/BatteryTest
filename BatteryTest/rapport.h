#pragma once

std::string BatteriRapport()
{
	// Hent brukernavn til windows konto som test er utført som.
	char username[257];
	DWORD usernameLen = sizeof(char)*257;
	if (!GetUserNameA(username, &usernameLen))
		GetError(L"GetUserNameA feilet.");

	// Er det startet en test i det hele tatt
	std::string Rapport = 
	u8"<h1>BATTERY TEST REPORT</h1>" 

	u8"<b>A calibration and chemical quality measurement has been performed on the PC("+ std::string(username) +u8") battery.</b><br><br>"

	u8"<b>Test duration:</b> "+ RapportDetaljer.TestVarighet +u8" (" + RapportDetaljer.TestStartet +u8" to "+ RapportDetaljer.TestStoppet +u8").<br>"
	u8"Report generated at "+ RapportDetaljer.TestKapasitetVedStopp +u8" ("+ RapportDetaljer.TestProsentResterende +") capacity.<br><br>"
	
	u8"<b>Predicted battery life:</b><br>"
	u8"<b>SHOULD</b>: " + RapportDetaljer.TestBurdeVareTil +"<br>"
	u8"<b>ACTUAL</b>: " + RapportDetaljer.TestVarerFaktiskTil +"<br><br>"

	u8"<b>Max Capacity:</b> " + std::to_string(MaxCapCurrentlyVal) +" mWh. Battery manufactured for "+ std::to_string(MaxCapByDesignVal) +" mWh.<br>"
	u8"<b>Health Percentage:</b> " + RapportDetaljer.TestMaksKapasitetHelse +"<br>"
	u8"<em>Information comes from the battery itself.</em><br><br>"

	u8"Beyond simple health, the test also measured chemical quality with SHOULD vs ACTUAL life prediction.<br>"
	u8"It is done by measuring against wattage discharged rate average for the duration of the test.<br>"
	u8"Try to avoid a lot of discharge spikes by leaving the computer alone while the test is running.<br>"
	u8"If the discharge spikes up a lot during the test, it will affect the ACTUAL draw to be shorter.<br>"
	u8"A few minutes difference is usually OK due to background and scheduled software that may interrupt.";

	return Rapport;
}