#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>

#include <sstream>

struct Candle {
	double open;
	double high;
	double low;
	double close;
	std::string time;
};

// checkBullish returns yes if a candle closes higher than it opens.
// Needs(one candle).
bool checkBullish(const Candle& c) {
		return c.close > c.open;
}

// checkBearish returns yes if a candle closes lower than it opens.
// Needs(one candle).
bool checkBearish(const Candle& c) {
		return c.close < c.open;
}

// checkBullishEngulfing returns yes if the first candle is bearish, the second is bullish, and the second candle's body fully covers the first candle's body.
// Needs(two candles, the earlier one first then the current).
bool checkBullishEngulfing(const Candle& c1, const Candle& c2) {
	return checkBearish(c1) && checkBullish(c2) && c2.close > c1.open && c2.open < c1.close;
}

// checkBearishEngulfing returns yes if the first candle is bullish, the second is bearish, and the second candle's body fully covers the first candle's body.
// Needs(two candles, the earlier one first then the current).
bool checkBearishEngulfing(const Candle& c1, const Candle& c2) {
	return checkBullish(c1) && checkBearish(c2) && c2.close < c1.open && c2.open > c1.close;
}

// scanForEngulfingSignal: Goes through the candles one by one.
// If the list is empty, it stops straight away.
// For each pair of candles, it checks for a bullish engulfing and a bearish engulfing.
// When it finds one, it prints the signal direction and the candle number.
void scanForEngulfingSignal(const std::vector<Candle>& candles) {
	if (candles.empty()) {
			return;
	}
	for (int i = 0; i < candles.size() - 1; i++) {
		if (checkBullishEngulfing(candles[i], candles[i + 1])) {
			std::cout << "Bullish Engulfing signal found at " << i + 1 << " and " << i + 2 << "\n";
		}
		else if (checkBearishEngulfing(candles[i], candles[i + 1])) {
			std::cout << "Bearish Engulfing signal found at " << i + 1 << " and " << i + 2 << "\n";
		}
	}
}

// calculateAverage returns the average close of 'N'' candles, counting back from the newest one, shifted back by 'S' candles.
// Needs(the candle list, 'N' how many candles to average and 'S' how many candles back to start from(0 means now)).
// Gives back 0 if there aren't enough candles or if N or S is a bad number.
double calculateAverage(const std::vector<Candle>& candles, int N, int S) {
	if (N <= 0 || S < 0 || N + S > candles.size()) {
		return 0;
	}
	double total = 0;
	for (int i = candles.size() - N - S; i < candles.size() - S; i++) {
		total += candles[i].close;
	}
	double average = total / N;
	return average;
}

// checkBullishCrossoverSignal Returns yes if the fast average was below or equal to the slow average one candle ago, and is above it now.
// Needs four averages in this order(fast now, slow now, fast one candle ago, slow one candle ago).
// Says no if any of the four is 0 or below.
bool checkBullishCrossoverSignal(double fastNow, double slowNow, double fastBefore, double slowBefore) {
	if (fastNow <= 0 || slowNow <= 0 || fastBefore <= 0 || slowBefore <= 0) {
		return false;
	}
	return fastBefore <= slowBefore && fastNow > slowNow;
}


// checkBearishCrossoverSignal Returns yes if the fast average was above or equal to the slow average one candle ago, and is below it now.
// Needs four averages in this order(fast now, slow now, fast one candle ago, slow one candle ago).
// Says no if any of the four is 0 or below.
bool checkBearishCrossoverSignal(double fastNow, double slowNow, double fastBefore, double slowBefore) {
	if (fastNow <= 0 || slowNow <= 0 || fastBefore <= 0 || slowBefore <= 0) {
		return false;
	}
	return fastBefore >= slowBefore && fastNow < slowNow;
}

// isSwingHigh Returns yes if the candle at this position has a higher high than the candle before and the candle after it.
// Needs(the candle list and a position)
// Returns no for the first and last positions, because they don't have a neighbour on both sides.
bool isSwingHigh(const std::vector<Candle>& candles, int i) {
		if (i <= 0 || i + 1 >= candles.size()) {
			return false;
		}
		return candles[i].high > candles[i - 1].high && candles[i].high > candles[i + 1].high;
}

