#pragma once
#include <fstream>
#include <vector>
#include <string>
#include "../gmod/vector3.h"

namespace app {
	class PathParser {
	public:
		enum class CommandType {
			SetMillimetres, // G71
			SetInches, // G70
			Start, // G40G90
			SetAngularSpeed, // N2Sxxxx
			SetClockwise, // M03
			SetMovementSpeed, // Fxxxx 
			FastMove, // G00
			WorkingMove, // G01 - for now only this one is used
			TurnOff, // M05
			End // M30
		};

		struct MillingCommand {
			std::string rawCommand;
			int lineNumber;
			int commandNumber; // Nxx
			CommandType type; // for now, all should be WorkingMove
			unsigned short speed = 15; // xxxx - for now unused
			gmod::vector3<float> coordinates; // Xff.fffYff.fffZff.fff
		};

		std::vector<MillingCommand> path;

		// in centimetres
		float pathLength = 0.0f;

		// in minutes
		// float pathTime = 0.0f; // unavailable due to the lack of velocity

		void Parse(std::ifstream& file);
		void Clear();
	private:
		void ParseN(MillingCommand& cmd, std::istringstream& line, int& lineNumber);
		void ParseG(MillingCommand& cmd, std::istringstream& line, int& lineNumber);
	};
}
