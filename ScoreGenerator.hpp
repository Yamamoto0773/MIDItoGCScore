#ifndef __SCORE_GENERATOR__
#define __SCORE_GENERATOR__


#include "MIDIReader.hpp"

#include <unordered_map>



enum class NoteType : int {
    Hit,
    Hold,
    Critical,
    DualHold,    
    Slide,
    SlideHold,
    DualSlide,
    Beat,
    Scratch,
    Adlib
};


struct GeneratingConfig {
    std::unordered_map<int, NoteType> keyAssign;
    math::Fraction holdMinLength;
};

class ScoreGenerator {
public:
    ScoreGenerator(const midireader::MIDIReader& midi);

    void generate(std::ostream& stream, size_t trackNumber, const GeneratingConfig& config);

    void generate_header(std::ostream& stream);

private:
    const midireader::MIDIReader& midi;
};


#endif
