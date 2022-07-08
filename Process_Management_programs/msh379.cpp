#include<iostream>
#include<cstdlib>
#include<unistd.h>
#include<sys/times.h>
#include<vector>
#include<csignal>
#include<sys/resource.h>

using namespace std;

void set_CPU_limit() {
  rlimit  rTimeLimit;
  rTimeLimit.rlim_cur = 1200; // Setting CPU time limit to 20 minutes
  // Setting time limit of CPU till it succeed
  if (setrlimit(RLIMIT_CPU, &rTimeLimit) < 0){
    cout << "Failed to set a limit on CPU time\n";
  }
  // while(setrlimit(RLIMIT_CPU, &rTimeLimit) < 0){
  //   continue;
  // }
  return; 
}

int run_command(string line, vector<pid_t>& tasks_pid,
 vector<string>& tasks_cmd, vector<bool>& tasks_state, vector<string> arguments){

  pid_t pid = fork();
  if (pid == 0){
    pid = getpid();
    string pgm = "";
    if (arguments[1][0] == '.' && arguments[1][1] == '/'){
      for (int i = 2; i < arguments[1].length(); i++){
        pgm += arguments[1][i];
      }
    }
    else{
      pgm = arguments[1];
    }

    if (arguments[1].compare("myclock")){
      pgm = "./" + arguments[1];
    }

    int no_of_arguments = arguments.size();

    switch (no_of_arguments)
    {
      case 6:{
      if (execlp(arguments[1].c_str(), pgm.c_str(), arguments[2].c_str(),
       arguments[3].c_str(), arguments[4].c_str(), arguments[5].c_str(),
        (char *) NULL) == -1) {
        return -1;
        }
        break;}
      case 5:{
        if (execlp(arguments[1].c_str(), pgm.c_str(), arguments[2].c_str(),
         arguments[3].c_str(), arguments[4].c_str(), (char *) NULL) == -1){
          return -1;
        }
        break;}
      case 4:{
        if (execlp(arguments[1].c_str(), pgm.c_str(), arguments[2].c_str(),
         arguments[3].c_str(), (char *) NULL) == -1){
          return -1;
        }
        break;}
      case 3:{
        if (execlp(arguments[1].c_str(), pgm.c_str(), arguments[2].c_str(),
         (char *) NULL) == -1){
          return -1;
        }
        break;}
      case 2:{
        if (execlp(arguments[1].c_str(), pgm.c_str(), (char *) NULL) == -1){
          return -1;
        }
        break;}
      default:{
        cout << "Too few argument try again\n";
        return -1;
        break;}
    }
  }
  tasks_pid.push_back(pid); tasks_cmd.push_back(line);
  tasks_state.push_back(true);
  return 0;
}

int cdir_command(vector<string> arguments){
  string path = "";
  if (arguments.size() < 1){
    cout << "Too few arguments try again\n";
    return -1;
  }
  path += "/" + arguments[1];
  if (chdir(path.c_str()) < 0)
    return -1;
  return 0;
}

int pdir_command(){
  char ptr[1024];
  if (getcwd(ptr, sizeof(ptr)) == NULL){
    return -1;
  }
  cout << "Current path: " << ptr << "\n";
  return 0;
}

int stop_command(vector<pid_t> tasks_pid, int index){
  pid_t pid = tasks_pid[index];
  if (kill(pid, SIGSTOP) < 0)
    return -1;
  return 0;
}

int Terminate_command(vector<pid_t> tasks_pid, vector<bool>& tasks_state, int index){
  pid_t pid = tasks_pid[index];
  if (tasks_state[index]) {
    if (kill(pid, SIGKILL) < 0)
      return -1;
  }
  tasks_state[index]= false;
  return 0;
}

void exit_command(vector<pid_t> tasks_pid, vector<bool> tasks_state){
  for (int i = 0; i < tasks_pid.size(); i++){
    if (tasks_state[i]){
      Terminate_command(tasks_pid, tasks_state, i);
    }
  }
  return;
}

int continue_command(vector<pid_t> tasks_pid, int index){
  pid_t pid = tasks_pid[index];
  if (kill(pid, SIGCONT) < 0)
    return -1;
  return 0;
}
/*
  Helper funtion to remove extra spaces
*/
string remove_extra_spaces(string original_str){
  string modified_str = "";
  for (int i = 0; i < original_str.length();){
    if (original_str[i] == ' '){
      if (i == 0 || i == original_str.length()-1){
        i++;
        continue;
      }
      while (original_str[i+1] == ' '){
        i++;
      }
    }
    modified_str += original_str[i++];
  }
  return modified_str;
}

string remove_command(string line, int length){
  string modified_str = "";
  for (int i = length+1; i < line.length(); i++)
    modified_str += line[i];
  return modified_str;
}

