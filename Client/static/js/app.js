
window.addEventListener('DOMContentLoaded', () => {
  const chartData = [
    {
      label: "Temperature",
      color: 'black',
      type: "line",
      borderColor: "rgba(0, 0, 0, 1)",
      data: [],
      yAxisID : "y",
    },
    {
      label: "Humdidity",
      color: 'black',
      type: "bar",
      backgroundColor: "rgba(0, 160, 0, 1)",
      data: [],
      yAxisID : "y1",
    },
  ];
  
  const ctx = document.getElementById('myChart').getContext('2d');
  const myChart = new Chart(ctx, {
    type: 'bar',
    data: {
      labels: [],
      datasets: chartData,
    },
    options: {
      responsive: true,
      scales: {
        x: {
          grid: {
            display: false, // Set display to false to hide the x-axis grid lines
          },
          display: true,
          title: {
            display: true,
            text: 'Time',
            color: 'black',
          },
        },
        y: {
          grid: {
            display: false, // Set display to false to hide the y-axis grid lines
          },
          display: true,
          position: "left",
          title: {
            display: true,
            text: 'Temperature',
            color: 'black',
          },
          ticks: {
            color: 'black',
            callback: function(value, index, values) {
              return value + ' °C'; 
            },
          },
        },
        y1: {
          grid: {
            display: false, // Set display to false to hide the y-axis grid lines
          },
          display: true,
          position: 'right',
          min : 0,
          max : 100,
          title: {
            display: true,
            text: 'Humidity',
            color: 'black',
          },
          ticks: {
            color: 'black',
            callback: function(value, index, values) {
              return value + ' %'; 
            },
          },
        },
      },
    },
  });

  function addData(label, value1, value2) {
    myChart.data.labels.push(label);
    myChart.data.datasets[0].data.push(value1);
    myChart.data.datasets[1].data.push(value2);
    myChart.update();

    // Show only 10 data points
    if (myChart.data.labels.length > 10) {
      removeFirstData();
    }
  }

  function removeFirstData() {
    myChart.data.labels.shift();
    myChart.data.datasets.forEach((dataset) => {
      dataset.data.shift();
    });
  }

  // MQTT configuration
  const socket = io.connect();

  // Receive details from server
  socket.on('updateSensorData', function (msg) {
    console.log(
      'Received sensorData :: ' +
        msg.date +
        ' :: ' +
        msg.value1 +
        ' :: ' +
        msg.value2
    );

    addData(msg.date, msg.value1, msg.value2);
  });
});
