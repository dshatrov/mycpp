/* MyNC - Linux Numerical Control System
 * Copyright (C) 2005  Dmitry M. Shatrov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __TSC_TSCMEASUREMENT_H__
#define __TSC_TSCMEASUREMENT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	long unsigned	tsc_low,
					tsc_high,
					tsc_low1,
					tsc_high1;
} TscMeasurement;

#define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

inline void tsc_start_measurement (TscMeasurement *tm) {
	rdtsc (tm->tsc_low, tm->tsc_high);
}

inline void tsc_stop_measurement (TscMeasurement *tm) {
	rdtsc (tm->tsc_low1, tm->tsc_high1);
}

inline long long unsigned tsc_get_ticks (const TscMeasurement *tm)
{
	long long unsigned tsc, tsc1;
	tsc = (long long unsigned) tm->tsc_high << 32 |
		((long long unsigned) tm->tsc_low & 0xffffffff);
	tsc1 = (long long unsigned) tm->tsc_high1 << 32 |
		((long long unsigned) tm->tsc_low1 & 0xffffffff);
	return tsc1 - tsc;
}

#ifdef __cplusplus
}
#endif

#endif /*__TSC_TSCMEASUREMENT_H__*/

