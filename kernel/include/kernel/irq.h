#ifndef _IRQ_H
#define _IRQ_H

#include <stdint.h>

uint16_t __pic_get_irq_reg(int ocw3);

uint16_t pic_get_irr(void);

uint16_t pic_get_isr(void);

void IRQ_set_mask(uint8_t IRQline);

void IRQ_clear_mask(uint8_t IRQline);

#endif