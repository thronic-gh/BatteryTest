#pragma once

void mainloop()
{
	while(1) {

		// Hold systemet v�kent.
		SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);

		// Flytting av vindu ved drag p� UI.
		HandleMousePositioning();

		// CPU vennlig.
		Sleep(10); 
	}
}