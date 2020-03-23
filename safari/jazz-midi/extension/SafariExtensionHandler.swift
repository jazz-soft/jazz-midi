import SafariServices

class PageData {
  private static var pages: [UInt: PageData] = [:]
  private static var count: UInt = 0
  private let id: UInt
  private var page: SFSafariPage
  private var date: Date
  private var outputs: [UInt: MidiOut] = [:]
  private var inputs: [UInt: MidiIn] = [:]

  static func update(_ n: UInt, _ p: SFSafariPage) {
    let now = Date()
    if let pg = pages[n] {
      pg.date = now
      pg.page = p
    }
    pages = pages.filter({ now.timeIntervalSince($0.1.date) < 5 })
  }
  
  static func find(_ id: UInt) -> PageData {
    return pages[id]!
  }
  
  static func findPage(_ id: UInt) -> SFSafariPage? {
    return pages[id]?.page
  }
  
  init(_ p: SFSafariPage) {
    PageData.count += 1
    id = PageData.count
    page = p
    date = Date()
    PageData.pages[id] = self
  }
  
  func getId() -> UInt {
    return id
  }

  func send(_ slot: UInt, _ data: [UInt8]) {
    outputs[slot]?.send(data)
  }

  func openout(_ slot: UInt, _ name: String) -> String {
    if let str = outputs[slot]?.name() {
      if (str == name) {
        return name;
      }
    }
    if let port = Midi.openMidiOut(name) {
      outputs[slot] = port
    }
    if let str = outputs[slot]?.name() {
      return str;
    }
    else {
      return ""
    }
  }

  func openin(_ slot: UInt, _ name: String) -> String {
    if let str = inputs[slot]?.name() {
      if (str == name) {
        return name;
      }
    }
    if let port = Midi.openMidiIn(name, MidiSubscriber(id, slot)) {
      inputs[slot] = port
    }
    if let str = inputs[slot]?.name() {
      return str;
    }
    else {
      return ""
    }
  }

  func closeout(_ slot: UInt) {
    outputs.removeValue(forKey: slot)
  }

  func closein(_ slot: UInt) {
    inputs.removeValue(forKey: slot)
  }
}

class MidiSubscriber {
  let id: UInt
  let slot: UInt
  init(_ i: UInt, _ n: UInt) {
    id = i
    slot = n
  }
  
  func onMidi(_ midi: [UInt8]) {
    let data: [Any] = ["midi", slot, 0] + midi
    PageData.findPage(id)?.dispatchMessageToScript(withName: "", userInfo: ["data" : data])
  }
}

class SafariExtensionHandler: SFSafariExtensionHandler {
  override func messageReceived(withName messageName: String, from page: SFSafariPage, userInfo: [String : Any]?) {
    page.getPropertiesWithCompletionHandler { properties in
      if var data = userInfo?["data"] as? [Any] {
        var id: UInt = 0
        var slot: UInt = 0
        if data.count > 0 {
          id = data[0] as! UInt
          data.remove(at: 0)
        }
        if data.count > 0 {
          slot = data[0] as! UInt
          data.remove(at: 0)
        }
        if id == 0 {
          id = PageData(page).getId()
          page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["init", id]])
        }
        else {
          PageData.update(id, page)
          if messageName == "refresh" {
            page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["refresh", Midi.refresh()]])
          }
          else if messageName == "play" {
            PageData.find(id).send(slot, data.map { $0 as! UInt8 })
          }
          else if messageName == "openout" {
            let name = PageData.find(id).openout(slot, data[0] as! String)
            page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["openout", slot, name]])
          }
          else if messageName == "openin" {
            let name = PageData.find(id).openin(slot, data[0] as! String)
            page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["openin", slot, name]])
          }
          else if messageName == "closeout" {
            PageData.find(id).closeout(slot)
          }
          else if messageName == "closein" {
            PageData.find(id).closein(slot)
          }
        }
      }
    }
  }
}
