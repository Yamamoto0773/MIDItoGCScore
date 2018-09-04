#include "MIDItoScore.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>


bool isNumber(char ch) {
	return '0' <= ch && ch < '9';
}

bool isAlphabet(char ch) {
	return ('a' <= ch && ch <= 'g') || ('A' <= ch && ch <= 'G');
}

bool getNumber(int &num, size_t digit) {
	bool ret = true;
	num = 0;

	for (int i = 0; i < digit; i++) {
		char ch;

		std::cin >> ch;
		if (isNumber(ch)) {
			num *= 10;
			num += atoi(&ch);
		} else {
			ret = false;
			break;
		}
	}
	
	std::cin.ignore();

	return ret;
}

int searchTrack(const std::vector<midireader::Track> &tracks, const std::string searchName) {
	int trackNum = -1;

	for (const auto t : tracks) {
		if (t.name == searchName) {
			trackNum = t.trackNum;
			break;
		}
	}

	return trackNum;
}



bool getInterval(std::string &interval) {
	bool ret = true;
	size_t progress = 0;

	while (true) {
		char ch;

		std::cin >> ch;

		if (ch == ' ')
			continue;

		switch (progress) {
		case 0:
			if (!isAlphabet(ch)) {
				ret = false;
				continue;
			}
			break;
		case 1:
			if (ch != '#' && !isNumber(ch)) {
				ret = false;
				continue;
			}
			break;
		case 2:
			if (!isNumber(ch)) {
				ret = false;
				continue;
			}
		}


		interval += ch;
		progress++;

		if (isNumber(ch))
			break;
	}

	std::cin.ignore();

	return ret;
}




