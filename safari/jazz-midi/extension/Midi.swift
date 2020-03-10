import Foundation
import AudioToolbox

protocol MidiOut {
    func send(_ : [UInt8])
}

class MidiOutDLS : MidiOut {
  private var graph : AUGraph?
  private var synthNode : AUNode = 0;
  private var outNode : AUNode = 0;
  private var synth : AudioUnit?
  private var out : AudioUnit?

  init?() {
    NewAUGraph(&graph)
    if (graph == nil) {
      //print("Cannot create AUGraph");
      return nil
    }
    var status : OSStatus = 0;
    var cd = AudioComponentDescription(
      componentType: kAudioUnitType_MusicDevice,
      componentSubType: kAudioUnitSubType_DLSSynth,
      componentManufacturer: kAudioUnitManufacturer_Apple,
      componentFlags: 0,
      componentFlagsMask: 0
    )
    status = AUGraphAddNode(graph!, &cd, &synthNode)
    if (status != 0) {
      //print("Cannot create synth node: \(status)");
      return nil
    }
    cd.componentType = kAudioUnitType_Output;
    cd.componentSubType = kAudioUnitSubType_DefaultOutput;
    status = AUGraphAddNode(graph!, &cd, &outNode);
    if (status != 0) {
      //print("Cannot create output node: \(status)");
      return nil
    }
    status = AUGraphOpen(graph!);
    if (status != 0) {
      //print("Cannot open AUGraph: \(status)");
      return nil
    }
    status = AUGraphConnectNodeInput(graph!, synthNode, 0, outNode, 0);
    if (status != 0) {
      //print("Cannot connect nodes: \(status)");
      return nil
    }
    status = AUGraphInitialize(graph!);
    if (status != 0) {
      //print("Cannot initialize AUGraph: \(status)");
      return nil
    }
    status = AUGraphStart(graph!);
    if (status != 0) {
      //print("Cannot start AUGraph: \(status)");
      return nil
    }
    AUGraphNodeInfo(graph!, synthNode, nil, &synth);
    AUGraphNodeInfo(graph!, outNode, nil, &out);
  }

  deinit {
    AUGraphStop(graph!)
    DisposeAUGraph(graph!)
  }

  func send(_ data: [UInt8]) {
    MusicDeviceMIDIEvent(synth!, UInt32(data[0]), UInt32(data[1]), UInt32(data[2]), 0)
  }

}

class Midi {

  let DLS = "Apple DLS Synth"

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
