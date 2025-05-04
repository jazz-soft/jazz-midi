chrome.runtime.onConnect.addListener(function(port) {
console.log('port.sender.tab.id', port.sender.tab.id);
  //chrome.action.show(port.sender.tab.id);
  var native = chrome.runtime.connectNative('com.jazz_soft.jazz_midi');
  native.onDisconnect.addListener(function() {
    chrome.action.setPopup({tabId: port.sender.tab.id, popup: 'bad.html'});
    chrome.action.setIcon({tabId: port.sender.tab.id, path: 'bad19.png'});
    port.disconnect();
  });
  port.onDisconnect.addListener(function(){native.disconnect();});
  native.onMessage.addListener(function(msg){
    port.postMessage(msg);}
  );
  port.onMessage.addListener(function(msg){native.postMessage(msg);});
  native.postMessage(['version']);
});