// isSwingLow Returns yes if the candle at this position has a lower low than the candle before it and the candle after it.
// Needs(the candle list and a position)
// Returns no for the first and last positions, because they don't have a neighbour on both sides.
bool isSwingLow(const std::vector<Candle>& candles, int i) {
		if (i <= 0 || i + 1 >= candles.size()) {
			return false;
		}
		return candles[i].low < candles[i - 1].low && candles[i].low < candles[i + 1].low;
}

// isTheSamePriceRange Returns yes if two prices are within the given range of each other.
// Needs(two prices and a range).
bool isTheSamePriceRange(double p1, double p2, double r) {
			return std::abs(p1 - p2) <= r;
}

// checkResistanceLevel Returns yes if both candles at two given positions are swing highs and their highs are close enough to count as the same price.
// Needs(the candle list, the two positions and a range for how close counts as the same).
bool checkResistanceLevel(const std::vector<Candle>& candles, int f, int s, double r) {
	if (f == s) {
		return false;
	}
	return isSwingHigh(candles, f) && isSwingHigh(candles, s) && isTheSamePriceRange(candles[f].high, candles[s].high, r);
}

// checkSupportLevel Returns yes if both candles at two given positions are swing lows and their lows are close enough to count as the same price.
// Needs(the candle list, the two positions and a range for how close counts as the same).
bool checkSupportLevel(const std::vector<Candle>& candles, int f, int s, double r) {
	if (f == s) {
		return false;
	}
	return isSwingLow(candles, f) && isSwingLow(candles, s) && isTheSamePriceRange(candles[f].low, candles[s].low, r);
}

// checkBreak Returns yes if the candle at the given position closes above a given price.
// Needs(the candle list, a position and a price).
bool checkBreak(const std::vector<Candle>& candles, int i, double l) {
	if (i < 0 || i >= candles.size()) {
		return false;
	}
	return candles[i].close > l;
}

// checkBreakDown Returns yes if the candle at the given position closes below a given price.
// Needs(the candle list, a position and a price).
bool checkBreakDown(const std::vector<Candle>& candles, int i, double l) {
	if (i < 0 || i >= candles.size()) {
		return false;
	}
	return candles[i].close < l;
}

// checkBounce Returns yes if the candle at the given position touched the given price with its low and then closes above it.
// Needs(the candle list, a position, a price, and a range for how close counts as touching).
bool checkBounce(const std::vector<Candle>& candles, int i, double l, double r) {
	if (i < 0 || i >=candles.size()) {
		return false;
	}
	return isTheSamePriceRange(candles[i].low, l, r) && candles[i].close > l;
}

// checkBounceBack Returns yes if the candle at the given position touched the given price with its high and then closes below it.
// Needs(the candle list, a position, a price, and a range for how close counts as touching).
bool checkBounceBack(const std::vector<Candle>& candles, int i, double l, double r) {
	if (i < 0 || i >=candles.size()) {
		return false;
	}
	return isTheSamePriceRange(candles[i].high, l, r) && candles[i].close < l;
}

// checkBullishFlip returns yes if two candles form a ceiling, a later candle closes above it, and an even later candle touches it and closes back above it.
// Needs(the candle list, the two ceiling positions, the break position, the bounce position, and a range).
// Says no if the break isn't after the ceiling, or the bounce isn't after the break.
bool checkBullishFlip(const std::vector<Candle>& candles, int f, int s, int a, int b, double r) {
		if (a <= s || b <= a) {
			return false;
		}
		return checkResistanceLevel(candles, f, s, r) && checkBreak(candles, a, candles[f].high) && checkBounce(candles, b, candles[f].high, r);
}

// checkBearishFlip returns yes if two candles form a floor, a later candle closes below it, and an even later candle touches it and closes back below it.
// Needs(the candle list, the two floor positions, the break position, the bounce position, and a range).
// Says no if the break isn't after the floor, or the bounce isn't after the break.
bool checkBearishFlip(const std::vector<Candle>& candles, int f, int s, int a, int b, double r) {
		if (a <= s || b <= a) {
			return false;
		}
		return checkSupportLevel(candles, f, s, r) && checkBreakDown(candles, a, candles[f].low) && checkBounceBack(candles, b, candles[f].low, r);
}

