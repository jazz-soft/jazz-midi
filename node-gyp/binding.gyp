{
  "targets":
  [
    {
      "target_name": "jazz",
      "sources": ["jazz-midi.cpp", "../midi/Midi.cpp"],
      "include_dirs": ["../midi"],
      "conditions":
      [
        [
          "OS=='win'",
          {
            "sources": ["../midi/MidiWin32n.cpp", "../midi/MidiInfoWin32.cpp"],
            "defines": ["UNICODE"],
            "libraries": ["-lWinmm.lib"]
          }
        ],
        [
          "OS=='mac'",
          {
            "sources": ["../midi/MidiMacOSX.cpp"],
            "link_settings":
            {
              "libraries":
              [
                "CoreMIDI.framework",
                "AudioUnit.framework",
                "AudioToolbox.framework"
              ],
            }
          }
        ],
        [
          "OS=='linux'",
          {
            "sources": ["../midi/MidiALSA.cpp"],
            "cflags": ["-Wno-sign-compare"],
            "libraries": ["-lasound"]
          }
        ]
      ]
    }
  ]
}
