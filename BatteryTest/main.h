#pragma once
#define NOMINMAX

// Hovedheader for program.

#include <Windows.h>
#include <string>
#include <bitset>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <SetupAPI.h>			// Husk å linke mot SetupAPI.lib
#include <batclass.h>
#include <devguid.h>
#include "globalevariabler.h"
#include "resource.h"
#include "geterror.h"
#include "ui.h"
#include "rapport.h"
#include "battery.h"
#include "cpustress.h"
#include "mainloop.h"