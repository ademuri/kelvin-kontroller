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

    #csv {
      width: 35em;
      margin-top: 1em;
    }

    input[type="text"] {
      width: 3em;
      font-size: 0.7rem;
    }

    #tune {
      float: left;
      width: 15%;
    }

    #display-panel {
      float: left;
      width: 85%;
    }

    #reset-graph-container {
      margin-top: 3em;
    }

    #fatal-fault {
      background-color: green;
    }

    #fatal-fault.fault {
      background-color: red;
    }

    #current-curve {
      height: 8em;
      width: 20em;
    }

    #curves label {
      display: block;
    }

    #curve-key {
      font-size: 0.5em;
      font-style: italic;
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
          <label for="heater">Heater:</label>
          <span id="heater"></span>
        </div>
        <div>
          <label for="fault">Fault:</label>
          <span id="fault"></span>
        </div>
        <div>
          <label for="fatal-fault">Running: </label>
          <span id="fatal-fault"></span>
        </div>
        <div>
          <label for="updated">Updated:</label>
          <span id="updated"></span>
        </div>
        <button id="reset">Reset controller</button>
      </div>
      <br />

      <div id="tune-and-graph">
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
          <div>
            <button id="autoscale">Autoscale</button>
          </div>

          <div>
            <button id="first-crack-start">First crack start</button>
          </div>
          <div>
            <button id="first-crack-end">First crack end</button>
          </div>
          <div>
            <button id="second-crack-start">Second crack start</button>
          </div>
          <div>
            <button id="second-crack-end">Second crack end</button>
          </div>

          <div>
            <button id="cooldown">Cooldown</button>
          </div>

          <div id="reset-graph-container">
            <button id="reset-graph">Reset graph</button>
          </div>
        </div>
        <div id="display-panel"></div>
      </div>
    </div>
  </div>

  <div>
    <textarea id="csv"></textarea>
  </div>
  <br />

  <div id="curves">
    <label for="current-curve">Curve</label>
    <div id="curve-key">Time, Temp, Fan=(prev or 255)</div>
    <textarea id="current-curve"></textarea>
    <button id="run-curve">Run curve</button>
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

<script src="https://cdn.plot.ly/plotly-2.18.2.min.js"></script>

