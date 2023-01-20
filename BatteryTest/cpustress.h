#pragma once

void CpuStress()
{
	bool _stop = false;

	while(1) {

		// Trådsikker sjekk av batteristatus.
		mutexlock.lock();
		if (!RunningOnBattery)
			_stop = true;
		else
			_stop = false;
		mutexlock.unlock();

		if (_stop) {
			Sleep(500);
			continue;
		}

		// Bruk hel, del og boolean for mest mulig bruk av cpu registre.
		int n = 99;
		for (int a=n; a>0; a--)
			n=n;

		float f = 99.9f;
		for (float a=f; a>0; a--)
			f=f;

		bool b = true;
		for (int a=0; a<99; a++)
			b = (b==true ? false : true);
	}
}