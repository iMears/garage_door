var timer = null;

function setStatus(msg) {
	if(timer != null) {
		clearTimeout(timer);
		timer = null;
	}
	console.log("Setting status to '" + msg + "'");
	Pebble.sendAppMessage({0: msg});
}

function set_garage_status(status){
	console.log("Setting garage status " + status);
	Pebble.sendAppMessage({1: status});
}

function clearStatusWithTimeout() {
	timer = setTimeout(function() {
		setStatus("");
	}, 2000);
}

function sendRequest() {
	var response;
	var req = new XMLHttpRequest();
	req.open('POST', "https://__YOUR_USERNAME__:__YOUR_PASSWORD__@__YOUR_HOSTNAME__.ngrok.com/press?format=json", true);
	// req.setRequestHeader('Content-type', 'application/json');
	req.onload = function(e) {
		if (req.readyState == 4) {
		    if(req.status >= 200 && req.status < 400) {
		    	json = JSON.parse(req.responseText);
		    	if(json && json.status == 'success') {
			    	setStatus('Success!')
			    	clearStatusWithTimeout();
			    }
			    else {
			    	setStatus('Error!');
			    	clearStatusWithTimeout();
			    }
		   	} else {
		   		console.log("Failed with status=" + req.status);
		   		console.log("Response text is " + req.responseText);
		    	setStatus('Failed to send request');
		    }
		}
	}
	req.send(null);
}

function sendStatusRequest(){
	var req = new XMLHttpRequest();
	req.open('GET', "https://__YOUR_USERNAME__:__YOUR_PASSWORD__@__YOUR_HOSTNAME__.ngrok.com/status?format=json", true);
	req.onload = function(e) {
		if (req.readyState == 4) {
			setTimeout(sendStatusRequest, 1000);

		    if(req.status >= 200 && req.status < 400) {
		    	json = JSON.parse(req.responseText);
		    	if(json) {
			    	if (json.closed){
			    		set_garage_status("Closed");
			    	}
			    	else {
			    		set_garage_status("Open");
			    	}
			    }
			    else {
			    	set_garage_status("Failed");
			    }
		   	} else {
		   		console.log("Failed with status=" + req.status);
		   		console.log("Response text is " + req.responseText);
		    	set_garage_status('Failed');
		    }
		}
	}
	req.send(null);
}

Pebble.addEventListener("ready",
    function(e) {
        console.log("Hello world! - Sent from your javascript application.");
        sendStatusRequest();
    }
);

Pebble.addEventListener("appmessage",
	function(e) {
		console.log("Got request from watch");
		setStatus('Sending request...');
		sendRequest();
    }
);
