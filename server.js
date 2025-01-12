require('dotenv').config();
const express = require('express');
const { Pool } = require('pg');
const cors = require('cors');

const app = express();
const port = process.env.PORT || 4000;

// Middleware
app.use(express.json());
app.use(cors());

// PostgreSQL Pool
const pool = new Pool({
  connectionString: process.env.DATABASE_URL,
  ssl: {
    rejectUnauthorized: false, // Required for some hosted databases
  },
});

// API Endpoints

// Get the current light state
app.get('/command', async (req, res) => {
  try {
    const result = await pool.query('SELECT light_state FROM light_state WHERE id = 1');
    res.json({ lightState: result.rows[0].light_state });
  } catch (err) {
    console.error(err);
    res.status(500).send('Server error');
  }
});

// Update the light state
app.post('/command', async (req, res) => {
  const { lightState } = req.body;
  if (!lightState || (lightState !== 'on' && lightState !== 'off')) {
    return res.status(400).send('Invalid light state');
  }

  try {
    await pool.query('UPDATE light_state SET light_state = $1, updated_at = NOW() WHERE id = 1', [lightState]);
    res.send('Light state updated');
  } catch (err) {
    console.error(err);
    res.status(500).send('Server error');
  }
});

// Get ESP8266 status
app.get('/status', async (req, res) => {
  try {
    const result = await pool.query('SELECT esp8266_status FROM light_state WHERE id = 1');
    res.json({ online: result.rows[0].esp8266_status });
  } catch (err) {
    console.error(err);
    res.status(500).send('Server error');
  }
});

// Update ESP8266 status
app.post('/status', async (req, res) => {
  const { online } = req.body;
  if (typeof online !== 'boolean') {
    return res.status(400).send('Invalid ESP8266 status');
  }

  try {
    await pool.query('UPDATE light_state SET esp8266_status = $1, updated_at = NOW() WHERE id = 1', [online]);
    res.send('ESP8266 status updated');
  } catch (err) {
    console.error(err);
    res.status(500).send('Server error');
  }
});

// Start the server
app.listen(port, () => {
  console.log(`API listening on http://localhost:${port}`);
});
