function init_chart(){
    var ctx = document.getElementById("chart_canvas");
    var temp_chart = new Chart(ctx, {
        type: 'line',
        options: {
            scales: {
                xAxes: [{
                    type: 'linear',
                    position: 'bottom'
                }]
            }
        }
    });
    setInterval(drawChart(temp_chart),5000);
}

function drawChart(temp_chart) {
    temp_chart.data= {
        datasets: [{
            label: 'Scatter Dataset',
            data: [{ x: -10, y: 0 }, { x: 0, y: 10 }, { x: 10, y: 5 }]
        }, {label: 'Temperature',
            data: [{ x: -5, y: 10}, {x: 5, y:5}]}]
    }
    temp_chart.addData();
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
