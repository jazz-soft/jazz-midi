import SafariServices

var port : MidiOut?

class SafariExtensionHandler: SFSafariExtensionHandler {
  override func messageReceived(withName messageName: String, from page: SFSafariPage, userInfo: [String : Any]?) {
    page.getPropertiesWithCompletionHandler { properties in
        //NSLog("Received: \(messageName) from: \(String(describing: properties?.url)) data: \(userInfo ?? [:])")
      if messageName == "unload" {
        NSLog("Unloading page: \(properties?.url)");
        //MidiOuts.remove(at: page)
      }
      else if messageName == "refresh" {
        page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["refresh", Midi.refresh()]])
      }
      else if var data = userInfo?["data"] as? [Any] {
        let slot = data[0] as! UInt
        data.remove(at: 0)
        NSLog("\(messageName) \(slot) : \(data)")
        if messageName == "play" {
          if port != nil {
            port!.send(data.map { $0 as! UInt8 })
          }
        }
        else if messageName == "openout" {
          if port == nil {
            NSLog("Opening: \(data[0])");
            port = MidiOutDLS()
          }
          else {
            NSLog("Already open: \(data[0])");
          }
          page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["openout", slot, data[0]]])
        }
        else if messageName == "openin" {
          page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["openin", slot, data[0]]])
        }
        else if messageName == "closeout" {
        }
        else if messageName == "closein" {
        }
      }
    }
  }

}
