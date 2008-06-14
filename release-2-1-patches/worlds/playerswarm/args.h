#include <libplayerc++/playerc++.h>
#include <iostream>
#include <unistd.h>

std::string  gHostname(PlayerCc::PLAYER_HOSTNAME);
uint         gPort(PlayerCc::PLAYER_PORTNUM);
uint         gIndex(0);
uint         gDebug(0);
uint         gFrequency(10); // Hz
uint         gDataMode(PLAYER_DATAMODE_PUSH);
bool         gUseLaser(false);
uint         gPop(1);

void print_usage(int argc, char** argv);

int parse_args(int argc, char** argv)
{
  // set the flags
  const char* optflags = "h:p:i:n:d:u:lm:";
  int ch;

  // use getopt to parse the flags
  while(-1 != (ch = getopt(argc, argv, optflags)))
  {
    switch(ch)
    {
      // case values must match long_options
      case 'h': // hostname
          gHostname = optarg;
          break;
      case 'n': // number of robots
          gPop = atoi(optarg);
          break;
      case 'p': // port
          gPort = atoi(optarg);
          break;
      case 'i': // index
          gIndex = atoi(optarg);
          break;
      case 'd': // debug
          gDebug = atoi(optarg);
          break;
      case 'u': // update rate
          gFrequency = atoi(optarg);
          break;
      case 'm': // datamode
          gDataMode = atoi(optarg);
          break;
      case 'l': // datamode
          gUseLaser = true;
          break;
      case '?': // help
      case ':':
      default:  // unknown
        print_usage(argc, argv);
        exit (-1);
    }
  }

  return (0);
} // end parse_args

void print_usage(int argc, char** argv)
{
  using namespace std;
  cerr << "USAGE:  " << *argv << " [options]" << endl << endl;
  cerr << "Where [options] can be:" << endl;
  cerr << "  -h <hostname>  : hostname to connect to (default: "
       << PlayerCc::PLAYER_HOSTNAME << ")" << endl;
  cerr << "  -p <port>      : port where Player will listen (default: "
       << PlayerCc::PLAYER_PORTNUM << ")" << endl;
  cerr << "  -i <index>     : device index (default: 0)"
       << endl;
  cerr << "  -n <number of robots>      : population size (default: "
       << 1 << ")" << endl;
  cerr << "  -d <level>     : debug message level (0 = none -- 9 = all)"
       << endl;
  cerr << "  -u <rate>      : set server update rate to <rate> in Hz"
       << endl;
  cerr << "  -l      : Use laser if applicable"
       << endl;
  cerr << "  -m <datamode>  : set server data delivery mode"
       << endl;
  cerr << "                      PLAYER_DATAMODE_PUSH = "
       << PLAYER_DATAMODE_PUSH << endl;
  cerr << "                      PLAYER_DATAMODE_PULL = "
       << PLAYER_DATAMODE_PULL << endl;
/*  cerr << "                      PLAYER_DATAMODE_PUSH_ALL = "
       << PLAYER_DATAMODE_PUSH_ALL << endl;
  cerr << "                      PLAYER_DATAMODE_PULL_ALL = "
       << PLAYER_DATAMODE_PULL_ALL << endl;
  cerr << "                      PLAYER_DATAMODE_PUSH_NEW = "
       << PLAYER_DATAMODE_PUSH_NEW << endl;
  cerr << "                      PLAYER_DATAMODE_PULL_NEW = "
       << PLAYER_DATAMODE_PULL_NEW << endl;
  cerr << "                      PLAYER_DATAMODE_ASYNC    = "
       << PLAYER_DATAMODE_ASYNC << endl;*/
} // end print_usage
