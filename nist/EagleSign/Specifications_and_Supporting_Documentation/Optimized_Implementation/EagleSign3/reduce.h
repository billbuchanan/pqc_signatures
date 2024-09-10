/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#ifndef REDUCE_H
#define REDUCE_H

#include <stdint.h>
#include "params.h"

#define add EAGLESIGN_NAMESPACE(add)
S_DOUBLE_Q_SIZE add(S_DOUBLE_Q_SIZE x, S_DOUBLE_Q_SIZE y);

#define sub EAGLESIGN_NAMESPACE(sub)
S_DOUBLE_Q_SIZE sub(S_DOUBLE_Q_SIZE x, S_DOUBLE_Q_SIZE y);

#define rshift1 EAGLESIGN_NAMESPACE(rshift1)
S_DOUBLE_Q_SIZE rshift1(S_DOUBLE_Q_SIZE x);

#define montymul EAGLESIGN_NAMESPACE(montymul)
S_DOUBLE_Q_SIZE montymul(S_DOUBLE_Q_SIZE x, S_DOUBLE_Q_SIZE y);

#define montymul2 EAGLESIGN_NAMESPACE(montymul2)
S_DOUBLE_Q_SIZE montymul2(S_DOUBLE_Q_SIZE x, S_DOUBLE_Q_SIZE y);

#define addq EAGLESIGN_NAMESPACE(addq)
Q_SIZE addq(S_Q_SIZE r);

#define reduce EAGLESIGN_NAMESPACE(reduce)
S_DOUBLE_Q_SIZE reduce(S_DOUBLE_Q_SIZE y);

#endif
