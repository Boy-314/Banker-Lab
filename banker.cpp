#include <iostream>
#include <cmath>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

struct task {
	string instruction;
	int delay;
	int resource;
	int claim;
};


void printOutput(int cycles[], int wait[], int length) {

	int x = 0;
	int waitTotal = 0;
	int runTotal = 0;

	for (int i = 0; i < length; i++) {

		printf("Task %i:\t\t", (x + 1));

		if (cycles[i] > -50) {

			printf("%i %i %i%%\n", cycles[i], wait[x], (int)round(wait[x] / (double)cycles[i] * 100));

			runTotal += cycles[i];
			waitTotal += wait[x];

		} else {

			cout << "aborted  " << endl;

		}
		x++;
	
	}

	printf("Total:\t\t%i %i %i%%\n\n", runTotal, waitTotal, (int)round(waitTotal / (double)runTotal * 100));

} // printOutput

void fifo(int t, int r, int available[], vector< vector<task> > process) {

	int remaining, current;
	int x, y;	//iterators
	int delay, resource, claim; // task vals

	string instruction;

	int deadlockCycle = 0;
	int terminated = 0;
	int deadlock = 0;
	int lines = 0;

	int cycles[t];
	int delayed[t];
	int wait[t];
	int released[r];
	int claims[t][r];

	vector<int> order;

	int **tasks;
	tasks = new int*[t];

	fill_n(cycles, t, 0);
	fill_n(delayed, t, 0);
	fill_n(wait, t, 0);
	fill_n(released, r, 0);

	for (x = 0; x < t; x++) {
		fill_n(claims[x], r, 0);
	}

	while (terminated != t) {
		deadlockCycle = 0;

		if (deadlock == (t - terminated)) {
			deadlockCycle = 1;

			for (x = 0; x < t; x++) {

				if ( !process[x].empty() ) {
					cycles[x] = -50;	// abort task

					// release all resources
					for (y = 0; y < r; y++) {
						available[y] += tasks[x][y];
						tasks[x][y] = 0;
					}

					process[x].clear();
					terminated++;
					break;
				}
			}
		}

		deadlock = 0;

		for (x = 0; x < t; x++) {
			if ( find(order.begin(), order.end(), x) == order.end() && !process[x].empty()) {
				order.push_back(x);
			}
		}

		remaining = t - terminated;

		for (x = 0; x < remaining; x++) {

			current = order[0];	// pick the first task waiting

			if ( !process[current].empty() ) {
				instruction = process[current][0].instruction;
				delay = process[current][0].delay;
				resource = process[current][0].resource;
				claim = process[current][0].claim;

				if (delay != delayed[current]) {
					delayed[current]++;
					cycles[current]++;
				} else {
					delayed[current] = 0;

					// initiate a new task
					if (instruction == "initiate") {
						
						tasks[current] = new int[r];

						tasks[current][resource - 1] = 0;
						cycles[current]++;

						process[current].erase(process[current].begin());
					} // initiate

					// request resources
					if (instruction == "request") {

						// if request not granted
						if (claim > available[resource - 1]) {

							if (deadlockCycle == 0) {
								cycles[current]++;
								wait[current]++; // wait...	
							}

							order.push_back(current);
							deadlock++;
						} else {
							tasks[current][resource - 1] += claim;
							available[resource - 1] -= claim;
							cycles[current]++;
							process[current].erase(process[current].begin());
						}
					} // request

					// release resources
					if (instruction == "release") {

						released[resource - 1] += claim;
						tasks[current][resource - 1] -= claim;
						cycles[current]++;
						process[current].erase(process[current].begin());

					} // release

					if (instruction == "terminate") {
						terminated++;
						process[current].erase(process[current].begin());
					} // terminate
				}
			} else {
				x--;
			}

			order.erase(order.begin());
		} // for remaining processes

		for (x = 0; x < r; x++) {
			available[x] += released[x];
		}

		fill_n(released, r, 0);
		lines++;

	} // loop

	cout << "FIFO: " << endl;

	printOutput(cycles, wait, t);

} // fifo

