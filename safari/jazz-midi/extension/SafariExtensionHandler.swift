import SafariServices

class SafariExtensionHandler: SFSafariExtensionHandler {
  override func messageReceived(withName messageName: String, from page: SFSafariPage, userInfo: [String : Any]?) {
    page.getPropertiesWithCompletionHandler { properties in
        NSLog("Received: \(messageName) from: \(String(describing: properties?.url)) data: \(userInfo ?? [:])")
      if messageName == "refresh" {
        page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["refresh", Midi.refresh()]])
      }
      else if messageName == "openout" {
        if let data = userInfo?["data"] {
          let array = data as? [Any]
          if let arr = array {
            page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["openout", arr[0], arr[1]]])
          }
        }
      }
      else if messageName == "openin" {
        if let data = userInfo?["data"] {
          let array = data as? [Any]
          if let arr = array {
            page.dispatchMessageToScript(withName: "", userInfo: ["data" : ["openin", arr[0], arr[1]]])
          }
        }
      }
    }
  }
}
