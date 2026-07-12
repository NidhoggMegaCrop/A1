// COMP2521 - Assignment 1
// Definition of the course data type

// !!! DO NOT MODIFY THIS FILE !!!

#ifndef COURSE_H
#define COURSE_H

#define MAX_COURSE_NAME 64

struct course {
	int code;
	char name[MAX_COURSE_NAME];
	int creditPoints;
};

#endif

