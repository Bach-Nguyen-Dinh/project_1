var http = require('http');
var fs = require('fs');

http.createServer(function (req, res) {
    fs.readFile('web.html', function (err, data) {
        res.writeHead(200, {'Content-Type': 'text\http'});
        res.write(data);
        return res.end();
    });
}).listen(8080, '192.168.100.15');