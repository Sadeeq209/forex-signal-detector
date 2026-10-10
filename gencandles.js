const fs = require("fs");
const { execFile } = require("child_process");
const initSqlJs = require("sql.js");

const folder = "C:/Users/Abubakar/Documents/Trading Signal Detector";

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
  const url = "https://api.twelvedata.com/time_series?symbol=EUR/USD&interval=1h&outputsize=100&apikey=YOUR_KEY";
  const reply = await fetch(url);
  const data = await reply.json();

  const lines = ["EURUSD"];
  for (const c of data.values.reverse()) {
    lines.push(c.datetime + "," + c.open + "," + c.high + "," + c.low + "," + c.close);
  }
  fs.writeFileSync(folder + "/candles.txt", lines.join("\n"));
  console.log("Saved " + data.values.length + " candles");

  execFile(folder + "/bin/Debug/Trading Signal Detector.exe", { cwd: folder, maxBuffer: 100 * 1024 * 1024 }, async (err, output) => {
    if (err) {
      console.log("Could not run the program:", err.message);
      return;
    }

    const SQL = await initSqlJs();
    const db = fs.existsSync("signals.db")
      ? new SQL.Database(fs.readFileSync("signals.db"))
      : new SQL.Database();
    db.run("CREATE TABLE IF NOT EXISTS signals (type TEXT, time TEXT, UNIQUE(type, time))");

    for (const line of output.split("\n")) {
      for (const [type, re] of rules) {
        const m = line.match(re);
        if (m) {
          const candle = data.values[m[1] - 1];
          if (candle) db.run("INSERT OR IGNORE INTO signals VALUES (?, ?)", [type, candle.datetime]);
        }
      }
    }

    fs.writeFileSync("signals.db", Buffer.from(db.export()));
    console.log("In database:", db.exec("SELECT COUNT(*) FROM signals")[0].values[0][0]);
    console.log(db.exec("SELECT type, COUNT(*) FROM signals GROUP BY type")[0].values);
  });
}
main();