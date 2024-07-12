/**
 * \file ryde_128s_verify.h
 * \brief NIST SIGNATURE VERIFICATION API used by the RYDE scheme
 */

#ifndef RYDE_128S_VERIFY_H
#define RYDE_128S_VERIFY_H

int ryde_128s_verify(const uint8_t* signature, size_t signature_size, const uint8_t* message, size_t message_size, const uint8_t* pk);

#endif

