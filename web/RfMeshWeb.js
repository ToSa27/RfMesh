var express = require('express'),
	app = express(),
	server = require('http').createServer(app),
	io = require('socket.io').listen(server),
	net = require('net');
	
server.listen(8000);

app.get('/', function(req, res){
	res.sendfile(__dirname + '/index.html');
});

function pushtoroot(form, data) {
	console.log('web socket rx: ' + data);
	var jdata = JSON.parse(data); 
	var json = JSON.stringify({type:form,data:jdata});
	console.log('submitting: ' + json);
	client.write(json);
}

io.sockets.on('connection', function(socket){
	console.log('web socket connected');
	socket.on('send message', function(data){
		console.log('web socket rx: ' + data);
		io.sockets.emit('new message', data);
	});
	socket.on('getConfig', function(data){
		pushtoroot('getConfig', data);
	});
	socket.on('pushConfig', function(data){
		pushtoroot('pushConfig', data);
	});
	socket.on('getMem', function(data){
		pushtoroot('getMem', data);
	});
	socket.on('setMem', function(data){
		pushtoroot('setMem', data);
	});
	socket.on('discover', function(data){
		pushtoroot('discover', data);
	});
	socket.on('reset', function(data){
		pushtoroot('reset', data);
	});
	socket.on('update', function(data){
		pushtoroot('update', data);
	});
	socket.on('dataLogging', function(data){
		pushtoroot('dataLogging', data);
	});
	socket.on('checkForNewFirmware', function(data){
		pushtoroot('checkForNewFirmware', data);
	});
});

var client = net.connect({path: '/tmp/RfMesh.sock'}, function() {
	console.log('unix socket connected');
});

client.on('data', function(data) {
	console.log('unix socket rx: ' + data.toString());
	var lines = data.toString().match(/^.*([\n\r]+|$)/gm);
	for (var i = 0; i < lines.length; i++) {
		var line = lines[i].trim();
		if (line) {
			var req = JSON.parse(line);
			if (req["type"].indexOf("Log:") == 0) {
				io.sockets.emit(req["type"], req["data"]);
			}
		}
	}
});

client.on('end', function() {
	console.log('unix socket disconnected');
	process.exit();
});
