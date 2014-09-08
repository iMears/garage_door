function fetch_status() {
	$.get("/status").done(fetch_success).fail(fetch_error).always(fetch_set_timeout);
}

function fetch_success(data) {
	if(data.closed){
		$(".status").text("Garage is closed.");
	}
	else {
		$(".status").text("Garage is open.");
	}
}

function fetch_error() {
	console.log("failed to fetch status...");
}

function fetch_set_timeout() {
	setTimeout(fetch_status, 1000);
}

$(fetch_status);
