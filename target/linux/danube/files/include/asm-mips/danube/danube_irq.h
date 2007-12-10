/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2005 infineon
 *   Copyright (C) 2007 John Crispin <blogic@openwrt.org> 
 *
 */
#ifndef _DANUBE_IRQ__
#define _DANUBE_IRQ__

#define INT_NUM_IRQ0			8
#define INT_NUM_IM0_IRL0		(INT_NUM_IRQ0 + 0)
#define INT_NUM_IM1_IRL0		(INT_NUM_IRQ0 + 32)
#define INT_NUM_IM2_IRL0		(INT_NUM_IRQ0 + 64)
#define INT_NUM_IM3_IRL0		(INT_NUM_IRQ0 + 96)
#define INT_NUM_IM4_IRL0		(INT_NUM_IRQ0 + 128)
#define INT_NUM_IM_OFFSET		(INT_NUM_IM1_IRL0 - INT_NUM_IM0_IRL0)

#define DANUBEASC1_TIR			(INT_NUM_IM3_IRL0 + 7)
#define DANUBEASC1_RIR			(INT_NUM_IM3_IRL0 + 9)
#define DANUBEASC1_EIR			(INT_NUM_IM3_IRL0 + 10)

#define DANUBE_TIMER6_INT		(INT_NUM_IM1_IRL0 + 23)

#define MIPS_CPU_TIMER_IRQ		7

#define DANUBE_DMA_CH0_INT		(INT_NUM_IM2_IRL0)
#define DANUBE_DMA_CH1_INT		(INT_NUM_IM2_IRL0 + 1)
#define DANUBE_DMA_CH2_INT		(INT_NUM_IM2_IRL0 + 2)
#define DANUBE_DMA_CH3_INT		(INT_NUM_IM2_IRL0 + 3)
#define DANUBE_DMA_CH4_INT		(INT_NUM_IM2_IRL0 + 4)
#define DANUBE_DMA_CH5_INT		(INT_NUM_IM2_IRL0 + 5)
#define DANUBE_DMA_CH6_INT		(INT_NUM_IM2_IRL0 + 6)
#define DANUBE_DMA_CH7_INT		(INT_NUM_IM2_IRL0 + 7)
#define DANUBE_DMA_CH8_INT		(INT_NUM_IM2_IRL0 + 8)
#define DANUBE_DMA_CH9_INT		(INT_NUM_IM2_IRL0 + 9)
#define DANUBE_DMA_CH10_INT		(INT_NUM_IM2_IRL0 + 10)
#define DANUBE_DMA_CH11_INT		(INT_NUM_IM2_IRL0 + 11)
#define DANUBE_DMA_CH12_INT		(INT_NUM_IM2_IRL0 + 25)
#define DANUBE_DMA_CH13_INT		(INT_NUM_IM2_IRL0 + 26)
#define DANUBE_DMA_CH14_INT		(INT_NUM_IM2_IRL0 + 27)
#define DANUBE_DMA_CH15_INT		(INT_NUM_IM2_IRL0 + 28)
#define DANUBE_DMA_CH16_INT		(INT_NUM_IM2_IRL0 + 29)
#define DANUBE_DMA_CH17_INT		(INT_NUM_IM2_IRL0 + 30)
#define DANUBE_DMA_CH18_INT		(INT_NUM_IM2_IRL0 + 16)
#define DANUBE_DMA_CH19_INT		(INT_NUM_IM2_IRL0 + 21)

extern void mask_and_ack_danube_irq (unsigned int irq_nr);

#endif