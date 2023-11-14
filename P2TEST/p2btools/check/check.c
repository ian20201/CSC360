#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define failf(...) do { \
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
		exit(1); \
	} while (0)

#define failerrno(func) do { \
		perror(func); \
		exit(1); \
	} while (0)

typedef struct train Train;
typedef struct event Event;

enum {
	WEST,
	EAST,
};

enum {
	// We require LOW < HIGH.
	LOW,
	HIGH,
};

enum {
	LOADING,
	READY,
	CROSSING,
	FINISHED,
};

struct train {
	uint8_t dir;
	uint8_t prio;
	uint8_t load_time;
	uint8_t cross_time;
	uint8_t state;
};

struct event {
	size_t train;
	uint32_t time;
	uint8_t type;
	uint8_t dir;
};

// Read a plain text file into an allocated buffer. A trailing newline is
// added if the last line doesn't have one already.
char *
read_text(char *path) {
	struct stat sb;
	if (stat(path, &sb) < 0) failerrno("stat");
	size_t size = sb.st_size;

	char *data = malloc(size + 2); // +1 for trailing newline, +1 for null
	if (!data) failerrno("malloc");

	FILE *file = fopen(path, "r");
	if (!file) failerrno("fopen");

	if (!fread(data, size, 1, file)) failerrno("fread");
	if (size > 0 && data[size-1] != '\n') data[size++] = '\n';
	data[size] = '\0';

	fclose(file);

	return data;
}

size_t
count_lines(char *raw) {
	size_t n = 0;
	for (; (raw = strchr(raw, '\n')); raw++) n += 1;
	return n;
}

// Parse exactly nfields fields from line delimited by at least 1 whitespace
// character, and put the results in fields[0],...,fields[nfields-1]. Leading
// and trailing whitespace are ignored. line is destructively updated.
void
parse_fields(size_t nfields, char **fields, char *line) {
	for (size_t i = 0; i < nfields; i++) {
		while (isspace(*line)) line++;
		if (!*line) failf("invalid format: expected %zu fields", nfields);
		fields[i] = line;
		while (*line && !isspace(*line)) line++;
		if (*line) *line++ = '\0';
	}
	while (isspace(*line)) line++;
	if (*line) failf("invalid format: expected %zu fields", nfields);
}

size_t
parse_trains(Train **trains, char *raw) {
	size_t ntrains = count_lines(raw);
	*trains = malloc(ntrains * sizeof **trains);
	if (!*trains) failerrno("malloc");

	for (size_t i = 0; i < ntrains; i++) {
		char *line = raw;
		raw = strchr(raw, '\n');
		*raw++ = '\0';

		char *fields[3];
		parse_fields(3, fields, line);

		if (strlen(fields[0]) != 1 || strspn(fields[0], "wWeE") != 1)
			failf("invalid format: train direction");
		char *end;
		long load_time = strtol(fields[1], &end, 10);
		if (*end != 0 || load_time < 1 || load_time > 99)
			failf("invalid format: train load time");
		long cross_time = strtol(fields[2], &end, 10);
		if (*end != 0 || cross_time < 1 || cross_time > 99)
			failf("invalid format: train cross time");

		(*trains)[i] = (Train){
			.dir        = tolower(*fields[0]) == 'w' ? WEST : EAST,
			.prio       = islower(*fields[0]) ? LOW : HIGH,
			.load_time  = load_time,
			.cross_time = cross_time,
			.state      = LOADING,
		};
	}

	return ntrains;
}

size_t
parse_events(Event **events, char *raw) {
	size_t nevents = count_lines(raw);
	*events = malloc(nevents * sizeof **events);
	if (!*events) failerrno("malloc");

	for (size_t i = 0; i < nevents; i++) {
		char *line = raw;
		raw = strchr(raw, '\n');
		*raw++ = '\0';

		char *fields[4];
		parse_fields(4, fields, line);

		char *end;
		long time = strtol(fields[0], &end, 10);
		if (*end != 0 || time < 0 || time > UINT32_MAX)
			failf("invalid format: event time");
		long train = strtol(fields[1], &end, 10);
		if (*end != 0 || train < 0 || train > SIZE_MAX)
			failf("invalid format: event train");
		if (strlen(fields[2]) != 1 || strspn(fields[2], "rcf") != 1)
			failf("invalid format: event type");
		if (strlen(fields[3]) != 1 || strspn(fields[3], "we") != 1)
			failf("invalid format: event direction");

		(*events)[i] = (Event){
			.train = train,
			.time  = time,
			.type  = *fields[2] == 'r' ? READY :
				(*fields[2] == 'c' ? CROSSING : FINISHED),
			.dir   = *fields[3] == 'w' ? WEST : EAST,
		};
	}

	return nevents;
}

