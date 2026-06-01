#ifndef RC4_H
#define RC4_H
/* ************************************************************************** */

/*!
 * \brief rc4_crypt
 * \param key
 * \param key_length
 * \param data
 * \param data_length
 */
void rc4_crypt(const unsigned char *key, const unsigned key_length,
               unsigned char *data, const unsigned data_length);

/* ************************************************************************** */
#endif // RC4_H
