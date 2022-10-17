#include <nlohmann/json.hpp>
#include "Main.h"

using namespace std;
using json = nlohmann::json;

namespace DSXSkyrim
{

    void to_json(json& j, const Instruction& p);

    void from_json(const json& j, Instruction& p);

    void to_json(json& j, const Packet& p);

    void from_json(const json& j, Packet& p);

    void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);

    std::string PacketToString(Packet& packet);



}  // namespace DSXSkyrim