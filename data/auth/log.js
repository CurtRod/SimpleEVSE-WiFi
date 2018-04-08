var websock = null;
var logdata;
var wsUri;

function twoDigits(value) {
   if(value < 10) {
    return "0" + value;
   }
   return value;
}

function initTable() {
  jQuery(function($) {
    FooTable.init("#latestlogtable", {
        columns: [
            {
            "name": "timestamp",
            "title": "Date",
            "parser": function(value) {
              var vuepoch = new Date(value * 1000);
              var formatted = twoDigits(vuepoch.getUTCDate())
                  + "." + twoDigits(vuepoch.getUTCMonth() + 1) 
                  + "." + twoDigits(vuepoch.getUTCFullYear())
                  + " " + twoDigits(vuepoch.getUTCHours())
                  + ":" + twoDigits(vuepoch.getUTCMinutes())
                  + ":" + twoDigits(vuepoch.getUTCSeconds());
                return formatted;
            },
            "sorted": true,
            "direction": "DESC"
            },
		  {
			"name": "duration",
			"title": "Duration",
			"parser": function(value) {
				if(value === 0) {
					return "<span class=\"glyphicon glyphicon-repeat\"></span>";
				}
				var seconds = (value / 1000).toFixed(0);
				var minutes = Math.floor(seconds / 60);
				var hours = "";
				if (minutes > 59) {
					hours = Math.floor(minutes / 60);
					hours = (hours >= 10) ? hours : "0" + hours;
					minutes = minutes - (hours * 60);
					minutes = (minutes >= 10) ? minutes : "0" + minutes;
				}
				seconds = Math.floor(seconds % 60);
				seconds = (seconds >= 10) ? seconds : "0" + seconds;
				if (hours != "") {
					return hours + ":" + minutes + ":" + seconds;
				}
				return minutes + ":" + seconds;
			}
		  },
		  {
		  	"name": "energy",
			"title": "Energy",
			"parser": function (value) {
					if(value === 0){
						return "<span class=\"glyphicon glyphicon-repeat\"></span>";
					}
					return value + " kWh";
				}
		  },
		  {
		  	"name": "costs",
			"title": "Costs",
			"parser": function (value) {
					return value + " €";
				}
		  },
		  {
            "name": "uid",
            "title": "UID",
			"breakpoints": "xs",
            "type": "text",
          },
          {
            "name": "username",
            "title": "User",
			"breakpoints": "xs sm",
          }
        ],
        rows: logdata
      });
  });
}

function start() {
  var protocol = "ws://"; 
  if (window.location.protocol === "https:") {
    protocol = "wss://";
  }
  wsUri =protocol+ window.location.hostname + "/ws"; 
  websock = new WebSocket(wsUri);
  websock.addEventListener('message', socketMessageListener);
  websock.addEventListener('close', socketCloseListener);
  websock.addEventListener('error', socketErrorListener);
  websock.onopen = function(evt) {
    var commandtosend = {};
    commandtosend.command = "latestlog";
    websock.send(JSON.stringify(commandtosend));
    commandtosend = {};
    commandtosend.command = "gettime";
    websock.send(JSON.stringify(commandtosend));
  };
}

function socketMessageListener(evt) {
  var obj = JSON.parse(evt.data);
  if (obj.type === "latestlog") {
    logdata = obj.list;
	for(i in logdata){ 
		var costs = logdata[i].price / 100 * logdata[i].energy;
		logdata[i].costs = round(costs);
	}
    initTable();
    document.getElementById("loading-img").style.display = "none";
  }
}

function round(x) {
  var k = (Math.round(x * 100) / 100).toString();
  k += (k.indexOf('.') == -1)? '.00' : '00';
  return k.substring(0, k.indexOf('.') + 3);
}

function socketCloseListener(evt) {
    console.log('socket closed');
    websock = new WebSocket(wsUri);
    websock.addEventListener('message', socketMessageListener);
    websock.addEventListener('close', socketCloseListener);
    websock.addEventListener('error', socketErrorListener);
}

function socketErrorListener(evt) {
    console.log('socket error');
    console.log(evt);
}
function openNav() {
    document.getElementById("mySidenav").style.width = "250px";
}

function closeNav() {
    document.getElementById("mySidenav").style.width = "0";
}