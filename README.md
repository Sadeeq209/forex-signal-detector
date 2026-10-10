# Forex Signal Detector

A trading signal detector written in C++. It reads hourly candles for any forex pair from a text file and scans them for 5 signals, each in a bullish and a bearish version.

## Signals
- Engulfing candle
- Moving average crossover (5 and 10 candles)
- Support/resistance flip
- Fibonacci 61.8% retracement
- RSI divergence (14 periods)

## How it works
1. A Node.js script fetches live candles from the Twelve Data API (currently set to EUR/USD) and saves them to `candles.txt`.
2. The C++ program reads the file and runs every scanner.
3. Node picks the signals out of the output and stores them in a SQLite database, shown on a simple web page.

## Status
All 10 detectors work on real data. Backtesting (how often price moved the way each signal suggested) is the next step, and I'm not claiming these signals make money.

## Note
The price-range settings in the scanners are tuned for pairs quoted to 4 decimals, like EUR/USD. Other pairs need those numbers adjusted.

## Built with
C++, Node.js, SQLite
