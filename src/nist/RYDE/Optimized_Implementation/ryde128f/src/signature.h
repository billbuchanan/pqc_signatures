/** 
 * @file ryde_128f_sign.h
 * @brief Sign algorithm of the RYDE scheme
 */

#ifndef RYDE_128F_SIGN_H
#define RYDE_128F_SIGN_H

int ryde_128f_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk);

#endif