// scanForCrossoverSignal: Goes through the candles from the oldest to the newest.
// For each one, it gets the fast and slow averages now, and the same two one candle earlier.
// If the fast one just crossed above the slow one, it prints a bullish signal with the candle number.
// If it just crossed below, it prints a bearish signal.
void scanForCrossoverSignal(const std::vector<Candle>& candles) {
	for (int i = candles.size(); i >= 0; i--) {
		double fastNow = calculateAverage(candles, 5, i);
		double slowNow = calculateAverage(candles, 10, i);
		double fastBefore = calculateAverage(candles, 5, i + 1);
		double slowBefore = calculateAverage(candles, 10, i + 1);
		if (checkBullishCrossoverSignal(fastNow, slowNow, fastBefore, slowBefore)) {
			std::cout << "Crossover bullish signal found at candle: " << candles.size() - i << std::endl;
		}
		else if (checkBearishCrossoverSignal(fastNow, slowNow, fastBefore, slowBefore)) {
			std::cout << "Crossover bearish signal found at candle: " << candles.size() - i << std::endl;
		}
	}
}

// noCloseAbove returns yes if no candle from the first position up to (not including) the last position closes above a given price.
bool noCloseAbove(const std::vector<Candle>& candles, int from, int to, double level) {
	for (int x = from; x < to; x++) {
		if (candles[x].close > level) {
			return false;
		}
	}
	return true;
}
// noCloseBelow returns yes if no candle from the first position up to (not including) the last position closes below a given price.
bool noCloseBelow(const std::vector<Candle>& candles, int from, int to, double level) {
	for (int x = from; x < to; x++) {
		if (candles[x].close < level) {
			return false;
		}
	}
	return true;
}
// scanForBullishFlip: Searches through the candle list for a bullish flip.
// It tries every group of four candles, in order, from oldest to newest.
// It checks that the first two make a ceiling, the third breaks above it, and the fourth bounces off it.
// When it finds one, it prints the ceiling candles, the break candle and the flip candle numbers.
void scanForBullishFlip(const std::vector<Candle>& candles) {
		for (int i = 0; i < candles.size(); i++) {
			for (int j = i + 1; j < candles.size(); j++) {
				for (int k = j + 1; k < candles.size(); k++) {
					for (int l = k + 1; l < candles.size(); l++) {
						if (checkResistanceLevel(candles, i, j, 0.0005) && checkBreak(candles, k, candles[i].high) && checkBounce(candles, l, candles[i].high, 0.0005) && noCloseAbove(candles, j + 1, k, candles[i].high) && noCloseBelow(candles, k, l, candles[i].high) && l - k <= 5 && k - j <= 10 && j - i <= 20) {
							std::cout << ":Resistance level found at " << i + 1 << " and " << j + 1 << "\n" << "A break found at " << k + 1 << "\n" << "Bullish flip found at " << l + 1 << "\n";
						}
					}
				}
			}
		}
}

// scanForBearishFlip: Searches through the candle list for a bearish flip.
// It tries every group of four candles, in order, from oldest to newest.
// It checks that the first two make a floor, the third breaks below it, and the fourth bounces up to it and closes back below.
// When it finds one, it prints the floor candles, the break down candle and the flip candle numbers.
void scanForBearishFlip(const std::vector<Candle>& candles) {
	for (int i = 0; i < candles.size(); i++) {
		for (int j = i + 1; j < candles.size(); j++) {
			for (int k = j + 1; k < candles.size(); k++) {
				for (int l = k + 1; l < candles.size(); l++) {
					if (checkSupportLevel(candles, i, j, 0.0005) && checkBreakDown(candles, k, candles[i].low) && checkBounceBack(candles, l, candles[i].low, 0.0005) && noCloseBelow(candles, j + 1, k, candles[i].low) && noCloseAbove(candles, k, l, candles[i].low) && l - k <= 5 && k - j <= 10 && j - i <= 20) {
						std::cout << "Support level found at " << i + 1 << " and " << j + 1 << "\n" << "A break down found at " << k + 1 << "\n" << "Bearish flip found at " << l + 1 << "\n";
					}
				}
			}
		}
	}
}

// calculateBullishFibonacciPercentage returns the 61.8% price after a rise, taken down from the rise high.
// Needs(the rise high and the rise low).
double calculateBullishFibonacciPercentage(double riseHigh, double riseLow) {
	double diff, p, percentage;
	diff = riseHigh - riseLow;
	p = diff / 100 * 61.8;
	percentage = riseHigh - p;
	return percentage;
}