void
check(size_t ntrains, Train *trains, size_t nevents, Event *events) {
	uint32_t time = 0;

	// occupant_start_time == 0  =>  no train on main track
	// occupant_start_time > 0   =>  occupant started crossing main track at occupant_start_time
	uint32_t occupant_start_time = 0;
	size_t occupant;

	// history == -n, n > 0  =>  last n trains went west
	// history == +n, n > 0  =>  last n trains went east
	// history == 0,         =>  no trains have crossed yet
	int history = 0;

	// We first check that each event is "locally" correct, i.e., that each
	// event does not break any rules in the specification given the current
	// simulation time and state of each train. Then we check that each train
	// successfully crossed the track. We do not check "global" properties,
	// like that the event sequence is optimal in some way, because the
	// specification does not require any such behavior. This leads to some
	// probably unexpected event sequences successfully checking. For instance,
	// given input
	//     w 1 1
	// the expected output is probably
	//     1 0 r w
	//     1 0 c w
	//     2 0 f w
	// but, in fact,
	//     1   0 r w
	//     n   0 c w
	//     n+1 0 f w
	// checks for any n >= 1.

	for (size_t ei = 0; ei < nevents; ei++) {
#define F(f, ...) failf("event %zu: " f, ei,##__VA_ARGS__)

		Event *e = &events[ei];

		size_t ti = e->train;
		if (ti >= ntrains) F("train index out of bounds");
		Train *t = &trains[ti];

		if (e->time < time) F("simulation time decreased");

		if (e->dir != t->dir) F("direction does not match train direction");

		switch (e->type) {
		case READY:
			if (t->state != LOADING) F("train is not loading");

			if (e->time < t->load_time) F("train finished loading early");
			if (e->time > t->load_time) F("train finished loading late");

			t->state = READY;
			break;

		case CROSSING:
			switch (t->state) {
			case LOADING:  F("train can not cross while loading");
			case CROSSING: F("train can not cross while crossing");
			case FINISHED: F("train can not cross twice");
			}

			if (occupant_start_time != 0) F("train can not cross while another train (%zu) is on the main track", occupant);

			for (size_t si = 0; si < ntrains; si++) {
				Train *s = &trains[si];
				if (s->state != READY) continue;

				if (s->dir == WEST && t->dir == EAST) {
					if (history <= -3) continue;
					if (history >= +3) F("train can not cross east while last 3 trains crossed east and a train travelling west (%zu) is ready", si);
				} else if (s->dir == EAST && t->dir == WEST) {
					if (history >= +3) continue;
					if (history <= -3) F("train can not cross west while last 3 trains crossed west and a train travelling east (%zu) is ready", si);
				}

				if (s->prio < t->prio) continue;
				if (s->prio > t->prio) F("train can not cross while a train with higher priority (%zu) is ready and there is no starvation", si);

				if (s->dir == t->dir) {
					if (s->load_time > t->load_time) continue;
					if (s->load_time < t->load_time) F("train can not cross while a train travelling in the same direction with equal priority and lower load time (%zu) is ready", si);
					if (si < ti) F("train can not cross while a train travelling in the same direction with equal priority and equal load time and lower index (%zu) is ready", si);
				} else if (s->dir == WEST /* && t->dir == EAST */) {
					if (history == 0) F("train can not cross east while no trains have crossed yet and a train travelling west with equal priority (%zu) is ready", si);
					if (history > 0) F("train can not cross east while last train crossed east and a train travelling west with equal priority (%zu) is ready", si);
				} else if (s->dir == EAST /* && t->dir == WEST */) {
					if (history < 0) F("train can not cross west while last train crossed west and a train travelling east with equal priority (%zu) is ready", si);
				}
			}

			occupant_start_time = e->time;
			occupant = ti;
			t->state = CROSSING;
			break;

		case FINISHED:
			if (t->state != CROSSING) F("train is not on the main track");

			if (e->time < occupant_start_time + t->cross_time) F("train finished crossing early");
			if (e->time > occupant_start_time + t->cross_time) F("train finished crossing late");

			t->state = FINISHED;
			occupant_start_time = 0;
			if (t->dir == WEST) history = MIN(history, 0) - 1;
			if (t->dir == EAST) history = MAX(history, 0) + 1;
			break;
		}

		time = e->time;

#undef F
	}

	for (size_t ti = 0; ti < ntrains; ti++) {
		switch (trains[ti].state) {
		case LOADING:  failf("train %zu did not finish loading", ti);
		case READY:    failf("train %zu did not start crossing", ti);
		case CROSSING: failf("train %zu did not finish crossing", ti);
		}
	}
}

int
main(int argc, char **argv) {
	if (argc != 3) failf("usage: %s INPUT OUTPUT", argv[0]);

	Train *trains;
	size_t ntrains = parse_trains(&trains, read_text(argv[1]));

	Event *events;
	size_t nevents = parse_events(&events, read_text(argv[2]));

	check(ntrains, trains, nevents, events);
}
