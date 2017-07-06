/**
* Redox test
* ----------
* Increment a key on Redis using synchronous commands in a loop.
*/

#include <iostream>
#include "redox.hpp"

using namespace std;
using namespace redox;

double time_s() {
  unsigned long ms = chrono::system_clock::now().time_since_epoch() / chrono::microseconds(1);
  return (double)ms / 1e6;
}

int main() {

  Redox rdx;
  rdx.noWait(true);

  if(!rdx.connect("localhost", 6379)) return 1;

  if(rdx.commandSync({"SET", "simple_loop:count", "0"})) {
    cout << "Reset the counter to zero." << endl;
  } else {
    cerr << "Failed to reset counter." << endl;
    return 1;
  }

  double t = 5; // s
  
  cout << "Sending \"" << "SET simple_loop:count" << "\" synchronously for " << t << "s..." << endl;

  double t0 = time_s();
  double t_end = t0 + t;
  int count = 1;

  while(time_s() < t_end) {
    rdx.set("simple_loop:count", std::to_string(count));
    count++;
  }

  double t_elapsed = time_s() - t0;
  double actual_freq = (double)(count - 1) / t_elapsed;

  long final_count = stol(rdx.get("simple_loop:count"));

  cout << "Sent " << (count - 1) << " commands in " << t_elapsed << "s, "
       << "that's " << actual_freq << " commands/s." << endl;

  cout << "Final value of counter: " << final_count << endl;

  rdx.disconnect();
  return 0;
}