// calculateBearishFibonacciPercentage returns the 61.8% price after a fall, taken up from the fall low.
// Needs(the fall high and the fall low).
double calculateBearishFibonacciPercentage(double fallHigh, double fallLow) {
	double diff, p, percentage;
	diff = fallHigh - fallLow;
	p = diff / 100 * 61.8;
	percentage = fallLow + p;
	return percentage;
}

// checkBullishFibonacciSignal returns yes if a candle bounces off the 61.8% level after a rise.
// Needs(the candle list, the rise low position, the rise high position, the bounce position, and a range).
// Says no if the positions are not in that order or are outside the list.
bool checkBullishFibonacciSignal(const std::vector<Candle>& candles, int i, int j, int k, double r) {
	if (i > j || j > k || i == j || j == k || i < 0 || k >= candles.size()) {
		return false;
	}
	double h;
	h = calculateBullishFibonacciPercentage(candles[j].high, candles[i].low);
	return checkBounce(candles, k, h, r);
}

// checkBearishFibonacciSignal returns yes if a candle bounces down off the 61.8% level after a fall.
// Needs(the candle list, the fall high position, the fall low position, the bounce position, and a range).
// Says no if the positions are not in that order or are outside the list.
bool checkBearishFibonacciSignal(const std::vector<Candle>& candles, int i, int j, int k, double r) {
	if (i > j || j > k || i == j || j == k || i < 0 || k >= candles.size()) {
		return false;
	}
	double h;
	h = calculateBearishFibonacciPercentage(candles[i].high, candles[j].low);
	return checkBounceBack(candles, k, h, r);
}

// scanForBullishFibonacciSignal: Searches through the candle list for a bullish Fibonacci signal.
// It tries every group of three candles, in order, as rise low, rise high and bounce.
// It only counts a group if the rise low is a swing low, the rise high is a swing high, and the bounce touches the 61.8% price.
// When it finds one, it prints the three candle numbers.
void scanForBullishFibonacciSignal(const std::vector<Candle>& candles) {
	for (int i = 0; i < candles.size(); i++) {
		for (int j = i + 1; j < candles.size(); j++) {
			for (int k = j + 1; k < candles.size(); k++) {
				if (checkBullishFibonacciSignal(candles, i, j, k, 0.001) && isSwingLow(candles, i) && isSwingHigh(candles, j) && candles[j].high - candles[i].low >= 0.004 && k - j <= 10 && j - i <= 10) {
					std::cout << "Rise low found at " << i + 1 << ", Rise high found at " << j + 1 << " and " << "Bounce found at " << k + 1 << "\n";
				}
			}
		}
	}
}

// scanForBearishFibonacciSignal: Searches through the candle list for a bearish Fibonacci signal.
// It tries every group of three candles, in order, as fall high, fall low and bounce back.
// It only counts a group if the fall high is a swing high, the fall low is a swing low, and the bounce back touches the 61.8% price.
// When it finds one, it prints the three candle numbers.
void scanForBearishFibonacciSignal(const std::vector<Candle>& candles) {
	for (int i = 0; i < candles.size(); i++) {
		for (int j = i + 1; j < candles.size(); j++) {
			for (int k = j + 1; k < candles.size(); k++) {
				if (checkBearishFibonacciSignal(candles, i, j, k, 0.001) && isSwingHigh(candles, i) && isSwingLow(candles, j) && candles[i].high - candles[j].low >= 0.004 && k - j <= 10 && j - i <= 10) {
					std::cout << "Fall high found at " << i + 1 << ", Fall low found at " << j + 1 << " and " << "Bounce back found at " << k + 1 << "\n";
				}
			}
		}
	}
}

// calculateRSI returns a number from 0 to 100 that shows how strong the recent rises were compared to the recent falls.
// Needs(the candle list, 'n' how many moves between closes to use, and 'f' how many candles back to start from, 0 means now).
// Gives back 100 if there were no falls, and 0 if n is a bad number or the list is too short.
double calculateRSI(const std::vector<Candle>& candles, int n, int f) {
	double totalGain = 0;
	double totalLost = 0;
	if (n <= 0 || f < 0 || n + f + 1 > candles.size()) {
		return 0;
	}
	for (int i = candles.size() - n - f; i < candles.size() - f; i++) {
		double move = candles[i].close - candles[i - 1].close;
		if (move > 0 ) {
			totalGain += move;
		} else {
			totalLost -= move;
		}
	}
	double averageGain = totalGain / n;
	double averageLost = totalLost / n;
	if (averageLost == 0) {
		return 100;
	}
	double rs = averageGain / averageLost;
	double rsi = 100 - (100 / (1 + rs));
	return rsi;
}

