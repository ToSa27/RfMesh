var net = require('net');

function route(command) {
  console.log("About to route command " + command);
  if (command.substr(0, 5)  == "/cmd.") {
    console.log("About to submit command " + command.substr(5));
    var socket = new net.Socket();
    socket.connect(8081);
    socket.write(command.substr(5));
    socket.end();
  }
}

exports.route = route;
