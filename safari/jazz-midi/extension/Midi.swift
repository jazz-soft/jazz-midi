import Foundation
import AudioToolbox

class Midi {
  static func refresh() -> [String : [[String: String]]] {
    var inputs : [[String : String]] = [];
    var outputs : [[String : String]] = [["name" : "Apple DLS Synth"]];
    let n_dst = MIDIGetNumberOfDestinations()
    let n_src = MIDIGetNumberOfSources()
    var S : Unmanaged<CFString>?
    for var i in 0 ..< n_src {
      var info : [String : String] = [:]
      let device = MIDIGetSource(i)
      MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &S)
      if let str = S {
        info["name"] = String(str.takeUnretainedValue())
      }
      inputs.append(info)
    }
    for var i in 0 ..< n_dst {
      var info : [String : String] = [:]
      let device = MIDIGetDestination(i)
      MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &S)
      if let str = S {
        info["name"] = String(str.takeUnretainedValue())
      }
      outputs.append(info)
    }
    return ["ins": inputs, "outs" : outputs];
  }
}