void banker(int t, int r, int available[], vector< vector<task> > process) {

	int current, remaining;

	string instruction;
	int delay, resource, claim;

	int x, y; // iterators

	int terminated = 0;
	int safe = 0;
	int lines = 0;

	int delayed[t];
	int cycles[t];
	int wait[t];
	int released[r];
	int original[t][r];
	int claims[t][r];

	vector<int> order;

	int **tasks;
	tasks = new int*[t];

	fill_n(delayed, t, 0);
	fill_n(cycles, t, 0);
	fill_n(wait, t, 0);
	fill_n(released, r, 0);

	for (x = 0; x < t; x++) {
		fill_n(claims[x], r, 0);
	}

	while (terminated != t) {
		for (x = 0; x < t; x++) {
			if ( find(order.begin(), order.end(), x) == order.end() && !process[x].empty()) {
				order.push_back(x);
			}
		}

		remaining = t - terminated;

		for (x = 0; x < remaining; x++) {

			current = order[0];

			if ( !process[current].empty()) {
				instruction = process[current][0].instruction;
				delay = process[current][0].delay;
				resource = process[current][0].resource;
				claim = process[current][0].claim;

				if (delay != delayed[current]) {
					delayed[current]++;
					cycles[current]++;
				} else {

					// initiate new task
					if (instruction == "initiate") {

						if (claim > available[resource - 1]) {

							cout << "Banker aborts: " 
							<< (current + 1) 
							<< "\n      Because claim " 
							<< claim << " exceeds resources available "
							<< available[resource - 1] << "\n";

							cycles[current] = -50; // abort task
							process[current].clear();
							terminated++;

						} else {

							delayed[current] = 0;

							tasks[current] = new int[r];
							
							tasks[current][resource - 1] = 0;

							cycles[current]++;

							process[current].erase(process[current].begin());

							claims[current][resource - 1] = -claim;
							
							original[current][resource - 1] = claim;
						}
					} // initiate

					// request resources
					if (instruction == "request") {

						if (claim > available[resource - 1] && claims[current][resource - 1] < 0) {

							cycles[current]++;
							wait[current]++;	// the current process waits
							order.push_back(current);

						} else {

							tasks[current][resource - 1] += claim;

							cycles[current]++;

							process[current].erase(process[current].begin());

							if (claims[current][resource - 1] < 0) {

								claims[current][resource - 1] += claim;
								available[resource - 1] -= claim;

							} else {

								claims[current][resource - 1] -= claim;

								if (claims[current][resource - 1] < 0) {

									cycles[current] = -50;	// abort task

									// release all resources
									for (y = 0; y < r; y++) {
										released[y] += tasks[current][y];
										available[y] += claims[current][resource - 1];
										tasks[x][y] = 0;
									}

									process[current].clear();
									terminated++;
								}
							}

							for (int i = 0; i < r; i++) {

								if (available[i] >= abs(claims[current][i])) {
									safe++;
								}
							}

							if (safe == r) {

								for (int i = 0; i < r; i++) {
									claims[current][i] = abs(claims[current][i]);
									available[i] -= claims[current][i];
								}

							}

							safe = 0;
							delayed[current] = 0;
						}
					} // request

					// release resources
					if (instruction == "release") {

						delayed[current] = 0;

						if (claims[current][resource - 1] >= 0) {
							claims[current][resource - 1] *= -1;

							for (int i = 0; i < r; i++) {
								if (claims[current][i] <= 0 || abs(claims[current][i]) == original[current][i]) {
									safe++;
								}
							}

							if (safe == r) {
								released[resource - 1] += claim;

								for (int i = 0; i < r; i++) {
									if (claims[current][i] > 0) {
										claims[current][i] *= -1;
									}
									available[i] -= claims[current][i];
								}
							}

							safe = 0;
							claims[current][resource - 1] -= claim;

						} else {
							claims[current][resource - 1] -= claim;
						}

						tasks[current][resource - 1] -= claim;
						cycles[current]++;
						process[current].erase(process[current].begin());
					} // release

					// terminate task
					if (instruction == "terminate") {

						delayed[current] = 0;
						terminated++;
						process[current].erase(process[current].begin());
					} // terminate
				}
			} else {
				x--;
			}

			order.erase(order.begin());
		} // process loop

		// mark all resources released in this cycle as available
		for (x = 0; x < r; x++) {
			available[x] += released[x];
		}

		fill_n(released, r, 0);
		lines++;
	} // full loop

	cout << "BANKER'S: " << endl;
	printOutput(cycles, wait, t);

} // banker

int main(int argc, char *argv[]) {

	string filepath = argv[1];
	ifstream infile( filepath );

	int t, r; // # of tasks, # of resources

	if( !( infile >> t >> r ) ) { // if we cannot find the first two input values, throw format error and terminate
		cout << "Input format error.";
		return 0; 
	} 

	int available[ r ];
	vector< vector<task> > process;

	int n;
	/* initialize array of available resources */
	for (int i = 0; i < r; i++) {
		if ( !( infile >> n ) ) {
			cout << "Input format error.";
			return 0;
		}
		available[i] =  n;
	}

	for (int i = 0; i < t; i++) {
		vector< task > temp;
		process.push_back(temp);
	}

	string inst;
	int current, x, y, z;

	while (infile >> inst >> current >> x >> y >> z) {

		task ttemp;

		ttemp.instruction = inst;
		ttemp.delay = x;
		ttemp.resource = y;
		ttemp.claim = z;

		process[current - 1].push_back(ttemp);
	}

	fifo(t, r, available, process);
	banker(t, r, available, process);

	return 0;
} // main