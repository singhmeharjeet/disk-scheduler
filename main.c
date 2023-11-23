#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#define FIRST_TRACK 0
#define LAST_TRACK 199

typedef struct Track {
	int value;
	int entry;
} Track;

FILE *output;
int sstf_head = 0;
int scan_head = 0;

bool C_SCAN = false;
int rewind_distance = 0;

void parse_command_line(int argc, char **argv, Track **tracks, int *num_tracks);

int sstf_compare(const void *a, const void *b);

int scan_compare_asc(const void *a, const void *b);

int scan_compare_dec(const void *a, const void *b);


// HELPER FUNCTIONS
int get_random_number(int start, int end);

void out_tracks(Track *tracks, int num_tracks);

void out(const char *format, ...);

int main(int argc, char **argv) {
	output = fopen("out.txt", "w");
	int num_tracks = 0;
	Track *fcfs_tracks = NULL;
	parse_command_line(argc, argv, &fcfs_tracks, &num_tracks);

	// Display the Input
	out("Requests:          \t");
	for (int i = 0; i < num_tracks; ++i) {
		out("%03d ", fcfs_tracks[i].value);
	}
	out("\nNumber of Tracks:\t%d\n", num_tracks);
	out("Head Position:     \t%d\n", fcfs_tracks[0].value);
	out("\n");

	// Shortest Seek Time First
	Track *sstf_tracks = (Track *) malloc(sizeof(Track) * num_tracks);
	memcpy(sstf_tracks, fcfs_tracks, sizeof(Track) * num_tracks);
	sstf_head = sstf_tracks[0].value;
	for (int i = 1; i < num_tracks; ++i) { // Really inefficient, but it works O(n* nlog(n))
		qsort(sstf_tracks + i, num_tracks - i, sizeof(Track), sstf_compare);
		sstf_head = sstf_tracks[i].value;
	}
	out("Shortest Seek Time First\n");
	out_tracks(sstf_tracks, num_tracks);


	C_SCAN = true;
	//C-SCAN
	Track *scan_tracks = (Track *) malloc(sizeof(Track) * num_tracks);
	memcpy(scan_tracks, fcfs_tracks, sizeof(Track) * num_tracks);
	scan_head = scan_tracks[0].value;

	// Can be implemented better with a partition algorithm,

	qsort(scan_tracks, num_tracks, sizeof(Track), scan_compare_asc);
	Track *last = &scan_tracks[num_tracks - 1];
	rewind_distance = (199 - last->value);
	Track *head = bsearch(&scan_head, scan_tracks, num_tracks, sizeof(Track), scan_compare_asc);
	// Head can't be null
	// Rotate the array so that the head is at the beginning
	Track *temp = (Track *) malloc(sizeof(Track) * num_tracks);
	memcpy(temp, scan_tracks, sizeof(Track) * num_tracks);
	memcpy(scan_tracks, head, sizeof(Track) * (num_tracks - (head - scan_tracks)));
	memcpy(scan_tracks + (num_tracks - (head - scan_tracks)), temp, sizeof(Track) * (head - scan_tracks));

	// Sort in opposite direction after the partition
	qsort(scan_tracks + (num_tracks - (head - scan_tracks)), (num_tracks - (head - scan_tracks)), sizeof(Track),
		  scan_compare_dec);
	free(temp);

	out("SCAN\n");
	out_tracks(scan_tracks, num_tracks);


	free(fcfs_tracks); // Initialized in parse_command_line
	free(sstf_tracks);
	free(scan_tracks);

	return 0;
}

void parse_command_line(int argc, char **argv, Track **tracks, int *num_tracks) {

	if (argc == 1) {
		*num_tracks = get_random_number(50, 100);
		*tracks = (Track *) malloc(sizeof(Track) * *num_tracks);
		for (int i = 0; i < *num_tracks; i++) {
			(*tracks)[i].value = get_random_number(FIRST_TRACK, LAST_TRACK);
			(*tracks)[i].entry = i;
		}
		return;
	} else if (argc == 2) {

		int size = 1;
		*tracks = (Track *) malloc(sizeof(Track) * size);
		*num_tracks = 0;
		while (true) {

			if (*num_tracks == size) {
				size = size << 1;
				*tracks = (Track *) realloc(*tracks, sizeof(Track) * size);
			}

			char *token = strsep(&argv[1], ",");
			if (token == NULL) {
				break;
			}

			const char *parse_error = NULL;
			int track = (int) strtonum(token, FIRST_TRACK, LAST_TRACK, &parse_error);

			if (parse_error) {
				free(*tracks);
				printf("Error: %s input.\n", parse_error);
				exit(1);
			}


			(*tracks)[*num_tracks].value = track;
			(*tracks)[*num_tracks].entry = *num_tracks;
			*num_tracks += 1;
		}
		if (*num_tracks < 3) {
			free(*tracks);
			printf("Error: At least 3 tracks are required.\n");
			exit(1);
		}

		// Resize the array to the correct size
		*tracks = (Track *) realloc(*tracks, sizeof(Track) * (*num_tracks));
		return;
	}

	printf("Usage: ./DSSimul <track1>,<track2>,...,<trackN>\n");
	exit(1);
}

int sstf_compare(const void *a, const void *b) {
	Track *track_a = (Track *) a;
	Track *track_b = (Track *) b;
	int a_diff = abs(track_a->value - sstf_head);
	int b_diff = abs(track_b->value - sstf_head);
	if (a_diff == b_diff) {
		return track_a->value - track_b->value;
	}

	return a_diff - b_diff;
}

int scan_compare_asc(const void *a, const void *b) {
	return ((Track *) a)->value - ((Track *) b)->value;
}

int scan_compare_dec(const void *a, const void *b) {
	return ((Track *) b)->value - ((Track *) a)->value;
}

int get_random_number(int start, int end) {
	return (int) (arc4random() % (end - start + 1) + start);
}

void out(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(output, format, args);
	va_end(args);
}

void out_tracks(Track *tracks, int num_tracks) {
	int distance = 0;
	out("Seek Sequence:    \t");
	for (int i = 0; i < num_tracks; ++i) {
		out("%03d ", tracks[i].value);
		if (i > 0) {
			distance += abs(tracks[i].value - tracks[i - 1].value);
		}
	}
	out("\n");
	out("Enter At:         \t");
	for (int i = 0; i < num_tracks; ++i) {
		out("%03d ", tracks[i].entry);
	}
	out("\n");
	out("Delays:           \t");
	int avg_delay = 0;
	int max_delay = 0;
	int delayed_tracks = 0;
	for (int i = 0; i < num_tracks; ++i) {
		if (i <= tracks[i].entry) {
			out("--- ");
			continue;
		}

		int delay = i - tracks[i].entry;
		if (delay > max_delay) {
			max_delay = delay;
		}
		avg_delay += delay;
		delayed_tracks++;
		out("%03d ", delay);
	}
	out("\n");
	out("Longest Delay: %d\n", max_delay);
	out("Average Delay: %03.2f\n", (float) avg_delay / (float) delayed_tracks);
	if (C_SCAN) {
		printf("Rewind Distance: %d\n", rewind_distance);
		out("Average Seek Distance: %03.2f\n", ((float) distance + (float) rewind_distance) / (float) (num_tracks));
	} else {
		out("Average Seek Distance: %03.2f\n", (float) distance / (float) (num_tracks));
	}

	out("\n");
}