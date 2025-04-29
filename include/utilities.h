#include <string>
#include <sstream>
#include <iomanip>

namespace Utilities {
  std::string formatHex(int fillWidth, auto value) {
    std::stringstream hexStream; 
    hexStream << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(fillWidth) << value;
    return hexStream.str();
  };
}
