chrome.runtime.onConnect.addListener(function(port) {
  chrome.pageAction.show(port.sender.tab.id);
  var native = chrome.runtime.connectNative('com.jazz_soft.jazz_midi');
  native.onDisconnect.addListener(function() {
    chrome.pageAction.setPopup({tabId: port.sender.tab.id, popup: 'bad.html'});
    chrome.pageAction.setIcon({tabId: port.sender.tab.id, path: 'bad19.png'});
    port.disconnect();
  });
  port.onDisconnect.addListener(function(){native.disconnect();});
  native.onMessage.addListener(function(msg){
    port.postMessage(msg);}
  );
  port.onMessage.addListener(function(msg){native.postMessage(msg);});
  native.postMessage(['version']);
});
