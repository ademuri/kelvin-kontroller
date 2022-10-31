#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
</head>
<body>
  <h3>Kelvin Kontroller</h3>
  <form method="post" enctype="multipart/form-data">
    <div>
      <label for="file">Firmware for STM32G030C8T6</label>
      <input type="file" id="file" name="file" accept=".bin" />
    </div>
    <div>
      <button>Flash</button>
    </div>
  </form>
</body>

<script>
const socket = new WebSocket('ws://192.168.86.38:80/websocket');
socket.addEventListener('message', (event) => {
  console.log(event.data);
});

function getData() {
  socket.send('{"command": "getData"}');  
}

setTimeout(getData, 1);
setInterval(getData, 5000);

</script>
)rawliteral";