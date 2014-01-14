
#include <sys/time.h>
#include "utils.h"
#include "sre.h"
using namespace ucltp;

int main(int argc, char** argv)
{
  struct timeval sTime, eTime;
  gettimeofday(&sTime, NULL);

  vector<char_t> chars;
  read_utf8_text("12.56%", chars);
  
  for (int i=0; i<1000000; i+=1) {
    Sre re;
    re.compile("\\d+(,\\d\\d\\d)*(\\.\\d+)?(%)?");
    int len = re.match(chars, 0);
  }

  gettimeofday(&eTime, NULL);
  long exeTime = (eTime.tv_sec-sTime.tv_sec)*1000000+(eTime.tv_usec-sTime.tv_usec);
  printf("exeTime = %d us\n", exeTime);
  
  return 0;
}
