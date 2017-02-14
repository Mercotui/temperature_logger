var samples = 10;
var speed = 1000;
var values = [];
var values2 = [];
var labels = [];
var charts = [];
var temperature_chart;

values.length = samples;
labels.length = samples;
values.fill(0);
labels.fill(0);

values2.length = samples;
values2.fill(0);

function init_chart(){
    var canvas = document.getElementById('chart_canvas');
    var ctx = canvas.getContext('2d');

    temperature_chart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: labels,
        datasets: [{
                label: "Weather",
                fillColor: "rgba(50,50,50,0.5)",
                strokeColor: "rgba(220,220,220,1)",
                pointColor: "rgba(220,220,220,1)",
                pointStrokeColor: "#fff",
                data: values
            },{
                label: "Room",
                fill:false,
                borderColor: "rgba(75,192,192,1)",
                fillColor: "rgba(151,187,205,0.2)",
                strokeColor: "rgba(100,100,205,1)",
                pointColor: "rgba(151,187,205,1)",
                pointStrokeColor: "#fff",
                data: values2
            }]
    }
    });

    setInterval(function() {
        requestAnimationFrame(chart_add);
      }, speed);
}

function chart_add(){
    var d = new Date();
    var label = d.getSeconds();
    labels.push(label);
    labels.shift();

    value = Math.random() * 50;
    values.push(value);
    values.shift();

    value = Math.random() * 50;
    values2.push(value);
    values2.shift();
    temperature_chart.update();
}

function load_data(){
    setTimeout(load_data, 10000);//300000); //5 minutes

    var data_file = window.location.origin + "/data.json";
    var http_request = new XMLHttpRequest();
    try{
        // Opera 8.0+, Firefox, Chrome, Safari
        http_request = new XMLHttpRequest();
    }catch (e){
        // Internet Explorer Browsers
        try{
            http_request = new ActiveXObject("Msxml2.XMLHTTP");

        }catch (e) {

            try{
                http_request = new ActiveXObject("Microsoft.XMLHTTP");
            }catch (e){
                // Something went wrong
                alert("Your browser broke!");
                return false;
            }

        }
    }

    http_request.onreadystatechange = function (){
        if (http_request.readyState == 4){
            // Javascript function JSON.parse to parse JSON data
            var jsonObj = JSON.parse(http_request.responseText);
            parse_data(jsonObj);
        }
    }
    http_request.open("GET", data_file, true);
    http_request.send();
}

function parse_data(jsonObj){
    dataPoints1.push({
        x: time.getTime(),
        y: yValue1
    });
    dataPoints2.push({
        x: time.getTime(),
        y: yValue2
    });

    chart.render();
}

init_chart();
