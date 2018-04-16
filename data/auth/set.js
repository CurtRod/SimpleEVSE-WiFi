var websock = null;
var utcSeconds;
var timeZone;
var recordstorestore = 0;
var slot = 0;
var page = 1;
var haspages;
var file = {};
var userdata = [];
var completed = false;
var wsUri;

function listCONF(obj) {
  document.getElementById("version").innerText = obj.version;
  document.getElementById("inputtohide").value = obj.ssid;
  document.getElementById("wifipass").value = obj.pswd;
  document.getElementById("wifibssid").value = obj.bssid;
  document.getElementById("gpioss").value = obj.sspin;
  document.getElementById("gain").value = obj.rfidgain;
  document.getElementById("gpioint").value = obj.intpin;
  document.getElementById("gpiobutton").value = obj.buttonpin;
  document.getElementById("impkwh").value = obj.kwhimp;
  document.getElementById("price").value = obj.price;
  document.getElementById("adminpwd").value = obj.adminpwd;
  document.getElementById("DropDownTimezone").value = obj.timezone;
  document.getElementById("hostname").value = obj.hostnm;
  if (obj.wmode === "1") {
	document.getElementById("wmodeap").checked = true;
	handleAP();
	syncBrowserTime(false);
  } else {
	document.getElementById("wmodesta").checked = true;
	handleSTA();
  }
  document.getElementById("checkboxRfid").checked = obj.rfid;
  handleRFID();
  document.getElementById("checkboxMeter").checked = obj.meter;
  handleMeter();
  var dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(obj, null, 2));
  var dlAnchorElem = document.getElementById("downloadSet");
  dlAnchorElem.setAttribute("href", dataStr);
  dlAnchorElem.setAttribute("download", "esp-rfid-settings.json");
  document.getElementById("maxinstall").value = obj.maxinstall;
  document.getElementById("checkboxButtonActive").checked = obj.buttonactive;
  handleButtonActive();
}

function browserTime() {
    var d = new Date(0);
    var c = new Date();
    var timestamp = Math.floor((c.getTime() / 1000) + ((c.getTimezoneOffset() * 60) * -1));
    d.setUTCSeconds(timestamp);
    document.getElementById("rtc").innerHTML = d.toUTCString().slice(0, -3);
}

function deviceTime() {
    var t = new Date(0); // The 0 there is the key, which sets the date to the epoch,
    t.setUTCSeconds(Math.floor(utcSeconds + (timeZone * 3600)));
    document.getElementById("utc").innerHTML = t.toUTCString().slice(0, -3);
	utcSeconds = utcSeconds + 1;
}

var t = setInterval(browserTime, 1000);
var tt = setInterval(deviceTime, 1000);

function syncBrowserTime(reload) {
  var d = new Date();
  var timestamp = Math.floor((d.getTime() / 1000) + ((d.getTimezoneOffset() * 60) * -1));
  var datatosend = {};
  datatosend.command = "settime";
  datatosend.epoch = timestamp;
  websock.send(JSON.stringify(datatosend));
  if(reload === true)
  {
	location.reload();
  }
}

function handleAP() {
  document.getElementById("hideBSSID").style.display = "none";
}

function handleSTA() {
  document.getElementById("hideBSSID").style.display = "block";
}

function handleRFID() {
	if (document.getElementById("checkboxRfid").checked === true){
		document.getElementById("gain").disabled = false;
		document.getElementById("gpioss").disabled = false;
	}
	else {
		document.getElementById("gain").disabled = true;
		document.getElementById("gpioss").disabled = true;	
	}
}

function handleMeter(){
	if (document.getElementById("checkboxMeter").checked === true){
		document.getElementById("gpioint").disabled = false;
		document.getElementById("impkwh").disabled = false;
		document.getElementById("price").disabled = false;
	}
	else {
		document.getElementById("gpioint").disabled = true;
		document.getElementById("impkwh").disabled = true;	
		document.getElementById("price").disabled = true;
	}
}
function handleButtonActive(){
	if(document.getElementById("checkboxButtonActive").checked === true){
		document.getElementById("gpiobutton").disabled = false;
	}
	else{
		document.getElementById("gpiobutton").disabled = true;
	}
}

function shWifi(){
	var x = document.getElementById("wifipass");
	if (x.type === "password"){
		x.type = "text";
		document.getElementById("shwifi").innerHTML = "hide";
	} else {
		x.type = "password";
		document.getElementById("shwifi").innerHTML = "show";
	}
}
function shAdmin(){
	var x = document.getElementById("adminpwd");
	if (x.type === "password"){
		x.type = "text";
		document.getElementById("shadmin").innerHTML = "hide";
	} else {
		x.type = "password";
		document.getElementById("shadmin").innerHTML = "show";
	}
}

