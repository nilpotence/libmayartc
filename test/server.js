var net = require('net');
var WebSocketServer = require('ws').Server;

process.on( 'SIGINT', function() {
	console.log( "\nExiting..." );
	server.close();
	wsServer.close();
	
	process.exit();
});


SIGNALING_BUFFER_SIZE = 8192;
var signalingBuffer = new Buffer(SIGNALING_BUFFER_SIZE);
var tmpbuff = new Buffer(SIGNALING_BUFFER_SIZE);
var buffer_pos = 0;
var len = 0;
function handleHostData(data, onmessage){
	data.copy(signalingBuffer, buffer_pos, 0, data.length);
	
	len = data.length;
	
	var msgComplete = false;
	buffer_pos += len;

	do{
		msgComplete = false;

		var msg_length = signalingBuffer.readInt32LE(0);

		if(buffer_pos > msg_length){
			var newMessage = signalingBuffer.toString('utf8',4,4+msg_length);
			
			onmessage(newMessage);
			
			signalingBuffer.copy(tmpbuff, 0, 4+msg_length, SIGNALING_BUFFER_SIZE-4-msg_length);
			tmpbuff.copy(signalingBuffer);
			
			buffer_pos -= (4 + msg_length);

			msgComplete = true;
		}
	}while(msgComplete && buffer_pos > 0);	
}

function sendToClient(msg){
	var msg_obj = JSON.parse(msg);
	var peerid = msg_obj.peerid;
	if(peerid != undefined && wsClients[peerid] != undefined){
		msg_obj.peerid = undefined;
		console.log("TO_CLIENT = " + JSON.stringify(msg_obj));
		wsClients[peerid].send(JSON.stringify(msg_obj));
	}
}
var hostSocket = null;
//Connect to UNIX socket
var server = net.createServer(function(socket) {
	
	hostSocket = socket;
	
	socket.on('data', function(data){
		handleHostData(data, sendToClient);
	});
	
	socket.on('close', function(){
		hostSocket = null;
	});
});
server.listen('/tmp/maya.sock');


/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////


var peer_base = 0;
function generatePeerID(){
	var ret = peer_base;
	peer_base++;
	return ret;
}

function handlePeerMessage(peerid, msg){
	
	var msg_obj = JSON.parse(msg);
	
	if(msg_obj != null){
		msg_obj["peerid"] = peerid;
		msg = JSON.stringify(msg_obj);
		
		sendToHost(msg);
	}
}

var length_buff = new Buffer(4);
function sendToHost(msg){
	if(hostSocket) {
		length_buff.writeUInt32LE(msg.length, 0);
		hostSocket.write(length_buff);
		hostSocket.write(msg);
	}
}


var wsServer = new WebSocketServer({port:8080});
var wsClients = [];


wsServer.on('connection', function(socket){
	
	var peerid = generatePeerID();
	
	wsClients[peerid] = socket;
	
	socket.on('message', function(msg){
		handlePeerMessage(peerid, msg);
	});
	
	socket.on('close', function(){
		wsClients[peerid] = null;
	});
	
});
