#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

using namespace std;

bool areSameFileCheck(FILE* f1, FILE* f2) {

  return false;
}


int main(int argc, char* argv[]) {
  if (argc >= 4) {
    cout << "usage: reverse <input> <output>" << endl;
    return 1;
  }

  char* inputFile;
  char* outputFile;
  string contents;
  bool print = true;

  FILE* in;
  FILE* out;

  // If user provides no arguments, we accept input text from stdin and we print
  // the output to stdout
  if (argc == 1) {
    cin >> contents;
  } else {
    inputFile = argv[1];

    if (argc == 3) {
      outputFile = argv[2];
      print = false;

      if (!(out = fopen(outputFile, "w"))) {
        cout << "error: cannot open file '" << outputFile << "'" << endl;
        return 1;
      }
    }

    // Attempt to open both files and report errors if applicable
    if (!(in = fopen(inputFile, "r"))) {
      cout << "error: cannot open file '" << inputFile << "'" << endl;
      return 1;
    }

    if (in == out) {
      cout << "Input and output file must differ" << endl;
      return 1;
    }
  }

  return 0;
}