function listSSID(obj) {
  obj.list.sort(function(a,b){return a.rssi <= b.rssi});
  var select = document.getElementById("ssid");
  for (var i = 0; i < obj.list.length; i++) {
    var opt = document.createElement("option");
    opt.value = obj.list[i].ssid;
    opt.bssidvalue = obj.list[i].bssid;
    opt.innerHTML = "BSSID: " + obj.list[i].bssid + ", Signal Strength: " + obj.list[i].rssi + ", Network: " + obj.list[i].ssid;
    select.appendChild(opt);
  }
  document.getElementById("scanb").innerHTML = "Re-Scan";
}

function listBSSID(obj) {
  var select = document.getElementById("ssid");
  document.getElementById("wifibssid").value = select.options[select.selectedIndex].bssidvalue;
}

function scanWifi() {
  websock.send("{\"command\":\"scan\"}");
  document.getElementById("scanb").innerHTML = "...";
  document.getElementById("inputtohide").style.display = "none";
  var node = document.getElementById("ssid");
  node.style.display = "inline";
  while (node.hasChildNodes()) {
    node.removeChild(node.lastChild);
  }
}

function saveConf() {
  var a = document.getElementById("adminpwd").value;
  if (a === null || a === "") {
    alert("Administrator Password cannot be empty");
    return;
  }
  var ssid;
  if (document.getElementById("inputtohide").style.display === "none") {
    var b = document.getElementById("ssid");
    ssid = b.options[b.selectedIndex].value;
  } else {
    ssid = document.getElementById("inputtohide").value;
  }
  var datatosend = {};
  datatosend.command = "configfile";
  var wmode = "0";
  if (document.getElementById("wmodeap").checked) {
    wmode = "1";
    datatosend.bssid = document.getElementById("wifibssid").value = 0;
  } else {
    datatosend.bssid = document.getElementById("wifibssid").value;
  }
  datatosend.ssid = ssid;
  datatosend.wmode = wmode;
  datatosend.pswd = document.getElementById("wifipass").value;
  datatosend.rfid = document.getElementById("checkboxRfid").checked;
  datatosend.sspin = document.getElementById("gpioss").value;
  datatosend.rfidgain = document.getElementById("gain").value;
  datatosend.meter = document.getElementById("checkboxMeter").checked;
  datatosend.intpin = document.getElementById("gpioint").value;
  datatosend.buttonpin = document.getElementById("gpiobutton").value;
  datatosend.kwhimp = document.getElementById("impkwh").value;
  datatosend.price = document.getElementById("price").value;
  datatosend.timezone = document.getElementById("DropDownTimezone").value;
  datatosend.hostnm = document.getElementById("hostname").value;
  datatosend.adminpwd = a;
  datatosend.maxinstall = document.getElementById("maxinstall").value;
  datatosend.buttonactive = document.getElementById("checkboxButtonActive").checked;

  websock.send(JSON.stringify(datatosend));
  alert("Device now should reboot with new settings");
  location.reload();
}

function backupuser() {
    var commandtosend = {};
    commandtosend.command = "userlist";
    commandtosend.page = page;
    websock.send(JSON.stringify(commandtosend));
}

function backupset() {
  var dlAnchorElem = document.getElementById("downloadSet");
  dlAnchorElem.click();
}

function piccBackup(obj) {
  var dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(obj, null, 2));
  var dlAnchorElem = document.getElementById("downloadUser");
  dlAnchorElem.setAttribute("href", dataStr);
  dlAnchorElem.setAttribute("download", "esp-rfid-users.json");
  dlAnchorElem.click();
}

function restoreSet() {
  var input = document.getElementById("restoreSet");
  var reader = new FileReader();
  if ("files" in input) {
    if (input.files.length === 0) {
      alert("You did not select file to restore");
    } else {
      reader.onload = function() {
        var json;
        try {
          json = JSON.parse(reader.result);
        } catch (e) {
          alert("Not a valid backup file");
          return;
        }
        if (json.command === "configfile") {
          var x = confirm("File seems to be valid, do you wish to continue?");
          if (x) {
            websock.send(JSON.stringify(json));
            alert("Device now should reboot with new settings");
            location.reload();
          }
        }
      };
      reader.readAsText(input.files[0]);
    }
  }
}

function restore1by1(i, len, data) {
  var part = 100 / len;
  var uid, user, acc, valid;
  document.getElementById("dynamic").style.width = part * (i + 1) + "%";
  var datatosend = {};
  uid = data[i].uid;
  user = data[i].username;
  acc = data[i].acctype;
  valid = data[i].validuntil;
  datatosend.command = "userfile";
  datatosend.uid = uid;
  datatosend.user = user;
  datatosend.acctype = acc;
  datatosend.validuntil = valid;
  websock.send(JSON.stringify(datatosend));
  slot++;
  if (slot === len) {
    document.getElementById("dynamic").className = "progress-bar progress-bar-success";
    document.getElementById("dynamic").innerHTML = "Completed";
    document.getElementById("dynamic").style.width = "100%";
    completed = true;
    document.getElementById("restoreclose").style.display = "block";
  }
}