// checkBullishRSIDivergence returns yes if the price makes a lower low at the second swing low, but the RSI there is higher than at the first swing low.
// Needs(the candle list, the two swing low positions with the earlier one first, and 'n' how many moves to use for the RSI).
// Says no if the positions are in the wrong order, either one is not a swing low, or there aren't enough candles before the first one to work out its RSI.
bool checkBullishRSIDivergence(const std::vector<Candle>& candles, int f, int s, int n) {
	if (f < n) {
		return false;
	}
	if (f > s || f == s || !isSwingLow(candles, f) || !isSwingLow(candles, s)) {
		return false;
	}
	if (candles[s].low >= candles[f].low) {
		return false;
	}
	double a = calculateRSI(candles, n, candles.size() - 1 - f);
	double b = calculateRSI(candles, n, candles.size() - 1 - s);
	return b > a;
}

// checkBearishRSIDivergence returns yes if the price makes a higher high at the second swing high, but the RSI there is lower than at the first swing high.
// Needs(the candle list, the two swing high positions with the earlier one first, and 'n' how many moves to use for the RSI).
// Says no if the positions are in the wrong order, either one is not a swing high, or there aren't enough candles before the first one to work out its RSI.
bool checkBearishRSIDivergence(const std::vector<Candle>& candles, int f, int s, int n) {
	if (f < n) {
		return false;
	}
	if (f > s || f == s || !isSwingHigh(candles, f) || !isSwingHigh(candles, s)) {
		return false;
	}
	if (candles[s].high <= candles[f].high) {
		return false;
	}
	double a = calculateRSI(candles, n, candles.size() - 1 - f);
	double b = calculateRSI(candles, n, candles.size() - 1 - s);
	return b < a;
}

// scanForBullishRSIDivergence: Searches through the candle list for a bullish RSI divergence.
// It tries every pair of candles, in order, as the first and second swing low.
// It prints the candle numbers when the second low is lower in price but higher in RSI than the first.
void scanForBullishRSIDivergence(const std::vector<Candle>& candles) {
	for (int i = 0; i < candles.size(); i++) {
		for (int j = i + 1; j < candles.size(); j++) {
			if (checkBullishRSIDivergence(candles, i, j, 14)) {
				std::cout << "Bullish RSI Divergence signal found at " << i + 1 << " and the second swing low found at " << j + 1 << "\n";
			}
		}
	}
}

// scanForBearishRSIDivergence: Searches through the candle list for a bearish RSI divergence.
// It tries every pair of candles, in order, as the first and second swing high.
// It prints the candle numbers when the second high is higher in price but lower in RSI than the first.
void scanForBearishRSIDivergence(const std::vector<Candle>& candles) {
	for (int i = 0; i < candles.size(); i++) {
		for (int j = i + 1; j < candles.size(); j++) {
			if (checkBearishRSIDivergence(candles, i, j, 14)) {
				std::cout << "Bearish RSI Divergence signal found at " << i + 1 << " and the second swing high found at " << j + 1 << "\n";
			}
		}
	}
}


int main() {
	std::ifstream file("candles.txt");
	if (! file.is_open()) {
		std::cout << "File can't open\n";
		return 1;
	}

	std::string c;
	std::getline(file, c);

	std::vector<Candle> fileCandles;
	while (std::getline(file, c)) {
		std::istringstream line(c);
		std::string piece;
		Candle candle;
		int count = 0;
		while (std::getline(line, piece, ',')) {
			count++;
			if (count == 1) candle.time = piece;
			if (count == 2) candle.open = std::stod(piece);
			if (count == 3) candle.high = std::stod(piece);
			if (count == 4) candle.low = std::stod(piece);
			if (count == 5) candle.close = std::stod(piece);
		}
		fileCandles.push_back(candle);
	}

	scanForEngulfingSignal(fileCandles);
	scanForCrossoverSignal(fileCandles);
	scanForBullishFlip(fileCandles);
	scanForBearishFlip(fileCandles);
	scanForBullishFibonacciSignal(fileCandles);
	scanForBearishFibonacciSignal(fileCandles);
	scanForBullishRSIDivergence(fileCandles);
	scanForBearishRSIDivergence(fileCandles);

	return 0;
}
