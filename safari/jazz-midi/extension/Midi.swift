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
  private let splitter = Midi1()

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
    splitter.write(data)
    while let msg = splitter.read() {
      if msg.count > 3 {
        MusicDeviceSysEx(synth!, msg, UInt32(data.count))
      }
      else {
        MusicDeviceMIDIEvent(synth!, UInt32(msg[0]), msg.count > 1 ? UInt32(msg[1]) : 0, msg.count > 2 ? UInt32(msg[2]) : 0, 0)
      }
    }
  }
}

class MidiOutImpl {
  static var ports: [String: MidiOutImpl] = [:]

  var port: MIDIPortRef
  let dest: MIDIEndpointRef
  var refcount: UInt = 0
  
  init?(_ name: String) {
    var s: Unmanaged<CFString>?
    let m = MIDIGetNumberOfDestinations()
    for i in 0 ..< m {
      let device = MIDIGetDestination(i)
      MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &s)
      if let str = s {
        if String(str.takeUnretainedValue()) == name {
          dest = device
          port = MIDIPortRef()
          if MIDIOutputPortCreate(midiCLient(), "port" as CFString, &port) != 0 {
            return nil
          }
          MidiOutImpl.ports[name] = self
          return
        }
      }
    }
    return nil
  }

  deinit {
    MIDIPortDispose(port)
  }
}

func midiproc(_ pktlist: UnsafePointer<MIDIPacketList>, _ readProcRefCon: UnsafeMutableRawPointer?, _ srcConnRefCon: UnsafeMutableRawPointer?) -> Void {
  let impl: MidiInImpl = Unmanaged.fromOpaque(readProcRefCon!).takeUnretainedValue()
  let subs = impl.subscribers
  var packets = pktlist.pointee.packet
  var packet = UnsafeMutablePointer<MIDIPacket>(&packets)

  for _ in 0 ..< pktlist.pointee.numPackets {
    var data = packet.pointee.data
    let msg = [UInt8](UnsafeBufferPointer(start: &data.0, count: Int(packet.pointee.length)))
    for i in 0 ..< subs.count {
      subs[i].subscriber.onMidi(msg)
    }
    packet = MIDIPacketNext(packet)
  }
}

class MidiInImpl {
  static var ports: [String: MidiInImpl] = [:]

  var subscribers: [MidiInHW] = []
  var port: MIDIPortRef
  let src: MIDIEndpointRef

  init?(_ name: String) {
    var s: Unmanaged<CFString>?
    let m = MIDIGetNumberOfSources()
    for i in 0 ..< m {
      let device = MIDIGetSource(i)
      MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &s)
      if let str = s {
        if String(str.takeUnretainedValue()) == name {
          src = device
          port = MIDIPortRef()
          if MIDIInputPortCreate(midiCLient(), "port" as CFString, midiproc, Unmanaged.passUnretained(self).toOpaque(), &port) != 0 {
            return nil
          }
          if MIDIPortConnectSource(port, src, nil) != 0 {
            return nil
          }
          MidiInImpl.ports[name] = self
          return
        }
      }
    }
    return nil
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

class MidiInHW : MidiIn {
  let portname: String
  let port: MidiInImpl
  let subscriber: MidiSubscriber

  init?(_ name: String, _ sub: MidiSubscriber) {
    portname = name
    subscriber = sub
    if let impl = MidiInImpl.ports[name] {
      port = impl
      port.subscribers.append(self)
    }
    else if let impl = MidiInImpl(name) {
      port = impl
      port.subscribers.append(self)
    }
    else {
      return nil
    }
  }

  deinit {
    port.subscribers.removeAll(where: { $0 === self })
    if port.subscribers.count == 0 {
      MidiInImpl.ports.removeValue(forKey: portname)
    }
  }

  func name() -> String { return portname }
}

class Midi1 {
  private var data: [UInt8] = []
  
  static func len(_ c: UInt8) -> Int {
    switch c & 0xf0 {
      case 0x80, 0x90, 0xA0, 0xB0, 0xE0:
        return 3
      case 0xC0, 0xD0:
        return 2
      default:
        switch c {
          case 0xF2:
            return 3
          case 0xF1, 0xF3:
            return 2
          default:
            return 1
        }
    }
  }
  
  func write(_ msg: [UInt8]) {
    data.append(contentsOf: msg)
  }
  
  func read() -> [UInt8]? {
    while data.count > 0 && (data[0] < 0x80 || data[0] == 0xf7) {
      data.remove(at: 0)
    }
    if data.count > 0 {
      if data[0] == 0xf0 {
        for i in 1 ..< data.count {
          if data[i] == 0xf7 {
            let msg = Array(data[0 ... i])
            data.removeSubrange(0 ... i)
            return msg
          }
        }
      }
      else if data.count >= Midi1.len(data[0]) {
        for i in 1 ..< Midi1.len(data[0]) {
          if data[i] >= 0x80 {
            return nil
          }
          let msg = Array(data[0 ..< Midi1.len(data[0])])
          data.removeSubrange(0 ..< Midi1.len(data[0]))
          return msg
        }
      }
      return nil
    }
    return nil
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
    var N : Int32 = 0
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
    var inputs: [[String: String]] = []
    var outputs: [[String: String]] = [["name": "Apple DLS Synth", "manufacturer": "Apple", "version": "1.0"]]
    let n_dst = MIDIGetNumberOfDestinations()
    let n_src = MIDIGetNumberOfSources()
    for i in 0 ..< n_src {
      let device = MIDIGetSource(i)
      inputs.append(getDeviceInfo(device))
    }
    for i in 0 ..< n_dst {
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

  static func openMidiIn(_ name: String, _ sub: MidiSubscriber) -> MidiIn? {
    return MidiInHW(name, sub)
  }
}
