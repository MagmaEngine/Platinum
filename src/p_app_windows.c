#include "platinum.h"
#include <locale.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

/**
 * p_windows_app_init
 *
 * Creates an application with GUI and input.
 * Takes a PWindowRequest for window creation and returns a PAppData.
 */
void p_windows_app_init(PAppData *app_data, PAppRequest app_request)
{
	P_UNUSED(app_data);
	P_UNUSED(app_request);

	// Makes a console
	AllocConsole();
	FILE* stream;

	// Reopen stdout in binary mode to support Unicode output
	if ((_wfreopen_s(&stream, L"CONOUT$", L"w", stdout) || _wfreopen_s(&stream, L"CONOUT$", L"w", stderr))!= 0)
	{
		perror("Failed to reopen stdout or stderr");
		exit(1);
	}
	_setmode(_fileno(stderr), _O_U16TEXT);
	_setmode(_fileno(stdout), _O_U16TEXT);

}

/**
 * p_windows_app_deinit
 *
 * Closes the application
 * Takes a PAppData to deconstruct.
 */
void p_windows_app_deinit(PAppData *app_data)
{
	P_UNUSED(app_data);
	//FreeConsole(); // Closes the log console. Also ends the application.
	//Not needed since console closes on its own anyways.
}