<script>

  function getInitialData() {
    return [
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
        name: 'Heater',
      },
      {
        y: [],
        yaxis: 'y2',
        mode: 'lines',
        name: 'Fan',
      },
    ];
  }

  const displayPanel = document.getElementById("display-panel");
  let data = getInitialData();
  const layout = {
    width: 1000,
    height: 500,
    xaxis: {
      title: 'Time, seconds',
    },
    yaxis: {
      title: 'Temperature, F',
      range: [50, 800],
    },
    yaxis2: {
      title: 'PWM',
      side: 'right',
      overlaying: 'y',
      range: [0, 1.01],
    }
  };

  Plotly.newPlot(displayPanel, data, layout);
  resetCsv();

  const socket = new WebSocket('ws://roaster.local:80/websocket');
  let seconds = 0;
  let setTemp = 0;
  let beanTemp = 0;
  // https://marriedtotheseacomics.com/post/103884129802/stop-hitting-yourself-from-married-to-the-sea
  let ignoreNextRelayout = true;
  let autoscaleX = true;
  let annotationIndex = 0;

  // Load tuning constants from local storage
  const prevP = localStorage.getItem("p");
  if (prevP !== null) {
    document.getElementById("p").value = prevP;
  }

  const prevI = localStorage.getItem("i");
  if (prevI !== null) {
    document.getElementById("i").value = prevI;
  }

  const prevD = localStorage.getItem("d");
  if (prevD !== null) {
    document.getElementById("d").value = prevD;
  }

  socket.addEventListener('message', (event) => {
    const response = JSON.parse(event.data);
    beanTemp = Math.round(response.data.BT);
    const envTemp = Math.round(response.data.ET);
    const ambientTemp = Math.round(response.data.AT);
    const heater = Math.round(response.data.HEATER * 100) / 100;
    const fan = Math.round(response.data.FAN / 255 * 100) / 100;

    document.getElementById("bt").innerHTML = beanTemp;
    document.getElementById("et").innerHTML = envTemp;
    document.getElementById("at").innerHTML = ambientTemp;
    document.getElementById("fault").innerHTML = response.data.FAULT;
    document.getElementById("fatal-fault").innerHTML = !response.data.FATAL_FAULT;
    document.getElementById("fatal-fault").classList =
      response.data.FATAL_FAULT === true ? ['fault'] : [];
    document.getElementById("fan").innerHTML = response.data.FAN;
    document.getElementById("heater").innerHTML = response.data.HEATER;
    document.getElementById("updated").innerHTML = response.data.UPDATED;

    Plotly.extendTraces(displayPanel, {
      y: [[envTemp], [beanTemp], [setTemp], [heater], [fan]],
    }, [0, 1, 2, 3, 4]);
    document.getElementById("csv").value += seconds + ", " +
      envTemp + ", " + beanTemp + ", " +
      setTemp + ", " + heater + ", " +
      fan + "\n";

    seconds++;
    if (autoscaleX) {
      const range = 600;
      const min = seconds > range ? (seconds - range) : 0;
      const max = seconds > range ? seconds : range;
      const layoutUpdate = {
        xaxis: {
          range: [min, max],
        },
      };
      ignoreNextRelayout = true;
      Plotly.relayout(displayPanel, layoutUpdate);
    }
  });

  displayPanel.on('plotly_relayout', (event) => {
    if (ignoreNextRelayout) {
      ignoreNextRelayout = false;
    } else {
      autoscaleX = false;
    }
  });

  // See https://plotly.com/javascript/zoom-events/
  document.getElementById("autoscale").addEventListener("click", (event) => {
    autoscaleX = true;
    ignoreNextRelayout = true;
  });

  function resetCsv() {
    document.getElementById("csv").value = "seconds, env_temp, bean_temp, set_temp, heater, fan\n";
  }

  function resetGraph() {
    data = getInitialData();
    seconds = 0;
    Plotly.react(displayPanel, data, layout);
    resetCsv();
  }
  document.getElementById("reset-graph").addEventListener("click", (event) => { resetGraph(); });

  function addAnnotation(text) {
    // This API is sort-of documented here: https://plotly.com/javascript/text-and-annotations/#styling-and-formatting-annotations
    const annotation = {
      x: seconds,
      y: beanTemp,
      text: text,
    };
    Plotly.relayout(displayPanel, `annotations[${annotationIndex++}]`, annotation);
  }
  document.getElementById("first-crack-start").addEventListener("click", (event) => {
    addAnnotation(`First crack start (${beanTemp}F)`);
  });
  document.getElementById("first-crack-end").addEventListener("click", (event) => {
    addAnnotation(`First crack end (${beanTemp}F)`);
  });
  document.getElementById("second-crack-start").addEventListener("click", (event) => {
    addAnnotation(`Second crack start (${beanTemp}F)`);
  });
  document.getElementById("second-crack-end").addEventListener("click", (event) => {
    addAnnotation(`Second crack end (${beanTemp}F)`);
  });

  document.getElementById("cooldown").addEventListener("click", (event) => {
    document.getElementById("set-fan").value = 255;
    document.getElementById("set-temp").value = 0;
    sendParams({ fan: 255, temp: 0 });
    addAnnotation(`Cooldown (${beanTemp}F)`);
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
    const p = parseFloat(event.target.value);
    sendParams({ p: p });
    localStorage.setItem("p", p);
  });

  document.getElementById("i").addEventListener("change", (event) => {
    const i = parseFloat(event.target.value);
    sendParams({ i: i });
    localStorage.setItem("i", i);
  });

  document.getElementById("d").addEventListener("change", (event) => {
    const d = parseFloat(event.target.value);
    sendParams({ d: d });
    localStorage.setItem("d", d);
  });

  document.getElementById("set-fan").addEventListener("change", (event) => {
    sendParams({ fan: parseFloat(event.target.value) });
  });

  document.getElementById("set-temp").addEventListener("change", (event) => {
    setTemp = parseFloat(event.target.value);
    sendParams({ temp: setTemp });
  });

  document.getElementById("run-curve").addEventListener("click", (event) => {
    const curveElement = document.getElementById("current-curve");
    localStorage.setItem("current-curve", curveElement.value);
    curveElement.classList = [];
    let curve = [];

    let error = false;
    let prevFan = 255;
    curveElement.value.split("\n").forEach((line) => {
      let point = {};
      const tokens = line.split(",");
      if (tokens.length < 2 || tokens.length > 3) {
        curveElement.classList = ["error"];
        error = true
        // forEach return early
        return;
      }

      point['time'] = parseInt(tokens[0]);
      if (point['time'] == NaN) {
        curveElement.classList = ["error"];
        error = true;
        return;
      }
      point['temp'] = parseInt(tokens[1]);
      if (point['temp'] == NaN) {
        curveElement.classList = ["error"];
        error = true;
        return;
      }
      if (tokens.length == 3) {
        point['fan'] = parseInt(tokens[2]);
        if (point['fan'] == NaN) {
          curveElement.classList = ["error"];
          error = true;
          return;
        }
        prevFan = point['fan'];
      } else {
        point['fan'] = prevFan;
      }

      curve.push(point);
    });

    if (error) {
      return;
    }

    resetGraph();
    const obj = { "command": "runCurve", "curve": curve };
    socket.send(JSON.stringify(obj));
  });
  document.getElementById("current-curve").value = 
    localStorage.getItem("current-curve");

  setInterval(getData, 1000);

</script>
)rawliteral";