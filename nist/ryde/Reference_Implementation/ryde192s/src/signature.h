/** 
 * @file ryde_192s_sign.h
 * @brief Sign algorithm of the RYDE scheme
 */

#ifndef RYDE_192S_SIGN_H
#define RYDE_192S_SIGN_H

int ryde_192s_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk);

#endif

