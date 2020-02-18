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

safari.self.addEventListener('jazz-midi-ext', function(e) {
  console.log('received from extension:', e);
});

document.addEventListener('jazz-midi', function(e) {
console.log("### received jazz-midi message:", e.detail);
  if (!e.detail) document.dispatchEvent(new Event('jazz-midi-msg'));
  if (!exchange) {
    exchange = document.createElement('div');
    exchange.id = 'jazz-midi-msg';
    document.body.appendChild(exchange);
  }
  if (!e.detail) {
    delayed(['version', 0, ver]);
    return;
  }
  var n = 0;
  var v = e.detail.slice();
  if (v[0] === 'new') {
    ports++;
    delayed(['version', ports, ver]);
    return;
  }
  console.log("### sending data to the extension:", v);
  safari.extension.dispatchMessage("data", { "data": v });
  console.log("### -");
});

console.log("### jazz-midi extension loaded!!!");
