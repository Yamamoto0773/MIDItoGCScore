#include "ScoreGenerator.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <array>
#include <filesystem>


bool isNumber(char ch) {
    return '0' <= ch && ch <= '9';
}

bool isIntervalAlphabet(char ch) {
    return ('a' <= ch && ch <= 'g') || ('A' <= ch && ch <= 'G');
}

int searchTrack(const std::vector<midireader::Track>& tracks, const std::string searchName) {
    int trackNum = -1;

    for (const auto t : tracks) {
        if (t.name == searchName) {
            trackNum = t.trackNum;
            break;
        }
    }

    return trackNum;
}

bool isInclude(int val, int flag) {
    return (val & flag) == flag;
}

bool toNumber(const std::string& str, int* number = nullptr) {
    size_t numEndedPos;
    int n;
    try {
        n = std::stoi(str, &numEndedPos);
    } catch (const std::exception&) {
        return false;
    }

    if (numEndedPos != str.length())
        return false;

    if (number) {
        *number = n;
    }

    return true;
}

bool toDouble(const std::string& str, double* number = nullptr) {
    size_t numEndedPos;
    double n;
    try {
        n = std::stod(str, &numEndedPos);
    } catch (const std::exception&) {
        return false;
    }

    if (numEndedPos != str.length())
        return false;

    if (number) {
        *number = n;
    }

    return true;
}


bool toIntervalStr(std::string& str, std::string& intervalStr) {
    intervalStr.clear();

    if (str.length() < 2)
        return false;

    if (!isIntervalAlphabet(str.at(0)))
        return false;

    constexpr std::array<char, 5> sharpAttachable = { 'C', 'D', 'F', 'G', 'A' };
    const bool isAttachable =
        std::find_if(
            sharpAttachable.cbegin(),
            sharpAttachable.cend(),
            [&](char ch) { return ch == std::toupper(str.at(0)); }
    ) != sharpAttachable.cend();

    if (!isAttachable && str.at(1) == '#')
        return false;

    std::string numStr;
    if (str.at(1) == '#') {
        numStr = str.substr(2);
        intervalStr += str.substr(0, 2);
    } else {
        numStr = str.substr(1);
        intervalStr += str.substr(0, 1);
    }

    int octave;
    if (!toNumber(numStr, &octave))
        return false;

    intervalStr += std::to_string(octave);

    return true;
}

bool toFraction(std::string& str, math::Fraction& frac) {
    using std::cin;

    size_t slashPos = str.find('/');
    if (slashPos == std::string::npos)
        return false;

    int numer, denom;
    try {
        const auto numerStr = str.substr(0, slashPos);
        numer = std::stoi(numerStr);
        const auto denomStr = str.substr(slashPos + 1);
        denom = std::stoi(denomStr);

        frac.set(numer, denom); // may occured that denom is zero exception
    } catch (const std::exception&) {
        return false;
    }

    if (frac <= 0)
        return false;

    return true;
}

void stop() {
    std::cout << "\nプログラムを終了するにはEnterを押してください\n";
    std::cin.ignore();
}

