var ver = '1.0';
var ports = 0;
var exchange;

function publish(data) {
  exchange.innerText += JSON.stringify(data) + '\n';
  document.dispatchEvent(new Event('jazz-midi-msg'));
}

function delayed(data) {
  setTimeout(function() { publish(data); }, 0);
}

safari.self.addEventListener('message', function(e) {
  console.log("### received from extension:", e.message.data);
  publish(e.message.data);
});

document.addEventListener('jazz-midi', function(e) {
  console.log("### received jazz-midi message:", e.detail);
  if (!e.detail) document.dispatchEvent(new Event('jazz-midi-msg'));
  if (!exchange) {
    exchange = document.createElement('div');
    exchange.id = 'jazz-midi-msg';
    document.body.appendChild(exchange);
    setInterval(function() { safari.extension.dispatchMessage("tick"); }, 5000);
  }
  if (!e.detail) {
    delayed(['version', 0, ver]);
    return;
  }
  var v = e.detail.slice();
  if (v[0] === 'new') {
    ports++;
    delayed(['version', ports, ver]);
    return;
  }
  safari.extension.dispatchMessage(v[0], { "data": v.slice(1) });
});
