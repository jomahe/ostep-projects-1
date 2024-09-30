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

vector<char*> paths{"/bin"};

void parseInput(FILE* &file, vector<char*> &commands) {
  if (!file) {
    cerr << "Invalid file!" << endl;
    exit(1);
  }

  char* line = nullptr;
  size_t lineLen = 0;

  while (getline(&line, &lineLen, file) != 0) {
    commands.push_back(line);
  }

  free(line);
}

void executeCommand(char* &command) {
  /* 1. Fork the process */
  pid_t rc = fork();

  if (rc > 0) {  // Parent process waits for the child, processes exit status
    int* status;
    waitpid(rc, status, 0);

    handleStatus(status);
  } else if (rc == 0) {  // Child process calls execv to execute command
    /* Parse the command into its constituent arguments with strsep */
    vector<char*> args;
    char* currArg;

    while((currArg = strsep(&command, " ")) != NULL) {
      args.push_back(currArg);
    }

    /* TODO: comment these two lines out after testing */
    cout << "Parsed input: " << args[0];
    for (size_t i = 1; i < args.size(); ++i) cout << " " << args[i];

    args.push_back(NULL);


    /* Check to see if the command is executable in any of the search paths */
    if (!inSearchPath(args[0])) {
      /* Handle an improper executable here*/
      exit(1);
    }

    /* Handle built-in commands */
    if (args[0] == "exit") {
      exit(0);
    } else if (args[0] == "cd") {

    } else if (args[0] == "path") {

    } else {
      /* Execute the command in the CLI with execv */
      execv(args[0], const_cast<char**>(args.data()));
    }
  } else {
    cerr << "Fork failed" << endl;
    exit(1);
  }
}

char* acceptInput() {
  string currLine;
  getline(cin, currLine);

  return strdup(currLine.c_str());
}

bool inSearchPath(char* executable) {

  return false;
}

void handleStatus(int* &status) {
  if (WIFEXITED(status)) {
    cout << "Child exited with status: " << WEXITSTATUS(status) << endl;
  } else if (WIFSIGNALED(status)) {
    cout << "Child terminated by signal: " << WTERMSIG(status) << endl;
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
    vector<char*> commands;

    if (!(in = fopen(argv[2], "r"))) {
      cerr << "wish: cannot open file: '" << argv[2] << "'"<< endl;
      exit(1);
    }

    parseInput(in, commands);
    for (char* &command : commands) {
      executeCommand(command);
    }
  } else {
    /* Enter interactive mode; allow user to type in one command at a time */
    while(true) {
      cout << "wish> ";
      char* command = acceptInput();
      executeCommand(command);
    }
  }
  return 0;
}