#include "platinum.h"
#include <locale.h>
#include <string.h>

/**
 * p_linux_app_init
 *
 * Creates an application with GUI and input.
 * Takes a PWindowRequest for window creation and modifies the PAppData.
 */
void p_linux_app_init(PAppData *app_data, PAppRequest app_request)
{
	P_UNUSED(app_data);
	P_UNUSED(app_request);
}

/**
 * p_linux_app_deinit
 *
 * Closes the application
 * Takes a PAppData to deconstruct.
 */
void p_linux_app_deinit(PAppData *app_data)
{
	P_UNUSED(app_data);
}
