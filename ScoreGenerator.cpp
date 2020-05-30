#include "ScoreGenerator.hpp"

#include <sstream>


NoteType operator | (const NoteType& left, const NoteType& right) {
    return static_cast<NoteType>(static_cast<int>(left) | static_cast<int>(right));
}

NoteType operator |= (NoteType& left, const NoteType& right) {
    return left = left | right;
}

NoteType operator & (const NoteType& left, const NoteType& right) {
    return static_cast<NoteType>(static_cast<int>(left) & static_cast<int>(right));
}


std::string toNoteTypeString(NoteType type) {
    switch (type) {
    case NoteType::Hit:
        return "hit";
    case NoteType::Hold:
        return "hold";
    case NoteType::Critical:
        return "critical";
    case NoteType::DualHold:
        return "dualhold";
    case NoteType::Slide:
        return "slide";
    case NoteType::SlideHold:
        return "slidehold";
    case NoteType::DualSlide:
        return "dualslide";
    case NoteType::Beat:
        return "beat";
    case NoteType::Scratch:
        return "scratch";
    case NoteType::Adlib:
        return "adlib";
    default:
        //throw std::exception("unknown note type");
        return "";
    }
}


ScoreGenerator::ScoreGenerator(const midireader::MIDIReader& reader)
    : midi(reader) {}

void ScoreGenerator::generate(std::ostream& stream, size_t trackNumber, const GeneratingConfig& config) {
    using namespace midireader;
    using namespace std::string_literals;

    stream << "\t" << R"("notes": [)" << "\n";

    int numofElements = 0;

    auto midiEvents = midi.getNoteEvent(trackNumber);
    auto current = midiEvents.cbegin();
    while (current != midiEvents.cend()) {
        if (current->type == MidiEvent::NoteOff) continue;

        // find note off event
        std::vector<NoteEvent>::const_iterator next;
        for (auto it = current + 1; it != midiEvents.cend(); it++) {
            if (it->interval != current->interval) continue;

            if (it->type == MidiEvent::NoteOff) {
                next = it;
                break;
            } else {
                std::stringstream errorMsg;
                errorMsg
                    << "エラー\n"
                    << "ノーツの重複が検出されました\n"
                    << "該当ノーツ詳細\n"
                    << "位置: 小節 " << it->bar << ":" << (it->posInBar + math::Fraction(1, it->posInBar.get().d)).get_str() << "\n"
                    << "音程(MIDI number): " << it->interval << "\n";

                throw std::exception(errorMsg.str().c_str());
            }
        }

        NoteType type;
        try {
            type = config.keyAssign.at(current->interval);
        } catch (const std::exception&) {
            current = next+1;
            continue;
        }

        if (type == NoteType::Hit || type == NoteType::Critical || type == NoteType::Slide) {
            math::Fraction noteLength = (next->bar + next->posInBar) - (current->bar + current->posInBar);
            if (noteLength >= config.holdMinLength) {
                if (type == NoteType::Hit) {
                    type = NoteType::Hold;
                } else if (type == NoteType::Critical) {
                    type = NoteType::DualHold;
                } else if (type == NoteType::Slide) {
                    type = NoteType::SlideHold;
                }
            }
        }

        // to human-readable notation
        math::Fraction at_frac = current->posInBar + math::Fraction(1, current->posInBar.get().d);
        math::Fraction end_at_frac = next->posInBar + math::Fraction(1, next->posInBar.get().d);

        if (numofElements > 0) {
            stream << ",\n";
        }

        std::stringstream str;
        str << "\t\t" << R"({)" << "\n";
        str << "\t\t\t" << R"("type": )" << "\"" << toNoteTypeString(type) << "\"" << ",\n"
               << "\t\t\t" << R"("at": )" << "\"" << current->bar << ":" << at_frac.get_str() << "\"";

        if (type != NoteType::Hit && type != NoteType::Critical && type != NoteType::Slide && type != NoteType::Adlib && type != NoteType::DualSlide) {
            str << ",\n";
            str << "\t\t\t" << R"("end_at": )" << "\"" << next->bar << ":" << end_at_frac.get_str() << "\"";
        }


        if (type == NoteType::Slide || type == NoteType::SlideHold) {
            stream << str.str()
                   << ",\n"
                   << "\t\t\t" << R"("direction": )" << "\"" << "left" << "\"";
        } else if (type == NoteType::DualSlide) {
            stream << str.str()
                   << ",\n"
                   << "\t\t\t" << R"("direction": )" << "\"" << "left" << "\""
                   << ",\n"
                   << "\t\t\t" << R"("direction2": )" << "\"" << "right" << "\"";
        } else {
            stream << str.str();
        }

        stream << "\n";
        stream << "\t\t" << R"(})";

        current = next + 1;

        numofElements++;
    }

    stream << "\n" << "\t]";
}


void ScoreGenerator::generate_header(std::ostream& stream) {
    auto beatEvents = midi.getBeatEvent();
    auto tempoEvents = midi.getTempoEvent();

    int numofElements = 0;

    stream << "\t" << R"("time_signature": [)" << "\n";
    for (const auto& event : beatEvents) {
        if (numofElements > 0) {
            stream << ",\n";
        }

        stream << "\t\t" << R"({)" << "\n"
               << "\t\t\t" << R"("at": )" << "\"" << event.bar << ":" << "1/1" << "\"" << ",\n"
               << "\t\t\t" << R"("value": )" << "\"" << event.beat.get_str() << "\"" << "\n"
               << "\t\t" << R"(})";

        numofElements++;
    }
    stream << "\n" << "\t]";

    stream << ",\n";

    numofElements = 0;
    stream << "\t" << R"("tempo": [)" << "\n";
    for (const auto& event : tempoEvents) {
        if (numofElements > 0) {
            stream << ",\n";
        }

        // 小節内の位置を1originにする
        math::Fraction pos = event.posInBar + math::Fraction(1, event.posInBar.get().d);

        stream << "\t\t" << R"({)" << "\n"
               << "\t\t\t" << R"("at": )" << "\"" << event.bar << ":" << pos.get_str() << "\"" << ",\n"
               << "\t\t\t" << R"("value": )" << event.tempo  << "\n"
               << "\t\t" << R"(})";

        numofElements++;
    }
    stream << "\n" << "\t]";
}
