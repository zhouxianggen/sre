
#include <sys/time.h>
#include "utils.h"
#include "sre.h"
using namespace ucltp;

int main(int argc, char** argv)
{
  struct timeval sTime, eTime;
  gettimeofday(&sTime, NULL);

  vector<char_t> chars;
  read_utf8_text(argv[2], chars);
  
  Sre re;
  re.build(argv[1]);
  gettimeofday(&sTime, NULL);
  match_result_t r = re.match(chars, 0);
  printf("match result is, len=%d, type=%d\n", r._len, r._type);

  gettimeofday(&eTime, NULL);
  long exeTime = (eTime.tv_sec-sTime.tv_sec)*1000000+(eTime.tv_usec-sTime.tv_usec);
  printf("exeTime = %d us\n", exeTime);
  
  return 0;
}
