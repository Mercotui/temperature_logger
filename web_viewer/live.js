google.charts.load('current', {'packages':['line']});
google.charts.setOnLoadCallback(get_data);

function get_data(){
    var data = [[0,   4.2,  6.2,  3.4],
    [1,  37.8, 80.8, 41.8],
    [2,  30.9, 69.5, 32.4],
    [3,  25.4,   57, 25.7],
    [4,  11.7, 18.8, 10.5],
    [5,  11.9, 17.6, 10.4],
    [6,   8.8, 13.6,  7.7],
    [7,   7.6, 12.3,  9.6],
    [8,  12.3, 29.2, 10.6],
    [9,  16.9, 42.9, 14.8],
    [10, 12.8, 30.9, 11.6],
    [11,  5.3,  7.9,  4.7],
    [12,  6.6,  8.4,  5.2],
    [13,  4.8,  6.3,  3.6],
    [14,  4.8,  6.3,  3.6],
    [15,  4.8,  6.3,  3.6],
    [16,  4.8,  6.3,  3.6],
    [17,  4.8,  6.3,  3.6],
    [18,  4.8,  6.3,  3.6],
    [19,  4.8,  6.3,  3.6],
    [20,  4.8,  6.3,  3.6],
    [21,  4.8,  6.3,  3.6],
    [22,  4.8,  6.3,  3.6],
    [23,  4.8,  6.3,  3.6]];
    //do magic.h

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

    setTimeout(get_data, 300000); //5 minutes
}
