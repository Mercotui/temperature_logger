var samples = 48;
var speed   = 1000;
var values  = [];
var values2 = [];
var labels  = [];
var charts  = [];
var temperature_chart;
var data_sets = new Array ();

values.length = samples;
labels.length = samples;
values.fill (0);
labels.fill (0);

function init_chart () {
    var canvas = document.getElementById ('chart_canvas');
    var ctx    = canvas.getContext ('2d');

    temperature_chart = new Chart (ctx, {
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
            }]
        }
    });

    setInterval (load_data (), speed);
}

function load_data () {
    setTimeout (load_data, 10000); // 300000); //5 minutes

    var data_file    = window.location.origin + "/data.json";
    var http_request = new XMLHttpRequest ();
    try {
        // Opera 8.0+, Firefox, Chrome, Safari
        http_request = new XMLHttpRequest ();
    } catch (e) {
        // Internet Explorer Browsers
        try {
            http_request = new ActiveXObject ("Msxml2.XMLHTTP");

        } catch (e) {

            try {
                http_request = new ActiveXObject ("Microsoft.XMLHTTP");
            } catch (e) {
                // Something went wrong
                alert ("Your browser broke!");
                return false;
            }
        }
    }

    http_request.onreadystatechange = function () {
        if (http_request.readyState == 4) {
            // Javascript function JSON.parse to parse JSON data
            var jsonObj = JSON.parse (http_request.responseText);
            parse_data (jsonObj);
        }
    };
    http_request.open ("GET", data_file, true);
    http_request.send ();
}

function parse_data (root) {
    var time_slots = new Array (samples);
    for (var i = 0; i < time_slots.length; i++) {
        time_slots[i] = new Array ();
    }

    for (var table_name in root) {
        if (root.hasOwnProperty (table_name)) {
            var table = root[table_name];
            for (var timestamp_str in table) {
                var temperature = table[timestamp_str];
                var timestamp   = new Date (timestamp_str);

                insert_data (time_slots, timestamp, temperature);
            }
        }
    }

    chart_data (time_slots);
}

function insert_data (time_slots, timestamp, temperature) {
    for (var time_slot_idx = time_slots.length - 1; time_slot_idx >= 0; time_slot_idx--) {
        var time_slot = new Date ();
        time_slot.setHours (time_slot.getHours () - (time_slot_idx + 1));

        if (time_slot > timestamp) {
            time_slots[time_slot_idx].push (temperature);
        }
    }
}

function chart_data (time_slots) {
    values.length = time_slots.length;

    for (var idx = time_slots.length - 1; idx >= 0; idx--) {
        if (time_slots[idx].length == 0) {
            values[idx] = NaN;
        } else {
            var sum = 0;
            for (var temperature of time_slots[idx]) {
                sum += temperature;
            }
            values[idx] = sum / time_slots[idx].length;
        }
    }

    labels = new Array (values.length);
    for (var time_slot_idx = values.length - 1; time_slot_idx >= 0; time_slot_idx--) {
        var time_slot         = new Date ();
        labels[time_slot_idx] = time_slot.getHours () - (time_slot_idx + 1);
    }

    // data_sets.push ();
    /*{
      label: "Room",
      fill: false,
      borderColor: "rgba(75,192,192,1)",
      fillColor: "rgba(151,187,205,0.2)",
      strokeColor: "rgba(100,100,205,1)",
      pointColor: "rgba(151,187,205,1)",
      pointStrokeColor: "#fff",
      data: values2
    }*/

    /*var d     = new Date ();
    var label = d.getSeconds ();
    labels.push (label);
    labels.shift ();

    value = Math.random () * 50;
    values.push (value);
    values.shift ();

    value = Math.random () * 50;
    values2.push (value);
    values2.shift ();*/

    requestAnimationFrame (function () {
        temperature_chart.update ();
    });
}

init_chart ();