function restoreUser() {
  var input = document.getElementById("restoreUser");
  var reader = new FileReader();
  if ("files" in input) {
    if (input.files.length === 0) {
      alert("You did not select any file to restore");
    } else {
      reader.onload = function() {
        var json;
        try {
          json = JSON.parse(reader.result);
        } catch (e) {
          alert("Not a valid backup file");
          return;
        }
        if (json.type === "esp-rfid-userbackup") {
          var x = confirm("File seems to be valid, do you wish to continue?");
          if (x) {
            recordstorestore = json.list.length;
            userdata = json.list;
            $("#restoremodal").modal();
            restore1by1(slot, recordstorestore, userdata);
          }
        }
      };
      reader.readAsText(input.files[0]);
    }
  }
}

function colorStatusbar(ref) {
  var percentage = ref.style.width.slice(0, -1);
  if (percentage > 50) ref.className = "progress-bar progress-bar-success";
  else if (percentage > 25) ref.className = "progress-bar progress-bar-warning";
  else ref.class = "progress-bar progress-bar-danger";
}

function refreshStats() {
  websock.send("{\"command\":\"status\"}");
}

function listStats(obj) {
  document.getElementById("chip").innerHTML = obj.chipid;
  document.getElementById("cpu").innerHTML = obj.cpu + " Mhz";
  document.getElementById("uptime").innerHTML = obj.uptime;
  document.getElementById("heap").innerHTML = obj.heap + " B";
  document.getElementById("heap").style.width = (obj.heap * 100) / 81920 + "%";
  colorStatusbar(document.getElementById("heap"));
  document.getElementById("flash").innerHTML = obj.availsize + " B";
  document.getElementById("flash").style.width = (obj.availsize * 100) / 1044464 + "%";
  colorStatusbar(document.getElementById("flash"));
  document.getElementById("spiffs").innerHTML = obj.availspiffs + " B";
  document.getElementById("spiffs").style.width = (obj.availspiffs * 100) / obj.spiffssize + "%";
  colorStatusbar(document.getElementById("spiffs"));
  document.getElementById("ssidstat").innerHTML = obj.ssid;
  document.getElementById("ip").innerHTML = obj.ip;
  document.getElementById("gate").innerHTML = obj.gateway;
  document.getElementById("mask").innerHTML = obj.netmask;
  document.getElementById("dns").innerHTML = obj.dns;
  document.getElementById("mac").innerHTML = obj.mac;
  document.getElementById("amps_conf").innerHTML = obj.evse_amps_conf;
  document.getElementById("amps_out").innerHTML = obj.evse_amps_out;
  document.getElementById("vehicle_status").innerHTML = obj.evse_vehicle_state;
  document.getElementById("amps_pp").innerHTML = obj.evse_pp_limit;
  document.getElementById("evse_version").innerHTML = obj.evse_firmware;
  document.getElementById("evse_status").innerHTML = obj.evse_state;
  document.getElementById("amps_boot").innerHTML = obj.evse_amps_afterboot;
  document.getElementById("evse_modbus").innerHTML = obj.evse_modbus_enabled;
  document.getElementById("amps_min").innerHTML = obj.evse_amps_min;
  document.getElementById("pp_detection").innerHTML = obj.evse_pp_detection;
}

function getnextpage() {
  if (page < haspages) {
  page = page + 1;
  var commandtosend = {};
  commandtosend.command = "userlist";
  commandtosend.page = page;
  websock.send(JSON.stringify(commandtosend));
  }
}

function builduserdata(obj) {
  userdata = userdata.concat(obj.list);
}

$(document).ready(function(){
    $('[data-toggle="tooltip"]').tooltip();   
});

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
    websock.send("{\"command\":\"getconf\"}");
    document.getElementById("loading-img").style.display = "none";
  };
  handleRFID();
  handleMeter();
}

function socketMessageListener(evt) {
  var obj = JSON.parse(evt.data);
  if (obj.command === "ssidlist") {
    listSSID(obj);
  } else if (obj.command === "configfile") {
    listCONF(obj);
    document.getElementById("loading-img").style.display = "none";
    websock.send("{\"command\":\"gettime\"}");
  } else if (obj.command === "gettime") {
    utcSeconds = obj.epoch;
	timeZone = obj.timezone;
  } else if (obj.command === "userlist") {
          haspages = obj.haspages;
    builduserdata(obj);
  } else if (obj.command === "status") {
    listStats(obj);
  } else if (obj.command === "result") {
    if (obj.resultof === "userfile") {
      if (!completed && obj.result === true) {
        restore1by1(slot, recordstorestore, userdata);
      }
    }
    else if (obj.resultof === "userlist") {
      if (page < haspages && obj.result === true) {
        getnextpage(page);
      }
      else if (page === haspages) {
        file.type = "esp-rfid-userbackup";
        file.version = "v0.4";
        file.list = userdata;
        piccBackup(file);
      }
    }
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

function openNav() {
    document.getElementById("mySidenav").style.width = "250px";
}

function closeNav() {
    document.getElementById("mySidenav").style.width = "0";
}
