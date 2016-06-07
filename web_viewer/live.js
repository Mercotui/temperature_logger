google.charts.load('current', {'packages':['line']});
google.charts.setOnLoadCallback(load_data);

function load_data(){
    var data_file = "TODO: NEED A LINK HERE";
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
    //TODO; DO MAGIC HERE

    draw_chart(data);
}

function draw_chart(data_array) {
    var data = new google.visualization.DataTable();
    data.addColumn('number', 'Time (hours)');
    data.addColumn('number', 'Weather');
    data.addColumn('number', 'RaspberryTAU');
    data.addColumn('number', 'Titus');

    data.addRows(data_array);

    var options = {
        chart: {
            title: 'Temperature',
            subtitle: 'in degrees celsius'
        },
        //vAxis.minorGridlines.units:
        width: 1500,
        height: 500
    };

    var chart = new google.charts.Line(document.getElementById('chart_div'));

    chart.draw(data, options);

    setTimeout(load_data, 300000); //5 minutes
}
