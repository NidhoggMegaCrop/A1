// COMP2521 - Assignment 1
// Concrete representation of the Course Catalogue ADT

// IMPORTANT: Only struct definitions should be placed in this file.
//            All other code (e.g., function prototypes) should be
//            placed in Catalogue.c.

#ifndef CATALOGUE_STRUCTS_H
#define CATALOGUE_STRUCTS_H

#include "course.h"

// DO NOT CHANGE THE NAME OF THIS STRUCT
struct catalogue {
	struct node *tree;  // DO NOT MODIFY/REMOVE THIS FIELD

	int numCourses;
};

// DO NOT CHANGE THE NAME OF THIS STRUCT
struct node {
	struct course course; // DO NOT MODIFY/REMOVE THIS FIELD
	struct node *left;    // DO NOT MODIFY/REMOVE THIS FIELD
	struct node *right;   // DO NOT MODIFY/REMOVE THIS FIELD

	int height; // height of this subtree, used to keep the tree balanced
};

// You may define additional structs here if needed

#endif

