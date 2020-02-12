var ports = [];
var exchange;

function createPort() {
  var port = chrome.runtime.connect();
  var id = ports.length;
  port.onDisconnect.addListener(function() {
    document.dispatchEvent(new Event('jazz-midi-msg'));
  });
  port.onMessage.addListener(function(v) {
    if (v[0] !== 'refresh') v.splice(1, 0, id);
    exchange.innerText += JSON.stringify(v) + '\n';
    document.dispatchEvent(new Event('jazz-midi-msg'));
  });
  ports.push(port);
}

document.addEventListener('jazz-midi', function(e) {
  if (!e.detail) document.dispatchEvent(new Event('jazz-midi-msg'));
  if (!exchange) {
    exchange = document.createElement('div');
    exchange.id = 'jazz-midi-msg';
    //exchange.style.visibility = 'hidden';
    document.body.appendChild(exchange);
    createPort();
  }
  if (!e.detail) return;
  var n = 0;
  var v = e.detail.slice();
  if (v[0] === 'new') {
    createPort(); return;
  }
  if (v[0] !== 'refresh' && v[0] !== 'watch' && v[0] !== 'unwatch') n = v.splice(1, 1);
  if (ports[n]) ports[n].postMessage(v);
});
