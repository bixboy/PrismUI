const http = require('http');
const fs = require('fs');
const path = require('path');
const PrismParser = require('./prism-parser');

const PORT = 3000;
// Use provided path or default to PrismUI
const TARGET_PATH = process.argv[2] ? path.resolve(process.argv[2]) : path.resolve(__dirname, '../../Source/PrismUI');
const SOURCE_DIR = fs.lstatSync(TARGET_PATH).isDirectory() ? TARGET_PATH : path.dirname(TARGET_PATH);

const parser = new PrismParser();

let currentData = [];
let clients = [];

console.log(`[Simulator] Target Path: ${TARGET_PATH}`);
console.log(`[Simulator] Watching Directory: ${SOURCE_DIR}`);
fs.watch(SOURCE_DIR, { recursive: true }, (event, filename) => {
    if (filename && (filename.endsWith('.cpp') || filename.endsWith('.h'))) {
        console.log(`[Prism] Change detected in ${filename}. Re-parsing...`);
        try {
            const targetCpp = filename.endsWith('.cpp') ? path.join(SOURCE_DIR, filename) : path.join(SOURCE_DIR, filename.replace('.h', '.cpp'));
            if (fs.existsSync(targetCpp)) {
                currentData = parser.parseFile(targetCpp);
                console.log(`[Prism] Parsed ${currentData.length} root widgets.`);
                console.log(JSON.stringify(currentData, null, 2));
                notifyClients();
            }
        } catch (err) {
            console.error(`[Prism] Parser Error: ${err.message}`);
        }
    }
});

function notifyClients() {
    clients.forEach(res => {
        res.write(`data: ${JSON.stringify(currentData)}\n\n`);
    });
}

// HTTP Server
const server = http.createServer((req, res) => {
    if (req.url === '/') {
        res.writeHead(200, { 'Content-Type': 'text/html' });
        res.end(fs.readFileSync(path.join(__dirname, 'index.html')));
    } else if (req.url === '/events') {
        // Server-Sent Events for Live Reload
        res.writeHead(200, {
            'Content-Type': 'text/event-stream',
            'Cache-Control': 'no-cache',
            'Connection': 'keep-alive'
        });
        clients.push(res);
        req.on('close', () => {
            clients = clients.filter(c => c !== res);
        });
        // Initial data
        res.write(`data: ${JSON.stringify(currentData)}\n\n`);
    } else {
        res.writeHead(404);
        res.end();
    }
});

server.listen(PORT, () => {
    console.log(`[Prism] Simulator running at http://localhost:${PORT}`);
});
