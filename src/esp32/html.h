#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <style>
    .html {
      font-family: Arial;
      font-size: 2.0rem;
    }
    .dashboard label {
      font-style: italic;
    }
    .error {
      background-color: red;
    }
  </style>
</head>
<body>
  <h3>Kelvin Kontroller</h3>
  <div id="dashboard">
    <div>
      <label for="bt">Bean temp:</label>
      <span id="bt"></span>
    </div>
    <div>
      <label for="et">Env temp:</label>
      <span id="et"></span>
    </div>
    <div>
      <label for="at">Ambient temp:</label>
      <span id="at"></span>
    </div>
    <div>
      <label for="fan">Fan speed:</label>
      <span id="fan"></span>
    </div>
    <div>
      <label for="fault">Fault:</label>
      <span id="fault"></span>
    </div>
    <div>
      <label for="updated">Updated:</label>
      <span id="updated"></span>
    </div>
    <button id="reset">Reset</button>
  </div>

  <br/>
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
const socket = new WebSocket('ws://roaster.local:80/websocket');
socket.addEventListener('message', (event) => {
  const response = JSON.parse(event.data);
  document.getElementById("bt").innerHTML = Math.round(response.data.BT);
  document.getElementById("et").innerHTML = Math.round(response.data.ET);
  document.getElementById("at").innerHTML = Math.round(response.data.AT);
  document.getElementById("fault").innerHTML = response.data.FAULT;
  document.getElementById("fan").innerHTML = response.data.FAN;
  document.getElementById("updated").innerHTML = response.data.UPDATED;
});

socket.addEventListener('error', (event) => {
  document.getElementById("dashboard").classList.add("error");
});

function getData() {
  socket.send('{"command": "getData"}');  
}

function reset() {
  socket.send('{"command": "setParams", "params": {"reset": true}}');
}

document.getElementById("reset").addEventListener("click", (event) => { reset(); });

setInterval(getData, 1000);

</script>
)rawliteral";