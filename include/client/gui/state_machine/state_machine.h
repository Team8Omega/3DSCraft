//
// Created by Elias on 15.07.2024.
//

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_


typedef struct state_machine_s {
		void (*current_state)(struct state_machine_s *);

		/** The context of a state machine is some user-provided pointer which can
     * be used to store information about the state machine instance, such as
     * the current connection if the state machine is used to implement a server
     * for example. */
		void *context;
} state_machine_t;

typedef void (*state_func)(state_machine_t *);

/** Allocates memory for a new state machine. */
state_machine_t *state_machine_create(void);

/** Frees memory used by a state machine. */
void state_machine_delete(state_machine_t *machine);

/** Returns the current state of the machine. */
state_func state_machine_get_current_state(const state_machine_t *machine);

/** Sets the current state of the machine. */
void state_machine_set_current_state(state_machine_t *machine, state_func state);

/** Computes a single step (transition) of the machine. */
void state_machine_run(state_machine_t *machine);

/** Returns 1 if the machine is in a valid state, 0 otherwise. */
int state_machine_state_is_valid(const state_machine_t *machine);

/** "exits" the state machine (put it into invalid state so subsequent calls to
 * state_machine_run() will do nothing.
 */
void state_machine_exit(state_machine_t *machine);

/** Sets the user context value. */
void state_machine_set_context(state_machine_t *machine, void *context);

/** Gets the user specified context value. */
void *state_machine_get_context(const state_machine_t *machine);

#endif
