#ifndef BIT_MATH_H
#define BIT_MATH_H

/* Single bit operations */
#define SET_BIT(REG, BIT)          ((REG) |= (1U << (BIT)))
#define CLR_BIT(REG, BIT)          ((REG) &= ~(1U << (BIT)))
#define TOG_BIT(REG, BIT)          ((REG) ^= (1U << (BIT)))
#define GET_BIT(REG, BIT)          (((REG) >> (BIT)) & 1U)

/* Mask operations */
#define SET_MASK(REG, MASK)        ((REG) |= (MASK))
#define CLR_MASK(REG, MASK)        ((REG) &= ~(MASK))

/* Bit-field operations */
/* MASK should be the value covering the field (e.g., 0x07 for 3 bits) */
#define WRITE_FIELD(REG, MASK, SHIFT, VAL) \
    ((REG) = ((REG) & ~((MASK) << (SHIFT))) | (((VAL) & (MASK)) << (SHIFT)))

#define READ_FIELD(REG, MASK, SHIFT) \
    (((REG) >> (SHIFT)) & (MASK))

#endif /* BIT_MATH_H */