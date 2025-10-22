#include "PathParser.h"
#include <stdexcept>
#include <sstream>
#include <iostream>

using namespace app;

void PathParser::Parse(std::ifstream& file) {
    std::string line;
    int lineNumber = 0;
    float prevX = 0.f, prevY = 0.f, prevZ = 0.f;
    bool isFirst = true;

    while (std::getline(file, line)) {
        lineNumber++;
        if (!line.empty() && line.back() == '\r') { line.pop_back(); }
        if (line.empty()) { continue; }

        MillingCommand cmd;
        cmd.rawCommand = line;
        cmd.lineNumber = lineNumber;

        std::istringstream iss(line);

        ParseN(cmd, iss, lineNumber);
        ParseG(cmd, iss, lineNumber);

        bool hasX = false, hasY = false, hasZ = false;
        char character;
        while (iss >> character) {
            if (!(character == 'X' || character == 'Y' || character == 'Z')) {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + " : expected X, Y or Z, got " + std::string(1, character));
            }

            int whole;
            if (!(iss >> whole) || std::abs(whole) >= 100) {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + " : invalid coordinate value for " + std::string(1, character));
            } 

            char dot;
            if (!(iss >> dot) || dot != '.') {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + " : missing fractional part for " + std::string(1, character));
            } 

            char buffer[4] = { 0 };
            if (!iss.read(buffer, 3)) {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + " : not enough characters available");
            }

            int fractional;
            try { 
                fractional = std::stoi(buffer);
            } catch (const std::exception& e) {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + " : invalid fractional part for " + std::string(1, character));
            }

            if (fractional < 0 || fractional >= 1000) {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + " : invalid fractional value for " + std::string(1, character));
            } 

            float value = whole;
            if (whole >= 0) {
                value += fractional / 1000.f;
            } else {
                value -= fractional / 1000.f; 
            }

            switch (character) {
                case 'X':
                {
                    if (hasY || hasZ) {
                        throw std::runtime_error("Line " + std::to_string(lineNumber) + " : wrong order of coordinates");
                    }
                    cmd.coordinates.x() = value;
                    hasX = true;
                    prevX = value;
                    break;
                }
                case 'Y':
                {
                    if (hasZ) {
                        throw std::runtime_error("Line " + std::to_string(lineNumber) + " : wrong order of coordinates");
                    }
                    cmd.coordinates.y() = value;
                    hasY = true;
                    prevY = value;
                    break;
                }
                case 'Z':
                {
                    cmd.coordinates.z() = value;
                    hasZ = true;
                    prevZ = value;
                    break;
                }
            }
        }

        if (!iss.eof()) {
            std::string remaining;
            std::getline(iss, remaining);
            throw std::runtime_error("Line " + std::to_string(lineNumber) + " : unexpected characters after coordinates: " + remaining);
        }

        if (!hasX && !hasY && !hasZ) {
            throw std::runtime_error("Line " + std::to_string(lineNumber) + " : move command has no coordinates");
        }
        if (!hasX) {
            cmd.coordinates.x() = prevX;
        }
        if (!hasY) {
            cmd.coordinates.y() = prevY;
        }
        if (!hasZ) {
            cmd.coordinates.z() = prevZ;
        }

        if (isFirst) {
            if (!hasX || !hasY || !hasZ) {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + " : first move command must include all coordinates");
            }
            isFirst = false;
        } else {
            MillingCommand& prevCmd = path.back();
            float length = (prevCmd.coordinates - cmd.coordinates).length() / 10.f;
            pathLength += length;
        }

        path.push_back(cmd);
    }

    // reset
    file.clear();
    file.seekg(0, std::ios::beg);
}

void PathParser::Clear() {
    path.clear();
    pathLength = 0.f;
}

void PathParser::ParseN(MillingCommand& cmd, std::istringstream& line, int& lineNumber) {
    char dot;
    if (!(line >> dot) || dot != 'N') {
        throw std::runtime_error("Line " + std::to_string(lineNumber) + " : line does not start with 'N'");
    }
    int nValue;
    if (!(line >> nValue) || nValue < 1) {
        throw std::runtime_error("Line " + std::to_string(lineNumber) + " : invalid or missing command number");
    }
    cmd.commandNumber = nValue;
}

void PathParser::ParseG(MillingCommand& cmd, std::istringstream& line, int& lineNumber) {
    char dot;
    if (!(line >> dot) || dot != 'G') {
        throw std::runtime_error("Line " + std::to_string(lineNumber) + " : expected G01, got invalid character");
    }
    if (!(line >> dot) || dot != '0') {
        throw std::runtime_error("Line " + std::to_string(lineNumber) + " : expected G01, got G" + std::string(1, dot));
    }
    if (!(line >> dot) || dot != '1') {
        throw std::runtime_error("Line " + std::to_string(lineNumber) + " : expected G01, got G0" + std::string(1, dot));
    }
    cmd.type = CommandType::WorkingMove;
}
