#include "fault_injection.h"

#include <stdlib.h>
#include <inttypes.h>

#define PROBABILITY_OF_RANDOM_FAULT 0.3

int random_fault_injected = 0;
int stuck_at_fault_int_injected = 0;
int stuck_at_fault_double_injected = 0;

/* Position of the fault */
int stuck_at_bit_int = 0;
int stuck_at_bit_double = 0;

/* Value to which the bit is stuck at */
int stuck_at_value_int = 0;
int stuck_at_value_double = 0;

void fault_injection_reset()
{
    random_fault_injected = 0;
    stuck_at_fault_int_injected = 0;
    stuck_at_fault_double_injected = 0;
}

static int faulty_int_random (int original_value)
{
    if (! random_fault_injected)
    {
        if (rand() < (RAND_MAX * PROBABILITY_OF_RANDOM_FAULT))
        {
                // Inject single-bit fault at a randomly chosen bit
            const int faulty_bit = rand() % (8 * sizeof(int));
            const int fault_mask = (1 << faulty_bit);
            random_fault_injected = 1;
            return original_value ^ fault_mask;
        }
    }

        // No fault to inject
    return original_value;
}

static double faulty_double_random (double original_value)
{
    if (! random_fault_injected)
    {
        if (rand() < (RAND_MAX * PROBABILITY_OF_RANDOM_FAULT))
        {
                // Inject single-bit fault at a randomly chosen bit
                // (notice that the resulting faulty value could end 
                // up being special values like infinity, NaN, etc.)
            const int faulty_bit = rand() % (8 * sizeof(double));
            const uint64_t fault_mask = (((uint64_t)1) << faulty_bit);
            uint64_t * const p_int = (uint64_t *)&original_value;
            *p_int ^= fault_mask;   // we're modifying original_value, which is 
                                    // our local copy of client code's value
            random_fault_injected = 1;
            return original_value;
        }
    }

        // No fault to inject
    return original_value;    
}

static int faulty_int_stuck_at (int original_value)
{
    int fault_mask;
    if (! stuck_at_fault_int_injected)
    {
        stuck_at_fault_int_injected = 1;
        stuck_at_bit_int = rand() % (8 * sizeof(int));
        stuck_at_value_int = rand() & 0x1;
    }

    fault_mask = (1 << stuck_at_bit_int);
    if (stuck_at_value_int == 0)
    {
        original_value &= (~fault_mask);
    }
    else if (stuck_at_value_int == 1)
    {
        original_value |= fault_mask;
    }

    return original_value;
}

static double faulty_double_stuck_at (double original_value)
{
    uint64_t fault_mask;
	uint64_t * p_int;

    if (! stuck_at_fault_double_injected)
    {
        stuck_at_fault_double_injected = 1;
        stuck_at_bit_int = rand() % (8 * sizeof(double));
        stuck_at_value_int = rand() & 0x1;
    }

    fault_mask = (((uint64_t)1) << stuck_at_bit_int);
    p_int = (uint64_t *)&original_value;
    if (stuck_at_value_int == 0)
    {
        *p_int &= (~fault_mask);
    }
    else if (stuck_at_value_int == 1)
    {
        *p_int |= fault_mask;
    }

    return original_value;
}

int faulty_int (int original_value, int fault_type)
{
    if (fault_type == RANDOM_FAULT)
    {
        return faulty_int_random (original_value);
    }
    else if (fault_type == STUCK_AT_FAULT)
    {
        return faulty_int_stuck_at (original_value);
    }

    return original_value;
}

double faulty_double (double original_value, int fault_type)
{
    if (fault_type == RANDOM_FAULT)
    {
        return faulty_double_random (original_value);
    }
    else if (fault_type == STUCK_AT_FAULT)
    {
        return faulty_double_stuck_at (original_value);
    }

    return original_value;
}
