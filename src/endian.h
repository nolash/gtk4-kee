#ifndef LASH_ENDIAN_H_
#define LASH_ENDIAN_H_

/**
 *
 * \brief Encapsulats all suppoerted integer lengths for endian conversion.
 *
 */
union le {
	short *s;
	int *i;
	long long *ll;
	unsigned char *c;
};

//int le(int l, void *n);
/*
 * Return true (1) if system is little-endian.
 */
int is_le();
/**
 * Convert to specified endian order.
 *
 * The integer data in \c n is changed in-place.
 *
 * If \c direction is same as system endianness, or \c l==1, no action is taken.
 *
 * \param direction 0 for big-endian, 1 for little-endian.
 * \param l Length of integer \c n in bytes.
 * \param n Integer data.
 * \return 1 if \c l is invalid byte size, 0 (success) otherwise.
 *
 */
int to_endian(char direction, int l, void *n);
/**
 * Change endian order of given number.
 *
 * The integer data in \c n is changed in-place.
 *
 * \param l Length of integer \c in bytes.
 * \param n Integer data.
 */
void flip_endian(int l, union le *n);

#endif // LASH_ENDIAN_H_