int main() {
    using namespace midireader;
    namespace fs = std::filesystem;
    using std::cin;
    using std::cout;
    using std::string;
    using std::wstring;

    bool loopFlag = true;

    // get the midi file path
    MIDIReader midi;
    
    Status ret;


    fs::path midiPath;

    cout << "MIDIファイルをドラック&ドロップし，Enterキーを押してください\n";
    while (true) {
        cout << ">";
        std::string filePath;        
        std::getline(cin, filePath);

        // erase ' or "
        if (!filePath.empty()) {
            if (filePath.front() == '\'' || filePath.front() == '\"')
                filePath.erase(filePath.begin());
            if (filePath.back() == '\'' || filePath.back() == '\"')
                filePath.erase(filePath.end() - 1);
        }

        midi.setAdjustmentAmplitude(2, 1024);
        ret = midi.openAndRead(filePath);

        // print result of reading the midi file
        switch (ret) {
        case midireader::Status::E_CANNOT_OPEN_FILE:
            cout << "[!] ファイルが開けません.";
            continue;
        case midireader::Status::E_INVALID_ARG:
            cout << "[!] パスが空です．\n";
            continue;
        case midireader::Status::E_UNSUPPORTED_FORMAT:
            cout << "[!] このフォーマットはサポートされていません.\n";
            continue;
        case midireader::Status::E_INVALID_FILE:
            cout << "[!] MIDIファイルではありません\n";
            continue;
        case midireader::Status::S_NO_EMBED_TIMESIGNATURE:
            cout << "[!] MIDIファイルに拍子情報が埋め込まれていません.DAWの書き出し設定を確認してください\n";
            continue;        
        }

        midiPath = filePath;
        cout << "読み込み完了\n\n";
        break;
    }

    // get interval
    cout << "打ち込みに使った音程を入力してください\n"
        << "(例: 音名で入力する場合:c3, d#3  番号で入力する場合:60, 63)\n";

    std::array<NoteType, 7> allocateTypes = { NoteType::Hit, NoteType::Critical, NoteType::Slide, NoteType::DualSlide, NoteType::Beat, NoteType::Scratch, NoteType::Adlib };
    std::vector<std::string> intervalStrings(allocateTypes.size());
    std::vector<int> intervalNumbers(allocateTypes.size(), -1);
    bool givedIntervalAsStr = false;

    for (int i = 0; i < allocateTypes.size(); i++) {
        while (true) {
            switch (i) {
            case 0: cout << " Hit/Hold >"; break;
            case 1: cout << " Critical/DualHold >"; break;
            case 2: cout << " Slide/SlideHold >"; break;
            case 3: cout << " DualSlide >"; break;
            case 4: cout << " Beat >"; break;
            case 5: cout << " Scratch >"; break;
            case 6: cout << " Adlib >"; break;
            }

            std::string inputStr, intervalStr;
            std::getline(cin, inputStr);

            int n;
            if (toIntervalStr(inputStr, intervalStr)) {
                intervalStrings.at(i) = intervalStr;
                givedIntervalAsStr = true;
                break;
            } else if (toNumber(inputStr, &n) && n >= 0) {
                intervalNumbers.at(i) = n;
                break;
            } else {
                cout << "[!] 音程として正しくありません．\n";
            }
        }
    }
    

    cout << '\n';

    PitchNotation pitchNotation;
    if (givedIntervalAsStr) {
        cout << "DAWの一番低い音程を入力してください [c-2, c-1, c0のどれか]\n";

        while (true) {
            cout << ">";

            std::string inputStr, intervalStr;
            std::getline(cin, inputStr);

            if (toIntervalStr(inputStr, intervalStr)) {
                intervalStr.front() = toupper(intervalStr.front());
                if (intervalStr == "C-2") {
                    pitchNotation = PitchNotation::A3_440Hz;
                    break;
                } else if (intervalStr == "C-1") {
                    pitchNotation = PitchNotation::A4_440Hz;
                    break;
                } else if (intervalStr == "C0") {
                    pitchNotation = PitchNotation::A5_440Hz;
                    break;
                } else {
                    cout << "[!] C-2, C-1, C0のどれかを入力してください．\n";
                }
            } else {
                cout << "[!] 音程として正しくありません．\n";
            }
        }

        // convert interval string to number
        for (size_t i = 0; i < allocateTypes.size(); i++) {
            if (intervalNumbers.at(i) == -1) {
                intervalNumbers.at(i) = midireader::toNoteNum(intervalStrings.at(i), pitchNotation);
            }
        }
    }

    // get hold minimal length
    cout << "\n";
    cout << "長押しノーツにする長さを入力してください (例: 1/16)\n"
        << "(16分音符1つ分以上の長さのノーツを長押しにしたい場合は 1/16と入力)\n";

    math::Fraction holdMinLen;
    while (true) {
        cout << ">";

        std::string str;
        std::getline(cin, str, '\n');

        if (toFraction(str, holdMinLen)) {
            break;
        } else {
            cout << "[!] 0より大きい分数を入力してください\n";
        }
    }


    cout << "\n";
    cout << "-- トラック一覧 --\n";
    for (const auto& track : midi.getTracks()) {
        cout << track.trackNum << "\t" << track.name << "\t" << midi.getNoteEvent(track.trackNum).size() / 2 << " notes.\n";
    }
    cout << "-----------------\n";

    cout << "譜面書き出しに使うトラック番号を入力してください\n";
    int trackNum;
    while (true) {
        cout << ">";

        std::string str;
        std::getline(cin, str, '\n');

        if (toNumber(str, &trackNum)) {
            break;
        } else {
            cout << "[!] 数字を入力してください\n";
        }
    }

    // ---------------------------------------------------
    // write score

    cout << "\n譜面データを作成します\n";
    
    const fs::path scoreFilePath = midiPath.parent_path() / "notes.json";

    std::ofstream scoreFile;
    scoreFile.open(scoreFilePath);

    ScoreGenerator generator(midi);

    scoreFile << "{\n";
    generator.generate_header(scoreFile);
    scoreFile << ",\n";

    GeneratingConfig config;
    config.holdMinLength = math::Fraction(1, 4);
    for (int i = 0; i < allocateTypes.size(); i++) {
        config.keyAssign[intervalNumbers[i]] = allocateTypes[i];
    }

    // write note position
    for (char targetTrackName = '1'; targetTrackName < '2'; targetTrackName++) {        
        generator.generate(scoreFile, trackNum, config);
    }

    scoreFile << "\n}\n";
    scoreFile.close();
    midi.close();

    cout << "\n譜面データを作成しました\n";

    stop();

    return 0;
}
