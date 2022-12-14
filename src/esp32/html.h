#pragma once

// TODO: compile in dashboard.html and serve it directly. Having a real .html
// file allows easy development without reprogramming.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
  <style>
    html {
      font-family: Arial;
      font-size: 2.0rem;
      background-color: black;
      color: white;
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
  <div id="container">
    <div id="control-panel">
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
      <br />

      <div id="tune">
        <div>
          <label for="p">P</label>
          <input type="text" id="p" value="0">
        </div>
        <div>
          <label for="i">I</label>
          <input type="text" id="i" value="0">
        </div>
        <div>
          <label for="d">D</label>
          <input type="text" id="d" value="0">
        </div>
        <div>
          <label for="set-fan">Fan</label>
          <input type="text" id="set-fan" value="0">
        </div>
        <div>
          <label for="set-temp">Set temp</label>
          <input type="text" id="set-temp" value="0">
        </div>
      </div>
    </div>
    <div id="display-panel" style="width:800px; height:250px;"> </div>
  </div>

  <br />
  <form method="post" enctype="multipart/form-data" style="visibility: hidden;">
    <div>
      <div id="tester"></div><label for="file">Firmware for STM32G030C8T6</label>
      <input type="file" id="file" name="file" accept=".bin" />
    </div>
    <div>
      <button>Flash</button>
    </div>
  </form>
</body>

<script src="https://cdn.plot.ly/plotly-2.16.1.min.js"></script>

<script>

  const displayPanel = document.getElementById("display-panel");
  let data = [
    {
      y: [],
      mode: 'lines',
      name: 'Env',
    },
    {
      y: [],
      mode: 'lines',
      name: 'Bean',
    },
    {
      y: [],
      mode: 'lines',
      name: 'Set',
    },
    {
      y: [],
      yaxis: 'y2',
      mode: 'lines',
      name: 'Fan',
    },
  ];
  const layout = {
    width: 800,
    height: 500,
    xaxis: {
      title: 'Time, seconds',
    },
    yaxis: {
      title: 'Temperature, F',
      range: [50, 600],
    },
    yaxis2: {
      title: 'Fan',
      side: 'right',
      overlaying: 'y',
      range: [0, 255],
    }
  };

  Plotly.newPlot(displayPanel, data, layout);

  const socket = new WebSocket('ws://roaster.local:80/websocket');
  let seconds = 0;
  let setTemp = 0;
  socket.addEventListener('message', (event) => {
    const response = JSON.parse(event.data);
    document.getElementById("bt").innerHTML = Math.round(response.data.BT);
    document.getElementById("et").innerHTML = Math.round(response.data.ET);
    document.getElementById("at").innerHTML = Math.round(response.data.AT);
    document.getElementById("fault").innerHTML = response.data.FAULT;
    document.getElementById("fan").innerHTML = response.data.FAN;
    document.getElementById("updated").innerHTML = response.data.UPDATED;

    Plotly.extendTraces(displayPanel, {
      y: [[response.data.ET], [response.data.BT], [setTemp], [response.data.FAN]],
    }, [0, 1, 2, 3]);

    seconds++;
    const range = 120;
    const min = seconds > range ? (seconds - range) : 0;
    const max = seconds > range ? seconds : range;
    const layoutUpdate = {
      xaxis: {
        range: [min, max],
      },
    };
    Plotly.relayout(displayPanel, layoutUpdate);
  });

  socket.addEventListener('error', (event) => {
    document.getElementById("dashboard").classList.add("error");
  });

  function getData() {
    socket.send('{"command": "getData"}');
  }

  function sendParams(params) {
    const obj = { "command": "setParams", "params": params };
    socket.send(JSON.stringify(obj));
  }

  function reset() {
    sendParams({ "reset": true });
  }

  document.getElementById("reset").addEventListener("click", (event) => { reset(); });

  document.getElementById("p").addEventListener("change", (event) => {
    sendParams({ p: parseFloat(event.target.value) });
  });

  document.getElementById("i").addEventListener("change", (event) => {
    sendParams({ i: parseFloat(event.target.value) });
  });

  document.getElementById("d").addEventListener("change", (event) => {
    sendParams({ d: parseFloat(event.target.value) });
  });

  document.getElementById("set-fan").addEventListener("change", (event) => {
    sendParams({ fan: parseFloat(event.target.value) });
  });

  document.getElementById("set-temp").addEventListener("change", (event) => {
    setTemp = parseFloat(event.target.value);
    sendParams({ temp: setTemp });
  });

  setInterval(getData, 1000);

</script>
)rawliteral";