#pragma once
#include <wtypes.h>

namespace app {
	struct Keyboard {
		bool isCtrlDown_flag = false;
		bool isShiftDown_flag = false;
		bool isAltDown_flag = false;
		bool isEscDown_flag = false;

		static bool isCtrlDown(WPARAM wParam);
		static bool isShiftDown(WPARAM wParam);
		static bool isAltDown(WPARAM wParam);
		static bool isEscDown(WPARAM wParam);
	};
}