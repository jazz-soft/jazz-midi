var ver = '1.0';
var ports = 0;
var exchange;
var page = 0;

function publish(data) {
  if (exchange) {
    exchange.innerText += JSON.stringify(data) + '\n';
    document.dispatchEvent(new Event('jazz-midi-msg'));
  }
}

function delayed(data) {
  setTimeout(function() { publish(data); }, 0);
}

safari.self.addEventListener('message', function(e) {
  //console.log("### received from extension:", e.message.data);
  if (e.message.data[0] == "init") {
    if (!page) {
      page = e.message.data[1]
      delayed(['version', 0, ver]);
    }
  }
  else publish(e.message.data);
});

document.addEventListener('jazz-midi', function(e) {
  console.log("### received jazz-midi message:", e.detail);
  if (!e.detail) document.dispatchEvent(new Event('jazz-midi-msg'));
  if (!exchange) {
    exchange = document.createElement('div');
    exchange.id = 'jazz-midi-msg';
    document.body.appendChild(exchange);
    safari.extension.dispatchMessage("init", { "data": [0] });
    setInterval(function() { console.log('tick'); safari.extension.dispatchMessage("tick", { data: [page] }); }, 500);
  }
  if (page) {
    var v = e.detail.slice();
    if (v[0] === 'new') {
      ports++;
      delayed(['version', ports, ver]);
      return;
    }
    var name = v[0];
    v[0] = page;
    safari.extension.dispatchMessage(name, { "data": v });
  }
});
