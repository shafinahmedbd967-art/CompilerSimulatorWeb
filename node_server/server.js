// final node_server/server.js
const express = require('express');
const bodyParser = require('body-parser');
const path = require('path');
const { spawn } = require('child_process');
const app = express();

// Body parser middleware
app.use(bodyParser.json());

// Serve frontend folder
app.use(express.static(path.join(__dirname, '../frontend')));

// Serve index.html on root
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, '../frontend/index.html'));
});

// -------------------- Compile endpoint --------------------
app.post('/compile', (req, res) => {
  const expr = req.body.expression;

  // Path to compiled C binary (Windows)
  const compilerPath = path.join(__dirname, '../backend/compiler.exe');

  // Spawn child process
  const child = spawn(compilerPath);

  let output = '';
  let error = '';

  // Write input expression to stdin
  child.stdin.write(expr + '\n');
  child.stdin.end();

  child.stdout.on('data', data => {
    output += data.toString();
  });

  child.stderr.on('data', data => {
    error += data.toString();
  });

  child.on('close', code => {
    if (code !== 0) {
      console.error('Compiler error:', error);
      return res.status(500).json({ error: 'Compilation failed' });
    }

    // Clean raw output for JSON.parse
    try {
      const cleanOutput = output
        .replace(/\r/g, '')          // remove CR
        .replace(/\t/g, '    ')      // replace tabs with spaces
        .replace(/[\x00-\x1F\x7F]/g, ''); // remove remaining control chars

      const json = JSON.parse(cleanOutput);
      res.json(json);
    } catch (e) {
      console.error('JSON parse error:', e, 'Raw:', output);
      res.status(500).json({ error: 'Invalid JSON from compiler' });
    }
  });
});

// -------------------- Start server --------------------
const PORT = 3000;
app.listen(PORT, () => console.log(`Server running at http://localhost:${PORT}`));
