#pragma once

void mainloop()
{
	while(1) {

		// Hold systemet våkent.
		SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);

		// Flytting av vindu ved drag på UI.
		HandleMousePositioning();

		// CPU vennlig.
		Sleep(10); 
	}
}