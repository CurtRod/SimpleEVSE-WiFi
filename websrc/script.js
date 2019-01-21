//Globals
var websock = null;
var wsUri;

//EVSE Control
var firstEVSEData = true;
var chargingTime;
var evseActive;

//Users
var timezone;
var userdata = [];
var page = 1;
var haspages;
var usertable = false;

//Log
var logdata;
var logtable = false;

//Settings
var utcSeconds;
var recordstorestore = 0;
var slot = 0;
var page = 1;
var haspages;
var file = {};
var completed = false;
var wsUri;


//Script data for EVSE Control
function loadEVSEControl() {
    websock.send("{\"command\":\"getevsedata\"}");
	document.getElementById("evseContent").style.display = "block";
	document.getElementById("usersContent").style.display = "none";
	document.getElementById("settingsContent").style.display = "none";
	document.getElementById("logContent").style.display = "none";
	document.getElementById("loginContent").style.display = "none";
	closeNav();
}
function listEVSEData(obj) {
  chargingTime = obj.evse_charging_time;
  evseActive = obj.evse_active;
  document.getElementById("evse_charging_time").innerHTML = getTimeFormat(obj.evse_charging_time);
  document.getElementById("evse_current_limit").innerHTML = obj.evse_current_limit + " A";
  document.getElementById("evse_current").innerHTML = obj.evse_current + " kW";
  document.getElementById("evse_charged_kwh").innerHTML = obj.evse_charged_kwh + " kWh / " + obj.evse_charged_amount + " €";
  document.getElementById("evse_charged_mileage").innerHTML = obj.evse_charged_mileage + " km";
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
		syncBrowserTime(false);
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

function sessionTimeOut() {
	websock.send("{\"command\":\"getevsedata\"}");
    return;
}


//Script data for Users
function loadUsers(){
	document.getElementById("evseContent").style.display = "none";
	document.getElementById("usersContent").style.display = "block";
	document.getElementById("settingsContent").style.display = "none";
	document.getElementById("logContent").style.display = "none";
	closeNav();
	
	userdata = [];
    var commandtosend = {};
    websock.send("{\"command\":\"userlist\", \"page\":" + page + "}");
}
function listSCAN(obj) {
  if (obj.known === 1) {
    $(".fooicon-remove").click();
    document.querySelector('input.form-control[type=text]').value = obj.uid;
    $(".fooicon-search").click();
  } else {
    $(".footable-add").click();
    document.getElementById("uid").value = obj.uid;
    document.getElementById("picctype").value = obj.type;
    document.getElementById("username").value = obj.user;
    document.getElementById("acctype").value = obj.acctype;
  }
}

function getnextpage() {
  document.getElementById("loadpages").innerHTML = "Loading " + page + "/" + haspages;
  if (page < haspages) {
    page = page + 1;
    var commandtosend = {};
    commandtosend.command = "userlist";
    commandtosend.page = page;
    websock.send(JSON.stringify(commandtosend));
  }
}

function builduserdata(obj) {
  userdata = obj.list;
}

function initUserTable() {
  jQuery(function($) {
    var $modal = $('#editor-modal'),
      $editor = $('#editor'),
      $editorTitle = $('#editor-title'),
      ft = FooTable.init('#usertable', {
        columns: [{
            "name": "uid",
            "title": "UID",
            "type": "text",
          },
          {
            "name": "username",
            "title": "User Name or Label"
          },
          {
            "name": "acctype",
            "title": "Access Type",
            "breakpoints": "xs",
            "parser": function(value) {
              if (value === 1) {
                return "Active";
              } else if (value===99) {
                return "Admin";
              } else {
                return "Disabled";
              }
            },
          },
          {
            "name": "validuntil",
            "title": "Valid Until",
            "breakpoints": "xs sm",
              "parser": function(value) {
                  var vuepoch = new Date(value * 1000);
                  var formatted = vuepoch.getFullYear()
                        + '-' + twoDigits(vuepoch.getMonth() + 1) 
                        + '-' + twoDigits(vuepoch.getDate());
                  return formatted;
            },
          }
        ],
        rows: userdata,
        editing: {
          addRow: function() {
            $editor[0].reset();
            $editorTitle.text('Add a new User');
            $modal.modal('show');
          },
          editRow: function(row) {
            var acctypefinder;
            var values = row.val();
            if (values.acctype === "Active") {
    acctypefinder = 1;
  }
  else if (values.acctype === "Admin") {
    acctypefinder = 99;
  }
  else if (values.acctype === "Disabled"){
    acctypefinder = 0;
  }
            $editor.find('#uid').val(values.uid);
            $editor.find('#username').val(values.username);
            $editor.find('#acctype').val(acctypefinder);
            $editor.find('#validuntil').val(values.validuntil);
            $modal.data('row', row);
            $editorTitle.text('Edit User # ' + values.username);
            $modal.modal('show');
          },
          deleteRow: function(row) {
            var uid = row.value.uid;
            var username = row.value.username;
            if (confirm("This will remove " + uid + " : " + username + " from database. Are you sure?")) {
              var jsontosend = "{\"uid\":\"" + uid + "\",\"command\":\"remove\"}";
              websock.send(jsontosend);
              row.delete();
            }
          }
        },
        components: {
          filtering: FooTable.MyFiltering
        }
      }),
      uid = 10001;
    $editor.on('submit', function(e) {
            if (this.checkValidity && !this.checkValidity()) {
                return;
            }
            e.preventDefault();
			var row = null;
            row = $modal.data("row"),
                values = {
                    uid: $editor.find("#uid").val(),
                    username: $editor.find("#username").val(),
                    acctype: parseInt($editor.find("#acctype").val()),
                    validuntil: (new Date($editor.find("#validuntil").val()).getTime() / 1000)
                };
            if (row instanceof FooTable.Row) {
				row.val(values);
            } else {
				var datatosend = {};
				datatosend.command = "userfile";
				datatosend.uid = $editor.find("#uid").val();
				datatosend.user = $editor.find("#username").val();
				datatosend.acctype = parseInt($editor.find("#acctype").val());
				var validuntil = $editor.find("#validuntil").val();
				var vuepoch = (new Date(validuntil).getTime() / 1000);
				datatosend.validuntil = vuepoch;
				websock.send(JSON.stringify(datatosend));
				$modal.modal("hide");
                values.id = uid++;
                ft.rows.add(values);
				return;
            }

    });
  });
  usertable = true;
}

function acctypefinder() {
  if (values.acctype === "Active") {
    return 1;
  }
  else if (values.acctype === "Admin"){
    return 99;
  }
  else {
    return 0;
  }
}

function acctypeparser(){
  var $editor = $('#editor');
  if($editor.find('#acctype option:selected').val() == 1){
    return "Active";
  } else if ($editor.find('#acctype option:selected').val() == 99) {
    return "Admin";
  } else {
    return "Disabled";
  }
}

FooTable.MyFiltering = FooTable.Filtering.extend({
  construct: function(instance) {
    this._super(instance);
    this.acctypes = ['1', '99', '0'];
    this.acctypesstr = ['Active', 'Admin', 'Disabled'];
    this.def = 'Access Type';
    this.$acctype = null;
  },
  $create: function() {
    this._super();
    var self = this,
      $form_grp = $('<div/>', {
        'class': 'form-group'
      })
      .append($('<label/>', {
        'class': 'sr-only',
        text: 'Status'
      }))
      .prependTo(self.$form);

    self.$acctype = $('<select/>', {
        'class': 'form-control'
      })
      .on('change', {
        self: self
      }, self._onStatusDropdownChanged)
      .append($('<option/>', {
        text: self.def
      }))
      .appendTo($form_grp);

    $.each(self.acctypes, function(i, acctype) {
      self.$acctype.append($('<option/>').text(self.acctypesstr[i]).val(self.acctypes[i]));
    });
  },
  _onStatusDropdownChanged: function(e) {
    var self = e.data.self,
      selected = $(this).val();
    if (selected !== self.def) {
      self.addFilter('acctype', selected, ['acctype']);
    } else {
      self.removeFilter('acctype');
    }
    self.filter();
  },
  draw: function() {
    this._super();
    var acctype = this.find('acctype');
    if (acctype instanceof FooTable.Filter) {
      this.$acctype.val(acctype.query.val());
    } else {
      this.$acctype.val(this.def);
    }
  }
});

//Functions for Log
function loadLog() {
	document.getElementById("evseContent").style.display = "none";
	document.getElementById("usersContent").style.display = "none";
	document.getElementById("settingsContent").style.display = "none";
	document.getElementById("logContent").style.display = "block";
	closeNav();
	
    var commandtosend = {};
    commandtosend.command = "latestlog";
    websock.send(JSON.stringify(commandtosend));
    commandtosend = {};
    commandtosend.command = "gettime";
    websock.send(JSON.stringify(commandtosend));

}

function initLogTable() {
  jQuery(function($) {
    window.FooTable.init("#latestlogtable", {
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
					else if(value === "e"){
						return "<span class=\"glyphicon glyphicon-remove\"></span>";
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
					else if(value === "e"){
						return "<span class=\"glyphicon glyphicon-remove\"></span>";
					}
					return value + " kWh";
				}
		  },
		  {
		  	"name": "costs",
			"title": "Costs"
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
  logtable = true;
}


//Script for Settings
function loadSettings() {
	document.getElementById("evseContent").style.display = "none";
	document.getElementById("usersContent").style.display = "none";
	document.getElementById("settingsContent").style.display = "block";
	document.getElementById("logContent").style.display = "none";
	closeNav();
	
	websock.send("{\"command\":\"getconf\"}");
	handleRFID();
	handleMeter();
	handleMeterType();
	handleButtonActive();
	handleStaticIP();
}
function listCONF(obj) {
  document.getElementById("inputtohide").value = obj.ssid;
  document.getElementById("wifipass").value = obj.pswd;
  document.getElementById("wifibssid").value = obj.bssid;
  if (typeof obj.staticip !== "undefined"){
	document.getElementById("checkboxStaticIP").checked = obj.staticip;
	document.getElementById("ipch").value = obj.ip;
	document.getElementById("subnetch").value = obj.subnet;
	document.getElementById("gatewaych").value = obj.gateway;
	document.getElementById("dnsch").value = obj.dns;
  }
  handleStaticIP();
  
  document.getElementById("gpioss").value = obj.sspin;
  document.getElementById("gain").value = obj.rfidgain;
  document.getElementById("gpiobutton").value = obj.buttonpin;
  
  if (typeof obj.wsauth !== "undefined"){
	document.getElementById("checkboxSafari").checked = obj.wsauth;
  }
  if (typeof obj.debug !== "undefined"){
	document.getElementById("checkboxDebug").checked = obj.debug;
  }
  if (typeof obj.intpin !== "undefined"){
	document.getElementById("gpioint").value = obj.intpin;
  }
  if (typeof obj.kwhimp !== "undefined"){
	document.getElementById("impkwh").value = obj.kwhimp;
  }
  if (typeof obj.implen !== "undefined"){
	document.getElementById("implen").value = obj.implen;
  }
  if (typeof obj.meterphase !== "undefined"){
	document.getElementById("meterphase").value = obj.meterphase;
  }
  document.getElementById("smetertype").value = obj.metertype;
  handleMeterType();
  if (typeof obj.price !== "undefined"){
	document.getElementById("price").value = obj.price;
  }
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
  
  if (typeof obj.avgconsumption !== "undefined"){
	  document.getElementById("avgconsumption").value = obj.avgconsumption;
  }
  else{
	  document.getElementById("avgconsumption").value = "15.5";
  }
  
  
  if (typeof obj.factor !== "undefined"){
	  document.getElementById("factor").value = obj.factor;
  }
  else {
	  document.getElementById("factor").value = "1";
  }
  if (typeof obj.ntpIP !== "undefined"){
	  document.getElementById("ntpIP").value = obj.ntpIP;
  }
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
    t.setUTCSeconds(Math.floor(utcSeconds + (timezone * 3600)));
    document.getElementById("utc").innerHTML = t.toUTCString().slice(0, -3);
	utcSeconds = utcSeconds + 1;
}

function chargingTime(){
	if (evseActive === true){
		chargingTime = chargingTime + 1000;
		document.getElementById("evse_charging_time").innerHTML = getTimeFormat(chargingTime);
	}
}

var t = setInterval(browserTime, 1000);
var tt = setInterval(deviceTime, 1000);
var ttt = setInterval(chargingTime, 1000);

function setEVSERegister() {
	var datatosend = {};
	datatosend.command = "setevsereg";
	datatosend.register = document.getElementById("evseRegToSet").value;
	datatosend.value = document.getElementById("evseRegValue").value;
	websock.send(JSON.stringify(datatosend));
	$("#evseRegModal").modal("hide");
	refreshStats();
}

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

function handleStaticIP(){
	if (document.getElementById("checkboxStaticIP").checked === true){
		document.getElementById("divIP").style.display = "block";
		document.getElementById("divSubnet").style.display = "block";
		document.getElementById("divGateway").style.display = "block";
		document.getElementById("divDNS").style.display = "block";
	}
	else{
		document.getElementById("divIP").style.display = "none";
		document.getElementById("divSubnet").style.display = "none";
		document.getElementById("divGateway").style.display = "none";
		document.getElementById("divDNS").style.display = "none";
	}
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
		document.getElementById("implen").disabled = false;
		document.getElementById("price").disabled = false;
		document.getElementById("smetertype").disabled = false;
	}
	else {
		document.getElementById("gpioint").disabled = true;
		document.getElementById("impkwh").disabled = true;	
		document.getElementById("implen").disabled = true;
		document.getElementById("price").disabled = true;
		document.getElementById("smetertype").disabled = true;
	}
}

function handleMeterType(){
	if (document.getElementById("smetertype").value !== "S0"){
		document.getElementById("divImpKwh").style.display = "none";
		document.getElementById("divImpLen").style.display = "none";
		document.getElementById("divMeterPin").style.display = "none";
		document.getElementById("divMeterPhase").style.display = "none";
		document.getElementById("meterRegisters").style.display = "block";
	}
	else{
		document.getElementById("divImpKwh").style.display = "block";
		document.getElementById("divMeterPin").style.display = "block";
		document.getElementById("divMeterPhase").style.display = "block";
		document.getElementById("divImpLen").style.display = "block";
		document.getElementById("meterRegisters").style.display = "none";
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
  listBSSID();
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
//Validate input
  var a = document.getElementById("adminpwd").value;
  if (a === null || a === "") {
    alert("Administrator Password cannot be empty");
    return;
  }
  else if (a.length < 8 ){
	alert("Administrator Password must be at least 8 characters");
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
	if (document.getElementById("wifipass").value.length < 8 &&
		document.getElementById("wifipass").value.length !== 0){
		alert("WiFi Password in AP mode must be at least 8 characters or empty (for wifi without protection)");
		return;
		}
  }
  else {
    datatosend.bssid = document.getElementById("wifibssid").value;
  }
 
  datatosend.ssid = ssid;
  datatosend.wmode = wmode;
  datatosend.pswd = document.getElementById("wifipass").value;
  
  if (document.getElementById("checkboxStaticIP").checked === true){
	datatosend.staticip = document.getElementById("checkboxStaticIP").checked;
	datatosend.ip = document.getElementById("ipch").value;
	datatosend.subnet = document.getElementById("subnetch").value;
	datatosend.gateway = document.getElementById("gatewaych").value;
	datatosend.dns = document.getElementById("dnsch").value;
  }
  
  datatosend.wsauth = document.getElementById("checkboxSafari").checked;
  datatosend.debug = document.getElementById("checkboxDebug").checked;

  datatosend.rfid = document.getElementById("checkboxRfid").checked;
  datatosend.sspin = document.getElementById("gpioss").value;
  datatosend.rfidgain = document.getElementById("gain").value;
  
  
  if (document.getElementById("checkboxMeter").checked === true){
	  datatosend.meter = document.getElementById("checkboxMeter").checked;
	  datatosend.metertype = document.getElementById("smetertype").value;
	  datatosend.price = document.getElementById("price").value;
	  if (document.getElementById("smetertype").value === "S0"){
		datatosend.intpin = document.getElementById("gpioint").value;
		datatosend.kwhimp = document.getElementById("impkwh").value;
		datatosend.implen = document.getElementById("implen").value;
		datatosend.meterphase = document.getElementById("meterphase").value;
	  }
  }
  datatosend.buttonpin = document.getElementById("gpiobutton").value;
  datatosend.factor = document.getElementById("factor").value;
  datatosend.timezone = document.getElementById("DropDownTimezone").value;
  datatosend.hostnm = document.getElementById("hostname").value;
  datatosend.adminpwd = a;
  datatosend.maxinstall = document.getElementById("maxinstall").value;
  datatosend.buttonactive = document.getElementById("checkboxButtonActive").checked;
  datatosend.avgconsumption = document.getElementById("avgconsumption").value;

  datatosend.ntpIP = document.getElementById("ntpIP").value;
  
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
        if (json.type === "evse-wifi-userbackup") {
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

function resetLogFile() {
	if (confirm("Are you sure to reset all your charging logs?")){
		websock.send("{\"command\":\"initlog\"}");
		alert("Log File Resetted!");
		location.reload();
	}
	else {
		alert("Aborted!");
	}
	
}

var FWUpdatePerc = 0;

function firwareUpdate(){
	//var tFirmware = setInterval(progressFWUpdate, 1200);
	$("#updatemodal").modal();
}

function progressFWUpdate(){
	if (FWUpdatePerc === 100){
		setTimeout(function(){location.reload()}, 1000);
	}
	else {
		FWUpdatePerc = FWUpdatePerc + 4;
		document.getElementById("dynamicFWUpdate").style.width = FQUpdatePerc + "%";
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
  document.getElementById("flash").style.width = (obj.availsize * 100) / (4194304 - obj.spiffssize) + "%";
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
  document.getElementById("amps_conf").innerHTML = obj.evse_amps_conf;			//1000
  document.getElementById("amps_out").innerHTML = obj.evse_amps_out;			//1001
  document.getElementById("vehicle_status").innerHTML = obj.evse_vehicle_state;	//1002
  document.getElementById("amps_pp").innerHTML = obj.evse_pp_limit;				//1003
  document.getElementById("amps_turn_off").innerHTML = obj.evse_turn_off;		//1004
  document.getElementById("evse_version").innerHTML = obj.evse_firmware;		//1005
  document.getElementById("evse_status").innerHTML = obj.evse_state;			//1006
  document.getElementById("amps_boot").innerHTML = obj.evse_amps_afterboot;		//2000
  document.getElementById("evse_modbus").innerHTML = obj.evse_modbus_enabled;	//2001
  document.getElementById("amps_min").innerHTML = obj.evse_amps_min;			//2002
  document.getElementById("evse_analog_in").innerHTML = obj.evse_analog_input;	//2003
  document.getElementById("amps_power_on").innerHTML = obj.evse_amps_poweron;	//2004
  document.getElementById("evse_2005").innerHTML = obj.evse_2005;				//2005
  document.getElementById("evse_sharing_mode").innerHTML = obj.evse_sharing_mode;//2006
  document.getElementById("pp_detection").innerHTML = obj.evse_pp_detection;	//2007
  
  document.getElementById("meter_total").innerHTML = obj.meter_total;
}

//General functions
function round(x) {
  var k = (Math.round(x * 100) / 100).toString();
  k += (k.indexOf('.') == -1)? '.00' : '00';
  return k.substring(0, k.indexOf('.') + 3);
}

function twoDigits(value) {
   if(value < 10) {
    return "0" + value;
   }
   return value;
}

function openNav() {
    document.getElementById("mySidenav").style.width = "250px";
}

function closeNav() {
    document.getElementById("mySidenav").style.width = "0";
}

function showCurrentModal() {
	$("#currentModal").modal();
}

function showEvseRegModal() {
	$("#evseRegModal").modal();
}

$(document).ready(function(){
    $('[data-toggle="tooltip"]').tooltip();
});

function socketMessageListener(evt) {
  var obj = JSON.parse(evt.data);
  if (obj.command === "getevsedata") {
    listEVSEData(obj);
  }
  else if (obj.command === "sessiontimeout") {
	  if (document.getElementById("evseContent").style.display !== "none"){
		sessionTimeOut();
	  }
  }
  else if (obj.command === "piccscan") {
	  if(document.getElementById("usersContent").style.display === "block"){
		listSCAN(obj);
	  }
  }
  else if (obj.command === "gettime") {
    timezone = obj.timezone;
	utcSeconds = obj.epoch;
  }
  else if (obj.command === "userlist") {
    haspages = obj.haspages;
    if (haspages === 0) {
		document.getElementById("usertable").innerHTML = null;
		initUserTable();
		$(".footable-show").click();
		$(".fooicon-remove").click();
	}
    builduserdata(obj);
  }
  else if (obj.command === "result") {
	if (obj.resultof === "userfile" && document.getElementById("settingsContent").style.display !== "none") {
      if (!completed && obj.result === true) {
        restore1by1(slot, recordstorestore, userdata);
      }
    }
    else if (obj.resultof === "userlist") {
      if (page < haspages && obj.result === true) {
        getnextpage(page);
      }
      else if (page === haspages) {
		if (document.getElementById("settingsContent").style.display !== "none"){
			file.type = "evse-wifi-userbackup";
			file.version = "v0.1";
			file.list = userdata;
			piccBackup(file);
		}
		else if (document.getElementById("usersContent").style.display !== "none"){
			document.getElementById("usertable").innerHTML = null;
			initUserTable();
			$(".footable-show").click();
			$(".fooicon-remove").click();
			websock.send("{\"command\":\"gettime\"}");
		}
      }
    }
  }
  else if (obj.command === "ssidlist") {
    listSSID(obj);
  } 
  else if (obj.command === "configfile") {
    listCONF(obj);
    websock.send("{\"command\":\"gettime\"}");
  } 
  else if (obj.command === "userlist") {
	haspages = obj.haspages;
	builduserdata(obj);
  } 
  else if (obj.command === "status") {
    listStats(obj);
  }
  if (obj.type === "latestlog") {
	logdata = obj.list;
	for(i in logdata){ 
	if(logdata[i].price === "e"){
		logdata[i].costs = "<span class=\"glyphicon glyphicon-remove\"></span>";
		}
		else {
			logdata[i].costs = String(round(logdata[i].price / 100 * logdata[i].energy)) + " €";
		}
	}
	if (logdata.length > 0 || logtable === false){
		document.getElementById("latestlogtable").innerHTML = null;
		initLogTable();
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
function login() {
    var username = "admin"
    var passwd = document.getElementById("password").value;
    var url = "/login";
    var xhr = new XMLHttpRequest();
    xhr.open("get", url, true, username, passwd);
    xhr.onload = function(e) {
      if (xhr.readyState === 4) {
        if (xhr.status === 200) {
		  wsConnect();
        } else {
          alert("Incorrect password!");
        }
      }
    };
    xhr.send(null);
}

function wsConnect(){
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
		loadEVSEControl();
	};
}

function start() {
	document.getElementById("evseContent").style.display = "none";
	document.getElementById("usersContent").style.display = "none";
	document.getElementById("settingsContent").style.display = "none";
	document.getElementById("logContent").style.display = "none";
}