int main() {

  set_CPU_limit(); // Set's the CPU time limit to 20 minutes

  tms start_time, end_time;
  clock_t start, end;

  if ((start = times(&start_time)) == -1){
    cout << "Failed to get current time\n";
  }

  // while ((start = times(&start_time)) == -1){
  //   cout << "Trying to get current time\n";
  //   continue;
  // }

  pid_t pid;
  pid = getpid(); // Current process id
  bool exit_shell = false;
  vector<pid_t> tasks_pid;
  vector<string> tasks_cmd;
  vector<bool> tasks_state;

  while(!exit_shell){
    string line = "";
    cout << "msh379 [" << pid << "]:";
    getline(cin, line);

    if (line == "")
      continue;

    line = remove_extra_spaces(line);

    // First element in argument will be the user command
    vector<string> arguments;
    string temp = "";

    for (int i = 0; i < line.length(); i++){
      if (line[i] == ' '){
        arguments.push_back(temp);
        temp = "";
      }
      else{
        if (line[i] != '$')
          temp += line[i];
      }
    }
    if (temp != ""){
      arguments.push_back(temp);
      temp = "";
    }

    // arguments[0] == user command
    if (arguments[0].compare("run") == 0){
      line = remove_command(line, 3);
      if (run_command(line, tasks_pid, tasks_cmd, tasks_state, arguments) < 0)
        cout << "execlp is unsuccessful Try again\n";
      continue;
    }
    else
    {
    if (arguments[0].compare("cdir") == 0){
      if (cdir_command(arguments) < 0)
        cout << "cdir command unsuccessful Try agian\n";
      continue;
    }
    else 
    {
    if (arguments[0].compare("pdir") == 0){
      if (pdir_command() < 0)
        cout << "pdir command unsuccessful Try again\n";
      continue;
    }
    else
    {
    if (arguments[0].compare("stop") == 0){
      if (arguments.size() != 2){
        cout << "Incorrect arguments Try again\n";
        continue;
      }
      if (stop_command(tasks_pid, stoi(arguments[1])) < 0){
        cout << "Something went wrong Try again\n";
          continue;
      }
    }
    else
    {
    if (arguments[0].compare("exit") == 0){
      exit_command(tasks_pid, tasks_state);
      break;
    }
    else
    {
    if (arguments[0].compare("quit") == 0){
      break;
    }
    else
    {
    if (arguments[0].compare("check") == 0){
      if (arguments.size() != 2){
        cout << "Incorrect arguments Try again\n";
        continue;
      }
      vector<string> processed_lines;
      char curr_line[100];

      FILE *result = popen("ps -u", "r");
      if (result == NULL){
        cout << "Something went wrong Try again\n";
      }
      else
      {
      int check = 0;
      while (!feof(result))
      {
      if (fgets(curr_line, 100, result) == NULL)
        break;
      else{
        string cmd = curr_line;
        if (cmd.find(arguments[1]) != string::npos){
          cout << curr_line << endl;
          check++;
        }
      }
      }
      if (check == 0){
        cout << " The target pid is defunct or not found\n";
      }
      pclose(result);
      }
      continue;
    }
    else
    {
    if (arguments[0].compare("lstasks") == 0){
      for (int i = 0; i < tasks_pid.size(); i++){
        if (tasks_state[i]){
          cout << "\n " << i << ": (pid= " << tasks_pid[i] << ", cmd = " << tasks_cmd[i] << ")\n";
        }
      }
    }
    else
    {
    if (arguments[0].compare("continue") == 0){
      if (arguments.size() != 2){
        cout << "Incorrect arguments Try again\n";
        continue;
      }
      if (continue_command(tasks_pid, stoi(arguments[1])) < 0){
        cout << "Something went wrong Try again\n";
        continue;
      }
    }
    else
    {
    if (arguments[0].compare("terminate") == 0){
      if (arguments.size() != 2){
        cout << "Incorrect arguments Try again\n";
        continue;
      }
      if (Terminate_command(tasks_pid, tasks_state, stoi(arguments[1])) < 0){
        cout << "Something went wrong Try again\n";
        continue;
      }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
  }

  while ((end = times(&end_time)) == -1){
    continue;
  }

  long double clocktck = 0;
  if ((clocktck = sysconf(_SC_CLK_TCK)) < 0) {  }
  cout << "    real:          " << (end-start) / clocktck << " sec.\n";
  cout << "    user:          " << (end_time.tms_utime - start_time.tms_utime) /  clocktck << " sec.\n";
  cout << "    sys:           " << (end_time.tms_stime - start_time.tms_stime) /  clocktck << " sec.\n";
  cout << "    child user:    " << (end_time.tms_cutime - start_time.tms_cutime) /  clocktck << " sec.\n";
  cout << "    child sys:     " << (end_time.tms_cstime - start_time.tms_cstime) /  clocktck << " sec.\n";
  return 0;
}
