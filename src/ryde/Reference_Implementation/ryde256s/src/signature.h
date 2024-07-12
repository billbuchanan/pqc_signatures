/** 
 * @file ryde_256s_sign.h
 * @brief Sign algorithm of the RYDE scheme
 */

#ifndef RYDE_256S_SIGN_H
#define RYDE_256S_SIGN_H

int ryde_256s_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk);

#endif

