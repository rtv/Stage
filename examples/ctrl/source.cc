#include "stage.hh"
using namespace Stg;

const double FLAGSZ = 0.25;

typedef struct {
  unsigned int capacity;
} info_t;

int Update(Model *mod, info_t *info)
{
  if (mod->GetFlagCount() < info->capacity) {
    // printf( "Source adding flag to model %s\n", mod->Token() );
    mod->PushFlag(new Model::Flag(Color(1, 1, 0), FLAGSZ));
  }
  return 0; // run again
}

void split(const std::string &text, const std::string &separators, std::vector<std::string> &words)
{
  int n = text.length();
  int start = text.find_first_not_of(separators);
  while ((start >= 0) && (start < n)) {
    int stop = text.find_first_of(separators, start);
    if ((stop < 0) || (stop > n))
      stop = n;
    words.push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(separators, stop + 1);
  }
}

// Stage calls this when the model starts up
extern "C" int Init(Model *mod, CtrlArgs *args)
{
  puts("Starting source controller");

  // tokenize the argument string into words
  std::vector<std::string> words;
  split(args->worldfile, std::string(" \t"), words);

  // expect a capacity as the 1th argument
  assert(words.size() == 2);
  assert(words[1].size() > 0);

  info_t *info = new info_t;
  info->capacity = atoi(words[1].c_str());

  printf("Source Capacity: %u\n", info->capacity);

  mod->AddCallback(Model::CB_UPDATE, (model_callback_t)Update, info);
  mod->Subscribe();
  return 0; // ok
}
