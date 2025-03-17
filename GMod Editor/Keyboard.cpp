#include "Keyboard.h"
#include <windowsx.h>

bool Keyboard::isCtrlDown(WPARAM wParam) {
	return wParam == VK_CONTROL;
}

bool Keyboard::isShiftDown(WPARAM wParam) {
	return wParam == VK_SHIFT;
}

bool Keyboard::isAltDown(WPARAM wParam) {
	return wParam == VK_MENU;
}

bool Keyboard::isEscDown(WPARAM wParam) {
	return wParam == VK_ESCAPE;
}
