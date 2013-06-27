/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

#include "test.h"


char details[DETAIL_LENGTH] = "\0";
int passed = 0;
int tests = 0;


void announce(const char *text) {
	printf("Testing %s ... ", text);
}

void evaluate(int result) {
	if (result) {
		printf("OK\n");
		passed++;
	} else {
		printf("Failed\n");
		if (strlen(details)) {
			printf("%s", details);
		}
	}
	tests++;
}

int equal_int(int result, int expected) {
	if (expected == result) {
		return 1;
	} else {
		snprintf(details, sizeof details,
				"\tExpected : %d\n\tActual   : %d\n",
				expected, result);
		return 0;
	}
}

int equal_str(const char *result, const char *expected) {
	if (strcmp(expected, result) == 0) {
		return 1;
	} else {
		snprintf(details, sizeof details,
				"\tExpected : %s\n\tActual   : %s\n",
				expected, result);
		return 0;
	}
}

int test_result() {
	printf("\n%d tests, %d passed, %d failed\n",
	       tests, passed, tests - passed);
	return passed == tests;
}

/* EOF */
