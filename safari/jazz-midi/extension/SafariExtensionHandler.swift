import SafariServices

class SafariExtensionHandler: SFSafariExtensionHandler {
  override func messageReceived(withName messageName: String, from page: SFSafariPage, userInfo: [String : Any]?) {
    page.getPropertiesWithCompletionHandler { properties in
        //NSLog("Received: \(messageName) from: \(String(describing: properties?.url)) data: \(userInfo ?? [:])")
      if messageName == "refresh" {
        page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["refresh", Midi.refresh()]])
      }
      else if var data = userInfo?["data"] as? [Any] {
        let slot = data[0] as! UInt
        data.remove(at: 0)
        NSLog("\(messageName) \(slot) : \(data)")
        if messageName == "send" {
        }
        else if messageName == "openout" {
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
