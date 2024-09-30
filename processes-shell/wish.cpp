#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

void parseInput(FILE* &file, vector<string>& commands) {
  if (!file) {
    cerr << "Invalid file!" << endl;
    exit(1);
  }

  char* line = nullptr;
  size_t lineLen = 0;

  while (getline(&line, &lineLen, file) != 0) {
    commands.push_back(string(line));
  }

  free(line);
}

void handleCommandStatus(int& status) {
  if (WIFEXITED(status) != 0) {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
  }

  // Otherwise, command status should be 0 meaning command exited successfully
}

void batchExecute(vector<string>& commands) {
  for (string& command : commands) {
    int rc = fork();
    if (rc < 0) {

    } else if (rc == 0) {
      cout << "Running command: " << command << " on child process with pid: " << getpid() << endl;
      int rc = system(command.c_str());
      handleCommandStatus(rc);
    } else {
      int rcWait = wait(NULL);
      cout << "Waiting for child with pid: "<< rcWait << endl;
    }
  }
}


int main(int argc, char* argv[]) {
  // We should be invoking the executable with either one or zero arguments
  if (argc > 2) {
    cerr << "Usage: ./wish <OPTIONAL FILENAME>" << endl;
    exit(1);

  } else if (argc == 2) {
    /* Read input from a batch file, execute the commands therein */
    FILE* in = nullptr;
    vector<string> commands;

    if (!(in = fopen(argv[2], "r"))) {
      cerr << "wish: cannot open file: '" << argv[2] << "'"<< endl;
      exit(1);
    }

    parseInput(in, commands);
    batchExecute(commands);
  } else {
    /* Enter interactive mode; allow user to type in one command at a time */
    while(true) {

    }
  }
  return 0;
}