// This file gets candles, runs my C++ program on them, and saves the signals it finds.
const fs = require("fs");
const { execFile } = require("child_process");
const initSqlJs = require("sql.js");

// The folder where my C++ project lives.
const folder = "C:/Users/Abubakar/Documents/Trading Signal Detector";

// Each line is a signal name and the sentence my C++ program prints when it finds it.
// The (\d+) part picks out the candle number from that sentence.
const rules = [
  ["Bullish Engulfing", /Bullish Engulfing signal found at \d+ and (\d+)/],
  ["Bearish Engulfing", /Bearish Engulfing signal found at \d+ and (\d+)/],
  ["Bullish Crossover", /Crossover bullish signal found at candle: (\d+)/],
  ["Bearish Crossover", /Crossover bearish signal found at candle: (\d+)/],
  ["Bullish Flip", /Bullish flip found at (\d+)/],
  ["Bearish Flip", /Bearish flip found at (\d+)/],
  ["Bullish Fibonacci", /Bounce found at (\d+)/],
  ["Bearish Fibonacci", /Bounce back found at (\d+)/],
  ["Bullish RSI Divergence", /Bullish RSI Divergence signal found at \d+ and the second swing low found at (\d+)/],
  ["Bearish RSI Divergence", /Bearish RSI Divergence signal found at \d+ and the second swing high found at (\d+)/],
];

async function main() {
  // Ask the website for the last 100 hourly EUR/USD candles.
  const url = "https://api.twelvedata.com/time_series?symbol=EUR/USD&interval=1h&outputsize=100&apikey=YOUR_KEY";
  const reply = await fetch(url);
  const data = await reply.json();

  // Write the candles into candles.txt, pair name first, then one candle per line.
  // reverse() puts the oldest candle first, because the website sends the newest first.
  const lines = ["EURUSD"];
  for (const c of data.values.reverse()) {
    lines.push(c.datetime + "," + c.open + "," + c.high + "," + c.low + "," + c.close);
  }
  fs.writeFileSync(folder + "/candles.txt", lines.join("\n"));
  console.log("Saved " + data.values.length + " candles");

  // Run my C++ program. "output" is everything it prints.
  execFile(folder + "/bin/Debug/Trading Signal Detector.exe", { cwd: folder, maxBuffer: 100 * 1024 * 1024 }, async (err, output) => {
    // If the program could not run, say so and stop.
    if (err) {
      console.log("Could not run the program:", err.message);
      return;
    }

    // Open the database file if it already exists, or make a new one.
    const SQL = await initSqlJs();
    const db = fs.existsSync("signals.db")
      ? new SQL.Database(fs.readFileSync("signals.db"))
      : new SQL.Database();
    // UNIQUE means the same signal at the same time can't be saved twice.
    db.run("CREATE TABLE IF NOT EXISTS signals (type TEXT, time TEXT, UNIQUE(type, time))");

    // Go through every line the C++ program printed and check it against each rule.
    for (const line of output.split("\n")) {
      for (const [type, re] of rules) {
        const m = line.match(re);
        if (m) {
          // m[1] is the candle number. Candle numbers start from 1, so take away 1 to find it in the list.
          const candle = data.values[m[1] - 1];
          // Save the signal name and the time of that candle. Skip it if it's already saved.
          if (candle) db.run("INSERT OR IGNORE INTO signals VALUES (?, ?)", [type, candle.datetime]);
        }
      }
    }

    // Save the database back into the file, then show how many signals are in it and how many of each type.
    fs.writeFileSync("signals.db", Buffer.from(db.export()));
    console.log("In database:", db.exec("SELECT COUNT(*) FROM signals")[0].values[0][0]);
    console.log(db.exec("SELECT type, COUNT(*) FROM signals GROUP BY type")[0].values);
  });
}
main();
