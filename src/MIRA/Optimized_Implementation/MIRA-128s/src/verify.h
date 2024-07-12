/**
 * \file sign_mira_128_verify.h
 * \brief NIST SIGNATURE VERIFICATION API used by the SIGN_MARS_CODE scheme
 */

#ifndef SIGN_MIRA_128_VERIFY_H
#define SIGN_MIRA_128_VERIFY_H

int sign_mira_128_verify(const uint8_t* signature, size_t signature_size, const uint8_t* message, size_t message_size, const uint8_t* pk);

#endif