int main() {

	using namespace midireader;
	using std::cin;
	using std::cout;

	bool loopFlag;

	// get class number
	int classNum;
	loopFlag = true;

	while (loopFlag) {
		cout << "�g�ԍ�����͂��ĉ�����(3��)\n>";

		if (getNumber(classNum, 3)) {
			loopFlag = false;
		} else {
			cout << "[!] ���̓G���[�ł��D3���̔��p�����œ��͂��ĉ�����\n";
		}
	}


	// get the midi file path
	MIDIReader midir;
	loopFlag = true;
	Status ret;

	while (loopFlag) {
		cout << "MIDI�t�@�C���ւ̃p�X����͂��ĉ�����(\"��\'�������܂܂ł�OK�ł�)\n>";

		std::string filePath;
		char str[256];
		cin.getline(str, 256, '\n');
		filePath = str;


		// erase ' or "
		if (!filePath.empty()) {
			if (filePath.front() == '\'' || filePath.front() == '\"')
				filePath.erase(filePath.begin());
			if (filePath.back() == '\'' || filePath.back() == '\"')
				filePath.erase(filePath.end() - 1);
		}

		midir.setAdjustmentAmplitude(2);
		ret = midir.openAndRead(filePath);

		if (ret == Status::E_CANNOT_OPEN_FILE) {
			cout << "[!] �t�@�C�����J���܂���.�p�X���m�F���ĉ�����\n";
		} else if (ret == Status::E_INVALID_ARG) {
			cout << "[!] �p�X����ł��D\n";
		} else {
			loopFlag = false;
		}
	}


	cout << "\nMIDI�t�@�C����ǂݍ���ł��܂�... ";

	// print result of reading the midi file
	switch (ret) {
	case midireader::Status::E_UNSUPPORTED_FORMAT:
		cout << "[!] ���̃t�H�[�}�b�g�̓T�|�[�g����Ă��܂���.\n";
		return 0;
	case midireader::Status::E_INVALID_FILE:
		cout << "[!] MIDI�t�@�C�����j�����Ă��܂�\n";
		return 0;
	case midireader::Status::S_OK:
		cout << "�ǂݍ��݊���\n\n";
		break;
	case midireader::Status::S_NO_EMBED_TIMESIGNATURE:
		cout << "[!] MIDI�t�@�C���ɔ��q��񂪖��ߍ��܂�Ă��܂���.\n";
		return 0;
	default:
		break;
	}

	if (midir.getTempoEvent().empty()) {
		cout << "[!] MIDI�t�@�C���Ƀe���|��񂪖��ߍ��܂�Ă��܂���.\n";
		return 0;
	}




	// get interval
	cout << "�ł����݂Ɏg������������͂��Ă������� (��:C3, D#3)\n";


	std::vector<std::string> intervalList;
	for (size_t i = 0; i < 4; i++) {
		loopFlag = true;
		while (loopFlag) {

			// print lane position;
			// ex. if i = 1, print "��������"
			for (size_t j = 0; j < i; j++) cout << "��";
			cout << "��";
			for (size_t j = 4 - i - 1; j > 0; j--) cout << "��";

			cout << " ������" << i+1 << "�Ԗڂ̃��[���̉��� >";

			std::string str;
			if (getInterval(str)) {
				intervalList.push_back(str);
				loopFlag = false;
			} else {
				cout << "[!] �����Ƃ��Đ���������܂���D\n";
			}
		}
	}


	// to upper
	for (auto &i : intervalList) {
		std::transform(i.begin(), i.end(), i.begin(), std::toupper);
	}


	// set note format
	miditoscore::MIDItoScore toscore;
	miditoscore::NoteFormat format;
	format.holdMaxVelocity = 63;
	format.laneAllocation = intervalList;


	// create score file name
	std::stringstream scoreName;
	time_t t = time(nullptr);
	tm lt;
	localtime_s(&lt, &t);

	{
		using namespace std;
		scoreName << classNum;
		scoreName << '_';
		scoreName << setfill('0') << setw(2) << lt.tm_mon + 1;
		scoreName << setfill('0') << setw(2) << lt.tm_mday;
		scoreName << ".txt";
	}											  




	// ---------------------------------------------------
	// write score

	cout << "\n���ʃf�[�^���쐬���܂�\n";

	std::ofstream score;
	score.open(scoreName.str());


	score << "begin:header\n\n";
	score << u8"id:<��ID>" << '\n';
	score << u8"title:<�Ȗ�>" << '\n';
	score << u8"artist:<�A�[�e�B�X�g��>" << '\n';


	// write tempo
	cout << "�e���|���\n";
	score << '\n';

	const auto tempo = midir.getTempoEvent();
	for (const auto t : tempo) {
		using namespace std;

		// ex. tempo:001:1/0:120.000
		score << "tempo:"
			<< setfill('0') << setw(3) << t.bar
			<< ':'
			<< t.posInBar.get_str()
			<< ':'
			<< setw(6) << fixed << setprecision(3) << t.tempo
			<< '\n';

		cout << "����:"
			<< setfill('0') << setw(3) << t.bar
			<< " ���ߓ��ʒu:"
			<< t.posInBar.get_str()
			<< " �e���|:"
			<< setw(6) << fixed << setprecision(3) << t.tempo
			<< '\n';
	}


	// write time signature
	cout << "\n���q���\n";

	const auto beat = midir.getBeatEvent();
	for (const auto b : beat) {
		using namespace std;

		// ex. beat:001:4/4
		score << "beat:"
			<< setfill('0') << setw(3) << b.bar
			<< ':'
			<< b.beat.get_str()
			<< '\n';

		cout << "����:"
			<< setfill('0') << setw(3) << b.bar
			<< " ���q:"
			<< b.beat.get_str()
			<< '\n';
	}

	score << "\nend\n\n";


	// write note position
	for (char targetTrackName = '1'; targetTrackName <= '3'; targetTrackName++) {

		size_t trackNum = -1;
		const auto tracks = midir.getTrackList();
		trackNum = searchTrack(tracks, std::string(1, targetTrackName));
		if (trackNum < 0) {
			continue;
		}


		cout << '\n';
		switch (targetTrackName) {
		case '1':
			cout << "easy���ʂ��쐬���ł�... ";
			score << "begin:easy\n\n";
			break;
		case '2':
			cout << "normal���ʂ��쐬���ł�... ";
			score << "begin:normal\n\n";
			break;
		case '3':
			cout << "hard���ʂ��쐬���ł�... ";
			score << "begin:hard\n\n";
			break;
		}


		auto ret = toscore.writeScore(score, format, midir, trackNum);

		score << "\nend\n\n";

		switch (ret) {
		case miditoscore::Status::S_OK:
			cout << "����\n";
			break;
		case miditoscore::Status::E_EXIST_CONCURRENTNOTES:
			cout << "[!] �����^�C�~���O�̃m�[�c�����݂��Ă��܂��D";
			cout << "�������̏I�_�Ǝn�_���d�Ȃ��Ă��Ȃ����`�F�b�N���ĉ�����\n";
			cout << "-- ���̂���m�[�c --\n";
			{
				auto notes = toscore.getConcurrentNotes();
				for (auto it = notes.cbegin(); it != notes.cbegin()+10; it++) {
					using namespace std;
					cout << "����:"
						<< setfill('0') << setw(3) << it->bar
						<< " ���ߓ��ʒu:"
						<< it->posInBar.get_str()
						<< " ����:"
						<< it->interval
						<< '\n';
				}

				if (notes.size() > 10)
					cout << "...��" << notes.size() - 10 << "�R\n";
			}
			break;
		case miditoscore::Status::S_EXIST_DEVIATEDNOTES:
			cout << "[!] �w�肳�ꂽ�����ɓ��Ă͂܂�Ȃ��m�[�c�����݂��Ă��܂�.\n";
			cout << "-- ���̂���m�[�c --\n";
			{
				auto notes = toscore.getDeviatedNotes();
				for (auto it = notes.cbegin(); it != notes.cbegin()+10; it++) {
					using namespace std;
					cout << "����:"
						<< setfill('0') << setw(3) << it->bar
						<< " ���ߓ��ʒu:"
						<< it->posInBar.get_str()
						<< " ����:"
						<< it->interval
						<< '\n';
				}

				if (notes.size() > 10)
					cout << "...��" << notes.size() - 10 << "�R\n";
			}
			break;
		default:
			break;
		}


		cout << "--�m�[�c����-----\n";
		cout <<	"    |";
		for (auto i : intervalList) {
			cout << std::setfill(' ') << std::setw(4) << i << '|';
		}
		cout << '\n';
		cout << "hit |";
		for (auto i : intervalList) {
			cout << std::setfill(' ') << std::setw(4) << toscore.numofHitNotes(i) << '|';
		}
		cout << '\n';
		cout << "hold|";
		for (auto i : intervalList) {
			cout << std::setfill(' ') << std::setw(4) << toscore.numofHoldNotes(i) << '|';
		}
		cout << '\n';

		size_t cnt = 0;
		cout << "all |";
		for (auto i : intervalList) {
			cout << std::setfill(' ') << std::setw(4) 
				<< toscore.numofHoldNotes(i) + toscore.numofHitNotes(i) << '|';

			cnt += toscore.numofHoldNotes(i) + toscore.numofHitNotes(i);
		}
		cout << '\n';
		cout << "total:" << cnt << '\n';
	


	}




}