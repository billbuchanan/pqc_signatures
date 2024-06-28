#define _GNU_SOURCE
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <inttypes.h>
#include <sys/types.h>

#include <time.h>

#define PERF(expr, count_ptr)                   \
  do {                                          \
    ioctl (fd, PERF_EVENT_IOC_RESET, 0);        \
    ioctl (fd, PERF_EVENT_IOC_ENABLE, 0);       \
    expr;                                       \
    ioctl (fd, PERF_EVENT_IOC_DISABLE, 0);      \
    read (fd, count_ptr, sizeof (long long));   \
  } while (0)
  

static long
perf_event_open (struct perf_event_attr *hw_event, pid_t pid,
                 int cpu, int group_fd, unsigned long flags)
{
  return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

static long
init_perf (struct perf_event_attr *pe)
{
  memset (pe, 0, sizeof (struct perf_event_attr));
  pe->type = PERF_TYPE_HARDWARE;
  pe->size = sizeof (struct perf_event_attr);
  pe->config = PERF_COUNT_HW_CPU_CYCLES;
  pe->disabled = 1;
  pe->exclude_kernel = 1;
  pe->exclude_hv = 1;
  return perf_event_open (pe, 0, -1, -1, 0);
}

#include "api.h"
#include "rng.h"

#define MSGBYTES 32

int
main (void)
{
  struct perf_event_attr pe;
  long fd;
  long long count;

  int status;
  unsigned char *pk, *sk, *msg, *m, *sm;
  unsigned long long mlen, smlen;
  unsigned char entropy_input[48] = { 0 };

  fd = init_perf (&pe);
  if (fd == -1)
    {
      fprintf (stderr, "Error opening leader %llx\n", pe.config);
      exit (EXIT_FAILURE);
    }

  pk = malloc (CRYPTO_PUBLICKEYBYTES);
  sk = malloc (CRYPTO_SECRETKEYBYTES);
  m = malloc (MSGBYTES);
  sm = malloc (CRYPTO_BYTES + MSGBYTES);
  msg = malloc (MSGBYTES);

  srand (time (NULL));
  entropy_input[0] = (unsigned char) rand ();
  randombytes_init (entropy_input, NULL, 128);

  for (int i = 0; i < MSGBYTES; i++)
    msg[i] = i;

  PERF (crypto_sign_keypair (pk, sk), &count);
  printf ("sign_keypair: %lld cycles\n", count);
  printf ("pk: %d bytes\n", CRYPTO_PUBLICKEYBYTES);
  printf ("sk: %d bytes\n", CRYPTO_SECRETKEYBYTES);

  PERF (crypto_sign (sm, &smlen, msg, MSGBYTES, sk), &count);
  printf ("sign: %lld cycles\n", count);
  printf ("sig: %d bytes\n", CRYPTO_BYTES);

  PERF (status = crypto_sign_open (m, &mlen, sm, smlen, pk), &count);
  printf ("sign_open: %lld cycles\n", count);
  printf ("status: %s\n", status == 0 ? "OK" : "KO");

  free (pk);
  free (sk);
  free (m);
  free (sm);
  free (msg);

  return status;
}
