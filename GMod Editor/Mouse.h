#pragma once
#include <wtypes.h>

struct Mouse {
	bool isLMBDown_flag = false;
	bool isMMBDown_flag = false;
	bool isRMBDown_flag = false;
	bool isCtrlDown_flag = false;
	bool isShiftDown_flag = false;

	POINT prevCursorPos = { 0, 0 };
	LONG wheelDistance = 0;

	void UpdatePos(LPARAM lParam);
	void UpdateDist(WPARAM wParam);
	void UpdateFlags(WPARAM wParam);

	static LONG GetXPos(LPARAM lParam);
	static LONG GetYPos(LPARAM lParam);
	static LONG GetWheelDelta(WPARAM wParam);

	static bool isLMBDown(WPARAM wParam);
	static bool isMMBDown(WPARAM wParam);
	static bool isRMBDown(WPARAM wParam);
	static bool isCtrlDown(WPARAM wParam);
	static bool isShiftDown(WPARAM wParam);
};