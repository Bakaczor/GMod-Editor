#include "Mouse.h"
#include <windowsx.h>

using namespace app;

void Mouse::UpdatePos(LPARAM lParam) {
	prevCursorPos = {
		GetXPos(lParam),
		GetYPos(lParam)
	};
}

void Mouse::UpdateDist(WPARAM wParam) {
	wheelDistance += GetWheelDelta(wParam);
}

void Mouse::UpdateFlags(WPARAM wParam) {
	isLMBDown_flag = isLMBDown(wParam);
	isMMBDown_flag = isMMBDown(wParam);
	isRMBDown_flag = isRMBDown(wParam);
	isCtrlDown_flag = isCtrlDown(wParam);
	isShiftDown_flag = isShiftDown(wParam);
}

LONG Mouse::GetXPos(LPARAM lParam) {
	return GET_X_LPARAM(lParam);
}

LONG Mouse::GetYPos(LPARAM lParam) {
	return GET_Y_LPARAM(lParam);
}

LONG Mouse::GetWheelDelta(WPARAM wParam) {
	return GET_WHEEL_DELTA_WPARAM(wParam);
}

bool Mouse::isLMBDown(WPARAM wParam) {
	WORD keys = GET_KEYSTATE_WPARAM(wParam);
	return (keys & MK_LBUTTON) != 0;
}

bool Mouse::isMMBDown(WPARAM wParam) {
	WORD keys = GET_KEYSTATE_WPARAM(wParam);
	return (keys & MK_MBUTTON) != 0;
}

bool Mouse::isRMBDown(WPARAM wParam) {
	WORD keys = GET_KEYSTATE_WPARAM(wParam);
	return (keys & MK_RBUTTON) != 0;
}

bool Mouse::isCtrlDown(WPARAM wParam) {
	WORD keys = GET_KEYSTATE_WPARAM(wParam);
	return (keys & MK_CONTROL) != 0;
}

bool Mouse::isShiftDown(WPARAM wParam) {
	WORD keys = GET_KEYSTATE_WPARAM(wParam);
	return (keys & MK_SHIFT) != 0;
}
