#include "PathParser.h"
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

using namespace app;

void PathParser::Save(const std::vector<gmod::vector3<float>>& path, const std::string& stage, const std::string& type, const std::string& diameter) const {
    fs::path currDir = fs::current_path();
    std::string fileName = stage + "." + type + diameter;
    fs::path filePath = currDir / fileName;

    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to create file for writing: " + filePath.string());
    }

    std::stringstream ss;

    int cmdNum = 3;
    std::string prevValueX = "", prevValueY = "", prevValueZ = "";

    enum HASVALUE { HAS_NONE = 0, HAS_X = 1, HAS_Y = 2, HAS_Z = 4 };
    unsigned short prevMask = 0, prevNumOfValues = 0;

    for (int k = 0; k < path.size(); k++) {
        const auto& point = path[k];

        std::string prefix = "N" + std::to_string(cmdNum) + "G01";
        std::string valueX = "X" + Float2String(point.z());
        std::string valueY = "Y" + Float2String(point.x());
        std::string valueZ = "Z" + Float2String(point.y());

        unsigned short mask = 0;
        unsigned short numberOfValues = 0;

        bool sameX = prevValueX == valueX;
        if (!sameX) {
            mask |= HAS_X;
            numberOfValues++;
        }
     
        bool sameY = prevValueY == valueY;
        if (!sameY) {
            mask |= HAS_Y;
            numberOfValues++;
        }

        bool sameZ = prevValueZ == valueZ;
        if (!sameZ) {
            mask |= HAS_Z;
            numberOfValues++;
        }

        if (mask == HAS_NONE) {
            // skip repeating lines
            continue; 
        }

        if (numberOfValues == 1 &&
            numberOfValues == prevNumOfValues &&
            mask == prevMask &&
            k != 0 && k != path.size() - 1) {

            const auto& nextPoint = path[k + 1];
            std::string nextValueX = "X" + Float2String(nextPoint.z());
            std::string nextValueY = "Y" + Float2String(nextPoint.x());
            std::string nextValueZ = "Z" + Float2String(nextPoint.y());

            unsigned short nextMask = 0;
            if (nextValueX != valueX) { nextMask |= HAS_X; }
            if (nextValueY != valueY) { nextMask |= HAS_Y; }
            if (nextValueZ != valueZ) { nextMask |= HAS_Z; }

            if (mask == nextMask) {
                // skip unnecessary intermediate points
                continue;
            }
        }

        prevMask = mask;
        prevNumOfValues = numberOfValues;

        ss << prefix;
        if (!sameX) {
            prevValueX = valueX;
            ss << valueX;
        }
        if (!sameY) {
            prevValueY = valueY;
            ss << valueY;
        }
        if (!sameZ) {
            prevValueZ = valueZ;
            ss << valueZ;
        }
        ss << std::endl;
        cmdNum++;
    }

    file << ss.str();
    file.close();
}

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

            bool negative = iss.peek() == '-';

            int whole;
            if (!(iss >> whole) || std::abs(whole) >= 1000) {
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
            if (negative) {
                value -= fractional * 1e-3f;
            } else {
                value += fractional * 1e-3f;
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

        cmd.coordinates = gmod::vector3<float>(cmd.coordinates.y(), cmd.coordinates.z(), cmd.coordinates.x());

        if (isFirst) {
            if (!hasX || !hasY || !hasZ) {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + " : first move command must include all coordinates");
            }
            isFirst = false;
            cmd.distance = 0.f;
        } else {
            MillingCommand& prevCmd = m_path.back();
            float length = (prevCmd.coordinates - cmd.coordinates).length() / 10.f;
            pathLength += length;
            cmd.distance = length;
        }
        m_path.push_back(cmd);
    }

    file.close();
}

void PathParser::Clear() {
    m_path.clear();
    pathLength = 0.f;
}

std::optional<PathParser::MillingCommand> PathParser::GetNextCommand() {
    if (m_cmdIt >= m_path.size()) {
        return std::nullopt;
    }
    return m_path[m_cmdIt++];
}

void PathParser::ResetCommandIterator() {
    m_cmdIt = 0;
}

std::optional<PathParser::NextStep> PathParser::GetNextStep(float step) {
    if (m_stepIt == 0) {
        m_stepIt++;
        if (m_path.empty()) { return std::nullopt; }
        auto& first = m_path.front();
        m_stepPos = first.coordinates;
        return NextStep{ { first.coordinates }, { first.commandNumber } };
    }
    if (m_stepIt >= m_path.size()) {
        return std::nullopt;
    }
    constexpr float fzero = 100.f * std::numeric_limits<float>::epsilon();
    float length = (m_path[m_stepIt].coordinates - m_stepPos).length();
    if (std::abs(length - step) < fzero) {
        auto& currCmd = m_path[m_stepIt];
        m_stepIt++;
        m_stepPos = currCmd.coordinates;
        return NextStep{ { currCmd.coordinates }, { currCmd.commandNumber } };
    }
    if (step < length) {
        auto& currCmd = m_path[m_stepIt];
        gmod::vector3<float> moveVec = (currCmd.coordinates - m_stepPos).normalized() * step;
        auto newCoord = m_stepPos + moveVec;
        m_stepPos = newCoord;
        return NextStep{ { newCoord }, { currCmd.commandNumber } };
    } else {
        std::vector<gmod::vector3<float>> newCoords;
        std::vector<int> commandNumbs;
        while (step > fzero && m_stepIt < m_path.size()) {
            auto& currCmd = m_path[m_stepIt];
            gmod::vector3<float> vec = (currCmd.coordinates - m_stepPos);
            float currLen = vec.length();

            if (currLen <= step) {
                newCoords.push_back(currCmd.coordinates);
                commandNumbs.push_back(currCmd.commandNumber);
                m_stepIt++;
                m_stepPos = currCmd.coordinates;
                step -= currLen;
            } else {
                gmod::vector3<float> moveVec = vec.normalized() * step;
                auto newCoord = m_stepPos + moveVec;
                m_stepPos = newCoord;

                newCoords.push_back(newCoord);
                commandNumbs.push_back(currCmd.commandNumber);
                m_stepPos = newCoord;
                step = 0;
            }
        }
        return NextStep{ newCoords, commandNumbs };
    }
}

void PathParser::ResetStepIterator() {
    m_stepIt = 0;
    m_stepPos = gmod::vector3<float>(0, 0, 0);
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

std::string PathParser::Float2String(float value) {
    std::string result = std::to_string(value);

    size_t decimalPos = result.find('.');
    if (decimalPos != std::string::npos) {
        if (result.length() > decimalPos + 4) {
            result = result.substr(0, decimalPos + 4);
        } else {
            // add missing zeros
            result.append(4 - (result.length() - decimalPos), '0');
        }
    } else {
        result += ".000";
    }

    return result;
}
