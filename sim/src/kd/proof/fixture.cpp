#include "kd/proof/fixture.hpp"

namespace kd::proof {

std::vector<data::SourceFile> fixture_files() {
    return {
        {"base/source.toml", "id = \"base\"\nversion = 1\nabout = \"the game\"\n"},
        {"demo/source.toml", "id = \"demo\"\nversion = 1\nabout = \"the demonstration\"\nrequires = [\"base\"]\n"},
        {"demo/marker/walker.toml",
         "speed = \"1.4 m/s\"\nreach = \"2 m\"\ngreets = \"30%\"\nrest = { life = \"2 h\", game = \"2 h\" }\n"
         "colour = \"#e8c25a\"\ngait = \"walk\"\nwalks_with = [\"demo:walker\", \"demo:strider\"]\n"},
        {"demo/marker/strider.toml",
         "speed = \"2 m/s\"\nreach = \"3 m\"\ngreets = \"1 in 10\"\nrest = { life = \"45 min\", game = \"45 min\" }\n"
         "colour = \"#8fd18a\"\ngait = \"stride\"\n"},
        {"demo/tuning/crowd.toml",
         "camps = 400\nper_camp = 25\narea = \"20 km\"\nwander = \"1 km\"\ndawn = \"6 h\"\ndusk = \"20 h\"\n"
         "homeward = \"40%\"\n"
         "greeting = { life = \"1 min\", game = \"1 min\" }\n"},
    };
}

}  // namespace kd::proof
