import Foundation
import AudioToolbox

class Midi {

  static func getDeviceInfo(_ device : MIDIEndpointRef) -> [String : String] {
    var info : [String : String] = [:]
    var S : Unmanaged<CFString>?
    MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &S)
    if let str = S {
      info["name"] = String(str.takeUnretainedValue())
    }
    MIDIObjectGetStringProperty(device, kMIDIPropertyManufacturer, &S)
    if let str = S {
      info["manufacturer"] = String(str.takeUnretainedValue())
    }
    else {
      info["manufacturer"] = "Unknown"
    }
    var N : Int32 = 0;
    MIDIObjectGetIntegerProperty(device, kMIDIPropertyDriverVersion, &N)
    if N == 0 || N >= 256 {
      info["version"] = "\(N >> 8).\(N & 0xff)"
    }
    else {
      info["version"] = "\(N).0"
    }
    return info
  }
  
  static func refresh() -> [String : [[String: String]]] {
    var inputs : [[String : String]] = [];
    var outputs : [[String : String]] = [["name" : "Apple DLS Synth", "manufacturer" : "Apple", "version" : "1.0"]];
    let n_dst = MIDIGetNumberOfDestinations()
    let n_src = MIDIGetNumberOfSources()
    for var i in 0 ..< n_src {
      let device = MIDIGetSource(i)
      inputs.append(getDeviceInfo(device))
    }
    for var i in 0 ..< n_dst {
      let device = MIDIGetDestination(i)
      outputs.append(getDeviceInfo(device))
    }
    return ["ins": inputs, "outs" : outputs];
  }
  
}
