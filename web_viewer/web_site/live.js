function onload(){
    var chart = new CanvasJS.Chart("chart_div",{
        zoomEnabled: true,
        title: {
            text: "Temperatures from datatbase"
        },
        toolTip: {
            shared: true

        },
        legend: {
            verticalAlign: "top",
            horizontalAlign: "center",
            fontSize: 14,
            fontWeight: "bold",
            fontFamily: "calibri",
            fontColor: "dimGrey"
        },
        axisX: {
            title: "chart updates every 3 secs"
        },
        axisY:{
            prefix: '$',
            includeZero: false
        },
        data: [{
            // dataSeries1
            type: "line",
            xValueType: "dateTime",
            showInLegend: true,
            name: "Company A",
            dataPoints: dataPoints1
        },
        {
            // dataSeries2
            type: "line",
            xValueType: "dateTime",
            showInLegend: true,
            name: "Company B" ,
            dataPoints: dataPoints2
        }],
        legend:{
            cursor:"pointer",
            itemclick : function(e) {
                if (typeof(e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
                    e.dataSeries.visible = false;
                }
                else {
                    e.dataSeries.visible = true;
                }
                chart.render();
            }
        }
    });

    load_data();
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
