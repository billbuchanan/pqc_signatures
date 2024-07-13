#define _GNU_SOURCE
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <stdint.h>
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
    read (fd, count_ptr, sizeof (uint64_t));    \
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

#include "biscuit.h"

static void
memrand (uint8_t *dst, size_t n)
{
  while (n--)
    *dst++ = rand ();
}

int
main (int argc, char *argv[])
{
  int i;
  int status = 0;
  int nb_tests = 100;
#ifndef FIXED_PARAMS
  params_t params[1] = {{0}};
#endif
#ifndef SEC_LEVEL
  int lambda = 0;
#else
  const int lambda = SEC_LEVEL;
#endif
#ifndef NB_ITERATIONS
  int tau = 0;
#else
  const int tau = NB_ITERATIONS;
#endif
#ifndef NB_PARTIES
  int N = 0;
#else
  const int N = NB_PARTIES;
#endif
#ifndef FIELD_SIZE
  int q = 0;
#else
  const int q = FIELD_SIZE;
#endif
#ifndef NB_VARIABLES
  int n = 0;
#else
  const int n = NB_VARIABLES;
#endif
#ifndef NB_EQUATIONS
  int m = 0;
#else
  const int m = NB_EQUATIONS;
#endif
#ifndef DEGREE
  int d = 0;
#else
  const int d = DEGREE;
#endif

  struct perf_event_attr pe;
  long fd;
  uint64_t count;
  double acc;

  fd = init_perf (&pe);
  if (fd == -1)
    {
      fprintf (stderr, "Error opening leader\n");
      exit (EXIT_FAILURE);
    }

  srand (0);

  for (i = 1; i < argc; i++)
    {
      int v;
      if (memcmp (argv[i], "lambda=", 7) == 0)
        {
          v = atoi (argv[i] + 7);
#ifdef SEC_LEVEL
          if (v != SEC_LEVEL)
            {
              printf ("found lambda=%d !! not used !! ", v);
              printf ("Will use fixed lambda=%d instead\n", SEC_LEVEL);
            }
#else
          params->lambda = lambda = v;
#endif
        }
      else if (memcmp (argv[i], "tau=", 4) == 0)
        {
          v = atoi (argv[i] + 4);
#ifdef NB_ITERATIONS
          if (v != NB_ITERATIONS)
            {
              printf ("found tau=%d !! not used !! ", v);
              printf ("Will use fixed tau=%d instead\n", NB_ITERATIONS);
            }
#else
          params->tau = tau = v;
#endif
        }
      else if (memcmp (argv[i], "N=", 2) == 0)
        {
          v = atoi (argv[i] + 2);
#ifdef NB_PARTIES
          if (v != NB_PARTIES)
            {
              printf ("found N=%d !! not used !! ", v);
              printf ("Will use fixed N=%d instead\n", NB_PARTIES);
            }
#else
          params->N = N = v;
#endif
        }
      else if (memcmp (argv[i], "q=", 2) == 0)
        {
          v = atoi (argv[i] + 2);
#ifdef FIELD_SIZE
          if (v != FIELD_SIZE)
            {
              printf ("found q=%d !! not used !! ", v);
              printf ("Will use fixed q=%d instead\n", FIELD_SIZE);
            }
#else
          params->q = q = v;
#endif
        }
      else if (memcmp (argv[i], "n=", 2) == 0)
        {
          v = atoi (argv[i] + 2);
#ifdef NB_VARIABLES
          if (v != NB_VARIABLES)
            {
              printf ("found n=%d !! not used !! ", v);
              printf ("Will use fixed n=%d instead\n", NB_VARIABLES);
            }
#else
          params->n = n = v;
#endif
        }
      else if (memcmp (argv[i], "m=", 2) == 0)
        {
          v = atoi (argv[i] + 2);
#ifdef NB_EQUATIONS
          if (v != NB_EQUATIONS)
            {
              printf ("found m=%d !! not used !! ", v);
              printf ("Will use fixed m=%d instead\n", NB_EQUATIONS);
            }
#else
          params->m = m = v;
#endif
        }
      else if (memcmp (argv[i], "d=", 2) == 0)
        {
          v = atoi (argv[i] + 2);
#ifdef DEGREE
          if (v != DEGREE)
            {
              printf ("found d=%d !! not used !! ", v);
              printf ("Will use fixed d=%d instead\n", DEGREE);
            }
#else
          params->d = d = v;
#endif
        }
      else
        {
          /* assume remaining is number of test. */
          nb_tests = atoi (argv[i]);
        }
    }

  {
    uint8_t *pk, *sk, *sig;
    uint8_t msg[128];
    uint64_t j, msglen;

    const int sklen = GET_PRIVKEY_BYTES (lambda, tau, N, q, n, m, d);
    const int pklen = GET_PUBKEY_BYTES (lambda, tau, N, q, n, m, d);
    const int siglen = GET_SIGNATURE_BYTES (lambda, tau, N, q, n, m, d);

    printf ("Run %d tests for each function and take the mean "
            "number of cycles\n", nb_tests);
    printf ("========================================"
            "========================================\n");

    sk = malloc (GET_PRIVKEY_BYTES (lambda, tau, N, q, n, m, d));
    pk = malloc (GET_PUBKEY_BYTES (lambda, tau, N, q, n, m, d));
    sig = malloc (GET_SIGNATURE_BYTES (lambda, tau, N, q, n, m, d));

    printf ("params: lambda=%d, tau=%d, N=%d, "
            "q=%d, n=%d, m=%d, d=%d\n", lambda, tau, N, q, n, m, d);
    printf ("sizes: sk=%d bytes, pk=%d bytes, sig=%d bytes\n",
            sklen, pklen, siglen);

    memrand (sk, lambda >> 2);
    acc = 0;
    for (i = 0; i < nb_tests; i++)
      {
        PERF (keygen (sk, pk, sk, params), &count);
        acc += count;
      }
    printf ("keygen: %f cycles\n", acc / nb_tests);

    msglen = sizeof msg;
    for (j = 0; j < msglen; j++)
      msg[j] = j;

    memrand (sig, lambda >> 2);
    acc = 0;
    for (i = 0; i < nb_tests; i++)
      {
        PERF (sign (sig, msg, msglen, sk, sig, params), &count);
        acc += count;
      }
    printf ("sign: %f cycles\n", acc / nb_tests);

    acc = 0;
    for (i = 0; i < nb_tests; i++)
      {
        PERF (status = verify (sig, msg, msglen, pk, params), &count);
        acc += count;
      }
    printf ("verif (%s): %f cycles\n", status == 0 ? "ok" : "ko",
            acc / nb_tests);

    free (sk);
    free (pk);
    free (sig);
  }

  return status;
}
