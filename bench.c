//#include <asm/msr.h>
#include <stdio.h>
#include <stdlib.h>

#define rdtsc(low,high) \
     __asm__ __volatile__("mfence" ::: "memory"); \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high)); \
     __asm__ __volatile__("mfence" ::: "memory");



typedef unsigned long long u64;

static inline u64 rdtsc_read(void)
{
	unsigned long high, low;
	rdtsc(low,high);
	return ((u64)high << 32) | low;
}

int* hist;
int histsize;

main(int argc, char* argv[])
{
  int maxhist = 10000;
  int bufsize = 1000000;
  int stride  = 984567;
  int maxaccs = 50;
  int rounds, cycle, bufentries;
  void **buf, **pos;
  int i, inext, j, accs = 0;
  u64 ov, diff, start, maxdiff;
  int istart, iend, res = 0;
  void* dummyP;

  dummyP = (void*) & dummyP;

  if (argc >1) bufsize = atoi(argv[1]) * 1000;
  if (argc >2) maxaccs = atoi(argv[2]);
  if (argc >3) stride = atoi(argv[3]);

  if ((bufsize == 0) || (maxaccs == 0) || (stride == 0)) {
     fprintf(stderr,
	     "Memlat: Histogram of access latencies from linked-list traversal.\n"
	     "Usage: %s [<size> [<accs> [<stride>]]]\n\n"
	     " <size>    Buffer size in K (def. 1000 for 1MB)\n"
	     " <accs>    Maximal number of accesses done in Mio. (def. 50)\n"
	     " <stride>  Stride between accesses (def. huge for pseudo-random)\n", argv[0]);
     exit(1);
  }
  
  bufentries = bufsize / sizeof(void*);
  buf = (void**) malloc( bufsize );
  for(i = 0;i<bufentries;i++)
    buf[i] = buf;

  fprintf(stderr, "Buf at %p (size %d, entries %d), Stride %d.\n",
	 buf, bufsize, bufentries, stride);
  
  cycle = 0;
  i = 0;
  do {
    inext = i+stride;
    while(inext>=bufentries) inext -= bufentries;
    buf[i] = buf + inext;
    cycle++;
    i = inext;
  } while(i != 0);

  fprintf(stderr,"=> CycleLen %d\n", cycle);

  maxdiff = 0;
  for(i=0;i<2;i++) {
    pos = buf;
    for(j=0;j<cycle;j++) {
      start = rdtsc_read();
      pos = *pos;
      diff = rdtsc_read() - start;
      if (diff>maxhist) continue;
      if (maxdiff < diff) maxdiff = diff;
    }
  }

  res += (int)(pos-buf);

  fprintf(stderr, "Max. CPU Cycles: %llu\n", maxdiff);

  histsize = maxdiff + 100;
  hist = (int*) malloc( histsize * sizeof(int) );
  for(i = 0;i<histsize;i++)
    hist[i] = 0;

  rounds = (int)(1000000.0 * maxaccs / cycle + 0.5);

  fprintf(stderr, "Running %d Iterations...\n",rounds);
  for(i=0;i<rounds;i++) {
    pos = buf;
    for(j=0;j<cycle;j++) {
      start = rdtsc_read();
      pos = *pos;
      diff = rdtsc_read() - start;
      if (diff>histsize) continue;
      hist[(diff/5)*5]++;
      accs++;
    }
  }

  res += (int)(pos-buf);

  fprintf(stderr, "Done %d accesses...\n", accs);

  ov = 9999;
  pos = dummyP;
  for(j=0;j<cycle;j++) {
    start = rdtsc_read();
    //pos = *pos;
    diff = rdtsc_read() - start;
    if (diff<ov) ov = diff;
  }
  fprintf(stderr, "Measurement overhead: %llu cycles\n", ov);
  
#if 0
  istart=0;
  iend = histsize-1;
#else
  for(istart = 0;istart<histsize;istart++)
    if (hist[istart] > accs/1000) break;
  for(iend = histsize-1;iend>=0;iend--)
    if (hist[iend] > accs/1000) break;
#endif

  //if (istart<ov) istart = ov;
  //iend += 20;
  //if (iend>=histsize) iend = histsize-1;
  
  fprintf(stderr, "Dumping histogram [%d;%d] (res %d)\n",
	  istart-ov, iend-ov, res);


  for(i = istart;i<iend;i++)
    printf("%d %.2f\n", i-ov, 1000.0 * hist[i] / accs );
}
