import Foundation
import AudioToolbox

var clientRef: MIDIObjectRef? = nil

func midiCLient() -> MIDIClientRef {
  if clientRef == nil {
    clientRef = MIDIClientRef()
    MIDIClientCreate("MIDI" as CFString, nil, nil, &clientRef!)
  }
  return clientRef!
}

protocol MidiOut {
  func name() -> String
  func send(_: [UInt8])
}

protocol MidiIn {
  func name() -> String
}

class MidiOutDLS : MidiOut {
  private var graph: AUGraph?
  private var synthNode: AUNode = 0
  private var outNode: AUNode = 0
  private var synth: AudioUnit?
  private var out: AudioUnit?

  init?() {
    NewAUGraph(&graph)
    if graph == nil {
      return nil
    }
    var status : OSStatus = 0
    var cd = AudioComponentDescription(
      componentType: kAudioUnitType_MusicDevice,
      componentSubType: kAudioUnitSubType_DLSSynth,
      componentManufacturer: kAudioUnitManufacturer_Apple,
      componentFlags: 0,
      componentFlagsMask: 0
    )
    status = AUGraphAddNode(graph!, &cd, &synthNode)
    if status != 0 {
      return nil
    }
    cd.componentType = kAudioUnitType_Output
    cd.componentSubType = kAudioUnitSubType_DefaultOutput
    status = AUGraphAddNode(graph!, &cd, &outNode)
    if status != 0 {
      return nil
    }
    status = AUGraphOpen(graph!)
    if status != 0 {
      return nil
    }
    status = AUGraphConnectNodeInput(graph!, synthNode, 0, outNode, 0)
    if status != 0 {
      return nil
    }
    status = AUGraphInitialize(graph!)
    if status != 0 {
      return nil
    }
    status = AUGraphStart(graph!)
    if status != 0 {
      return nil
    }
    AUGraphNodeInfo(graph!, synthNode, nil, &synth)
    AUGraphNodeInfo(graph!, outNode, nil, &out)
  }

  deinit {
    AUGraphStop(graph!)
    DisposeAUGraph(graph!)
  }

  func name() -> String { return Midi.DLS }

  func send(_ data: [UInt8]) {
    if data.count > 3 {
      MusicDeviceSysEx(synth!, data, UInt32(data.count))
    }
    else {
      MusicDeviceMIDIEvent(synth!, UInt32(data[0]), data.count > 1 ? UInt32(data[1]) : 0, data.count > 2 ? UInt32(data[2]) : 0, 0)
    }
  }

}

class MidiOutImpl {
  static var ports: [String: MidiOutImpl] = [:]

  var port: MIDIPortRef
  let dest: MIDIEndpointRef
  var refcount: UInt
  
  init?(_ name: String) {
    dest = MIDIGetDestination(0)
    port = MIDIPortRef()
    MIDIOutputPortCreate(midiCLient(), "port" as CFString, &port)
    refcount = 0
    MidiOutImpl.ports[name] = self
  }

  deinit {
    MIDIPortDispose(port)
  }

}

class MidiOutHW : MidiOut {
  let portname: String
  let port: MidiOutImpl

  init?(_ name: String) {
    portname = name
    if let impl = MidiOutImpl.ports[name] {
      port = impl
      port.refcount += 1
    }
    else if let impl = MidiOutImpl(name) {
      port = impl
      port.refcount += 1
    }
    else {
      return nil
    }
  }

  deinit {
    port.refcount -= 1
    if port.refcount == 0 {
      MidiOutImpl.ports.removeValue(forKey: portname)
    }
  }

  func name() -> String { return portname }

  func send(_ data: [UInt8]) {
    let size = data.count + 64
    let packetList = UnsafeMutablePointer<MIDIPacketList>.allocate(capacity: size)
    let packet = MIDIPacketListInit(packetList)
    MIDIPacketListAdd(packetList, size, packet, 0, data.count, data)
    MIDISend(port.port, port.dest, packetList)
    packetList.deallocate()
  }

}

class Midi {

  static let DLS = "Apple DLS Synth"

  static func getDeviceInfo(_ device: MIDIEndpointRef) -> [String: String] {
    var info: [String: String] = [:]
    var S: Unmanaged<CFString>?
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

  static func refresh() -> [String: [[String: String]]] {
    var inputs: [[String: String]] = [];
    var outputs: [[String: String]] = [["name": "Apple DLS Synth", "manufacturer": "Apple", "version": "1.0"]];
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
    return ["ins": inputs, "outs" : outputs]
  }

  static func openMidiOut(_ name: String) -> MidiOut? {
    if name == DLS {
      return MidiOutDLS()
    }
    return MidiOutHW(name)
  }

  static func openMidiIn(_ name: String) -> MidiIn? {
    return nil
  }

}
