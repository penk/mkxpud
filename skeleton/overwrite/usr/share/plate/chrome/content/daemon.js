var ws = require("ws"), 
sys = require('sys'), 
fs = require('fs'),
Inotify = require('inotify').Inotify;

var inotify = new Inotify();
var server = ws.createServer();

var callback = function(event) {
    var mask = event.mask;
	if(mask & Inotify.IN_CLOSE_WRITE) {
		// file was modified

		fs.createReadStream("/tmp/xpudctrl", {
		    'flags': 'r',
		    'encoding': 'UTF-8',
		    'mode': 0666,
		    'bufferSize': 4 * 1024
		}).addListener("data", function(data){
			data = data.toString();
			console.log(data);
			server.broadcast(data);
		});

	}
};

var fd = { path: '/tmp/xpudctrl',
	watch_for: Inotify.IN_CLOSE_WRITE,
	callback: callback
};

var watch = inotify.addWatch(fd);

server.listen(8080);
