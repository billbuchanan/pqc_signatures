/** 
 * @file sign_mira_256_sign.h
 * @brief Sign algorithm of the MIRA scheme
 */

#ifndef SIGN_MIRA_256_SIGN_H
#define SIGN_MIRA_256_SIGN_H

int sign_mira_256_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk);

#endif

