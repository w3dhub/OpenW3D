#include "wwdialog.h"

#if defined(OPENW3D_WIN32)
#include <windows.h>
#elif defined(OPENW3D_SDL3)
#include <SDL3/SDL_messagebox.h>

static const SDL_MessageBoxButtonData sdl3_ok_buttons[] = {
	{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, MESSAGEBOX_BUTTON_OK, "OK", },
};
static const SDL_MessageBoxButtonData sdl3_abort_retry_ignore_buttons[] = {
	{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, MESSAGEBOX_BUTTON_ABORT, "Abort", },
	{ 0, MESSAGEBOX_BUTTON_RETRY, "Retry", },
	{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, MESSAGEBOX_BUTTON_IGNORE, "Ignore", },
};

#endif


int Show_Message_Box(unsigned int flags, const char *message, const char *title)
{
#if defined(OPENW3D_WIN32)
	int mb_flags = 0;

	switch (flags & MESSAGEBOX_SEVERITY_MASK) {
	case MESSAGEBOX_SEVERITY_INFO:
		mb_flags |= 0;
		break;
	case MESSAGEBOX_SEVERITY_WARNING:
		mb_flags |= MB_ICONWARNING;
		break;
	case MESSAGEBOX_SEVERITY_ERROR:
		mb_flags |= MB_ICONERROR;
		break;
	}
	switch (flags & MESSAGEBOX_BUTTONS_MASK) {
	case MESSAGEBOX_BUTTONS_OK:
		mb_flags |= MB_OK;
		break;
	case MESSAGEBOX_BUTTONS_ABORTRETRYIGNORE:
		mb_flags |= MB_ABORTRETRYIGNORE;
		break;
	}
	switch (MessageBoxA(NULL, message, title, mb_flags)) {
	case IDOK:
	case IDYES:
	case IDCONTINUE:
	default:
		return MESSAGEBOX_BUTTON_OK;
	case 0:
	case IDCANCEL:
		return MESSAGEBOX_BUTTON_CANCEL;
	case IDNO:
	case IDABORT:
		return MESSAGEBOX_BUTTON_ABORT;
	case IDRETRY:
	case IDTRYAGAIN:
		return MESSAGEBOX_BUTTON_RETRY;
	case IDIGNORE:
		return MESSAGEBOX_BUTTON_IGNORE;
	}
#elif defined(OPENW3D_SDL3)
	SDL_MessageBoxData sdl3_message_box_data = { 0 };
	sdl3_message_box_data.title = title;
	sdl3_message_box_data.message = message;
	switch (flags & MESSAGEBOX_SEVERITY_MASK) {
	case MESSAGEBOX_SEVERITY_INFO:
		sdl3_message_box_data.flags |= SDL_MESSAGEBOX_INFORMATION;
		break;
	case MESSAGEBOX_SEVERITY_WARNING:
		sdl3_message_box_data.flags |= SDL_MESSAGEBOX_WARNING;
		break;
	case MESSAGEBOX_SEVERITY_ERROR:
		sdl3_message_box_data.flags |= SDL_MESSAGEBOX_ERROR;
		break;
	}
	switch (flags & MESSAGEBOX_BUTTONS_MASK) {
	case MESSAGEBOX_BUTTONS_OK:
		sdl3_message_box_data.buttons = sdl3_ok_buttons;
		sdl3_message_box_data.numbuttons = SDL_arraysize(sdl3_ok_buttons);
		break;
	case MESSAGEBOX_BUTTONS_ABORTRETRYIGNORE:
		sdl3_message_box_data.buttons = sdl3_abort_retry_ignore_buttons;
		sdl3_message_box_data.numbuttons = SDL_arraysize(sdl3_abort_retry_ignore_buttons);
		break;
	}
	int clicked_button;
	if (!SDL_ShowMessageBox(&sdl3_message_box_data, &clicked_button)) {
		return MESSAGEBOX_BUTTON_CANCEL;
	}
	return clicked_button;
#else
	assert(0);
#endif
}
