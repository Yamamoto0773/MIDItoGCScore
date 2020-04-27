# MIDItoGCScore
MIDIファイルからGCSimulatorのノーツ配置を作成するツールです．

### 概要
ノーツ配置が書かれたMIDIファイルを読み込み，GCSimulatorのノーツ配置jsonファイルを書き出します．

SMF(Standard MIDI File)のうち，フォーマット0と1に対応しています．フォーマット2には対応していません．
また，デルタタイムを実時間で表す形式(ほとんどないと思いますが)には対応していません．


### 使い方

```cpp
#include "ScoreGenerator.hpp"

int main() {
    using namespace midireader;
    MIDIReader midi;

    // Read Midi file
    midi.openAndRead("sample.mid");

    // Open score file
    std::ofstream scoreFile;
    scoreFile.open("notes.json");

    // Set midi data to ScoreGenerator
    ScoreGenerator generator(midi);

    // Write Header
    scoreFile << "{\n";
    generator.generate_header(scoreFile);
    scoreFile << ",\n";

    // Set Configrations
    GeneratingConfig config;
    // convert quater-note to long target
    config.holdMinLength = math::Fraction(1, 4); 
    // convert the notes which its note-number is 48 to Hit target
    config.keyAssign[48] = NoteType::Hit;

    // Write body
    generator.generate(scoreFile, 1, config);

    scoreFile << "\n}\n";
    scoreFile.close();
    midi.close();

    return 0;
}

```
上の例では，"sample.mid"というMIDIファイルのうち，トラック1のノーツを"notes.json"という譜面ファイルに書き出しています．
また，同時に譜面データに変換するときのフォーマットを指定しています．

上の例では，4分音符以上の長さのノーツを長押しノーツに，
また，ノートナンバーが48のノーツをhitターゲットにしています．


### 書き出し例
```json
{
    "tempo": [
        {
            "at": "1:1/1",
            "value": "4/4"
        }
    ],
    "time_signature": [
        {
            "at": "1:1/1",
            "value": 120
        }
    ],
    "notes": [
        {
            "type": "hit",
            "at": "1:1/1"
        },
        {
            "type": "hit",
            "at": "1:2/4"
        },
        {
            "type": "hit",
            "at": "1:4/4"
        },
        {
            "type": "hold",
            "at": "2:1/1",
            "end_at": "2:2/2"
        }
    ]
}

```


### 備考
三連符配置のあるMIDIファイルを譜面データに書き出すと，分母が巨大な数になる場合があります．
そのような場合には，以下の処理をMIDIを読み込む前に追加して下さい．
```
// Setting of Timing Adjustment
midireader.setAdjustmentAmplitude(1);
```
`MIDIReader::setAdjustmentAmplitude()`は，指定範囲内でノーツのタイミング補正を行う関数です．
引数で補正範囲を指定できますが，通常は1で大丈夫です．


### ライセンス (about License)
(This software is released under the MIT License, see LICENSE)

このリポジトリにあるソースコードはすべてMIT Licenseのもとで公開されています。
MIT LicenseについてはLICENSEを参照して下さい。
ざっくり説明すると以下のようになっています。

<dl>
    <dt>条件</dt>
    <dd>著作権とライセンスの表示</dd>
    <dt>許可</dt>
    <dd>商用利用</dd>
    <dd>修正</dd>
    <dd>配布</dd>
    <dd>個人利用</dd>
    <dt>禁止</dt>
    <dd>責任免除</dd>
</dl>
