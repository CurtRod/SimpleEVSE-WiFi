var websock = null;
var wsUri;
var firstEVSEData = true;

function start() {
  var protocol = "ws://"; 
  if (window.location.protocol === "https:") {
    protocol = "wss://";
  }
  wsUri =protocol+ window.location.hostname + "/ws"; 
  websock = new WebSocket(wsUri);
  websock.addEventListener('message', socketMessageListener);
  websock.addEventListener('error', socketErrorListener);
  websock.addEventListener('close', socketCloseListener);

  websock.onopen = function(evt) {
    websock.send("{\"command\":\"getevsedata\"}");
    document.getElementById("loading-img").style.display = "none";
  };
}

function socketMessageListener(evt) {
  var obj = JSON.parse(evt.data);
  if (obj.command === "getevsedata") {
    listEVSEData(obj);
  }
  if (obj.command === "sessiontimeout") {
    sessionTimeOut();
  }
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

function listEVSEData(obj) {
  document.getElementById("evse_current_limit").innerHTML = obj.evse_current_limit + " A";
  document.getElementById("evse_current").innerHTML = obj.evse_current + " kW";
  document.getElementById("evse_charging_time").innerHTML = obj.evse_charging_time;
  document.getElementById("evse_charged_kwh").innerHTML = obj.evse_charged_kwh + " kWh";
  
	if (obj.evse_active === false){		//EVSE not active
		$("#evseActive").addClass('hidden');
		$("#evseNotActive").removeClass('hidden');
	}
	else {	//EVSE active
		$("#evseNotActive").addClass('hidden');
		$("#evseActive").removeClass('hidden');
	}
	
	if (obj.evse_vehicle_state === 0){	//modbus error
		$("#carStatusDetected").addClass('hidden');
		$("#carStatusCharging").addClass('hidden');
		$("#carStatusReady").removeClass('hidden');
		document.getElementById("evse_vehicle_state").innerHTML = "Modbus Error";
	}
	if (obj.evse_vehicle_state === 1){	//Ready
		$("#carStatusDetected").addClass('hidden');
		$("#carStatusCharging").addClass('hidden');
		$("#carStatusReady").removeClass('hidden');
		document.getElementById("evse_vehicle_state").innerHTML = "Ready";
	}
	if (obj.evse_vehicle_state === 2){	//Vehicle Detected
		$("#carStatusReady").addClass('hidden');
		$("#carStatusCharging").addClass('hidden');
		$("#carStatusDetected").removeClass('hidden');
		document.getElementById("evse_vehicle_state").innerHTML = "Vehicle Detected";
	}
	if (obj.evse_vehicle_state === 3){	//Vehicle charging
		$("#carStatusReady").addClass('hidden');
		$("#carStatusDetected").addClass('hidden');
		$("#carStatusCharging").removeClass('hidden');
		document.getElementById("evse_vehicle_state").innerHTML = "Charging...";
	}
	document.getElementById("evse_charging_time").innerHTML = getTimeFormat(obj.evse_charging_time);
	if (firstEVSEData === true){
		document.getElementById("myRange").max = obj.evse_maximum_current;
		document.getElementById("myRange").value = obj.evse_current_limit;
		document.getElementById("slider_current").innerHTML = obj.evse_current_limit;
		firstEVSEData = false;
	}
	if (obj.ap_mode === true){
		syncBrowserTime();
	}
}

function handleSlider(value){
	document.getElementById("slider_current").innerHTML = value;
}

function setEVSECurrent() {
	var currentToSet = document.getElementById("myRange").value;
	websock.send("{\"command\":\"setcurrent\", \"current\":" + currentToSet + "}");
	$("#currentModal").modal("hide");
}
function activateEVSE(){
	websock.send("{\"command\":\"activateevse\"}");
	$("#buttonActivate").addClass('disabled');
	setTimeout(function(){$("#buttonActivate").removeClass('disabled');}, 4000);
}
function deactivateEVSE(){
	websock.send("{\"command\":\"deactivateevse\"}");
	$("#buttonDeactivate").addClass('disabled');
	setTimeout(function(){$("#buttonDeactivate").removeClass('disabled');}, 4000);
}

function getTimeFormat(millisec) {
    var seconds = (millisec / 1000).toFixed(0);
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

function syncBrowserTime() {
  var d = new Date();
  var timestamp = Math.floor((d.getTime() / 1000) + ((d.getTimezoneOffset() * 60) * -1));
  var datatosend = {};
  datatosend.command = "settime";
  datatosend.epoch = timestamp;
  websock.send(JSON.stringify(datatosend));
}

function sessionTimeOut() {
	websock.send("{\"command\":\"getevsedata\"}");
    return;
}

function openNav() {
    document.getElementById("mySidenav").style.width = "250px";
}

function closeNav() {
    document.getElementById("mySidenav").style.width = "0";
}

function refreshStats() {
  websock.send("{\"command\":\"status\"}");
}

$(document).ready(function(){
    $('[data-toggle="tooltip"]').tooltip();   
});