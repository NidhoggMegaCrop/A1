// COMP2521 - Assignment 1
// Tests for the Course Catalogue ADT

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Catalogue.h"
#include "CatalogueStructs.h"
#include "course.h"

static void testBasicOperations(void);
static void checkPrint(Catalogue catalogue, char *expectedPrint);
static void checkFileContents(FILE *file, char *expectedContents);

static void testClosest(void);
static void testLevelOrder(void);
static void testGetRange(void);

static void testBalance(void);
static void testBalance1(void);
static void testBalance2(void);
static bool isHeightBalanced(struct node *t);
static bool doIsHeightBalanced(struct node *t, int *height);

static void testIndexingOperations(void);

static void testConstruct(void);

int main(void) {
	testBasicOperations();

	testClosest();
	testLevelOrder();
	testGetRange();

	testBalance();

	testIndexingOperations();

	testConstruct();

	printf("All basic tests passed!\n");
}

////////////////////////////////////////////////////////////////////////
// Part 1

static void testBasicOperations(void) {
	Catalogue catalogue = CatalogueNew();

	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 3121, "Algorithm Design and Analysis", 6);

	assert(CatalogueNumCourses(catalogue) == 3);
	assert(CatalogueFind(catalogue, 1511).code == 1511);
	assert(CatalogueFind(catalogue, 9999).code == COURSE_UNDEFINED);

	CatalogueDelete(catalogue, 1511);
	assert(CatalogueNumCourses(catalogue) == 2);
	assert(CatalogueFind(catalogue, 1511).code == COURSE_UNDEFINED);

	checkPrint(
		catalogue,
		"{"
		"(2521, Data Structures and Algorithms, 6), "
		"(3121, Algorithm Design and Analysis, 6)"
		"}"
	);

	CatalogueFree(catalogue);
}

static void checkPrint(Catalogue catalogue, char *expectedPrint) {
	FILE *out = tmpfile();
	CataloguePrint(catalogue, out);
	checkFileContents(out, expectedPrint);
	fclose(out);
}

/**
 * This function checks if the contents of a file matches the expected
 * contents. If the contents do not match, the program will exit with an
 * assertion error.
 */
static void checkFileContents(FILE *file, char *expectedContents) {
	fflush(file);
	fseek(file, 0, SEEK_SET);
	char *line = NULL;
	size_t n = 0;
	getline(&line, &n, file);
	if (strcmp(line, expectedContents) != 0) {
		printf("Test failed for CataloguePrint: expected \"%s\", "
		       "saw \"%s\"\n", expectedContents, line);

		assert(strcmp(line, expectedContents) == 0);
	}
	free(line);
}

////////////////////////////////////////////////////////////////////////
// Part 2

static void testClosest(void) {
	Catalogue catalogue = CatalogueNew();
	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 3121, "Algorithm Design and Analysis", 6);

	assert(CatalogueClosest(catalogue, 2000).code == 1511);
	assert(CatalogueClosest(catalogue, 2500).code == 2521);
	CatalogueFree(catalogue);
}

static void testLevelOrder(void) {
	Catalogue catalogue = CatalogueNew();

	CatalogueInsert(catalogue, 1521, "Computer Systems Fundamentals", 6);
	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 3121, "Algorithm Design and Analysis", 6);
	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 3311, "Database Systems", 6);

	struct course courses[5];
	int numCourses = CatalogueLevelOrder(catalogue, courses);
	assert(numCourses == 5);
	assert(courses[0].code == 1521);
	assert(courses[1].code == 1511);
	assert(courses[2].code == 3121);
	assert(courses[3].code == 2521);
	assert(courses[4].code == 3311);

	CatalogueFree(catalogue);
}

static void testGetRange(void) {
	Catalogue catalogue = CatalogueNew();
	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 1521, "Computer Systems Fundamentals", 6);
	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 3121, "Algorithm Design and Analysis", 6);

	struct course result[4];
	int numCourses = CatalogueGetRange(catalogue, 1500, 2600, result);
	assert(numCourses == 3);
	assert(result[0].code == 1511);
	assert(result[1].code == 1521);
	assert(result[2].code == 2521);
	CatalogueFree(catalogue);
}

////////////////////////////////////////////////////////////////////////
// Part 3

static void testBalance(void) {
	testBalance1();
	testBalance2();
}

static void testBalance1(void) {
	Catalogue catalogue = CatalogueNew();

	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 1521, "Computer Systems Fundamentals", 6);
	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);

	// The tree should have been rebalanced after inserting 1511
	// NOTE: Normally, a user should not have access to the concrete
	//       representation of an ADT, but since we have #included
	//       CatalogueStructs.h, we have access for testing purposes.
	assert(isHeightBalanced(catalogue->tree));

	CatalogueFree(catalogue);
}

static void testBalance2(void) {
	Catalogue catalogue = CatalogueNew();

	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 1521, "Computer Systems Fundamentals", 6);
	CatalogueInsert(catalogue, 3121, "Algorithm Design and Analysis", 6);
	CatalogueInsert(catalogue, 3311, "Database Systems", 6);
	CatalogueDelete(catalogue, 1521);

	// The tree should have been rebalanced after deleting 1521
	assert(isHeightBalanced(catalogue->tree));

	CatalogueFree(catalogue);
}

static bool isHeightBalanced(struct node *t) {
	int height = -1;
	return doIsHeightBalanced(t, &height);
}

static bool doIsHeightBalanced(struct node *t, int *height) {
	if (t == NULL) {
		*height = -1;
		return true;
	}

	int lh = -1;
	int rh = -1;
	if (!doIsHeightBalanced(t->left, &lh) || !doIsHeightBalanced(t->right, &rh)) {
		return false;
	}
	if (abs(lh - rh) > 1) return false;
	*height = (lh > rh ? lh : rh) + 1;
	return true;
}

////////////////////////////////////////////////////////////////////////
// Part 4

static void testIndexingOperations(void) {
	Catalogue catalogue = CatalogueNew();

	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 3121, "Algorithm Design and Analysis", 6);
	CatalogueInsert(catalogue, 1521, "Computer Systems Fundamentals", 6);

	assert(CatalogueAtIndex(catalogue, 0).code == 1511);
	assert(CatalogueAtIndex(catalogue, 2).code == 2521);
	assert(CatalogueIndexOf(catalogue, 3121) == 3);
	assert(CatalogueCountLower(catalogue, 2500) == 2);

	CatalogueFree(catalogue);
}

////////////////////////////////////////////////////////////////////////
// Part 5

static void testConstruct(void) {
	struct course courses[] = {
		{1511, "Programming Fundamentals", 6},
		{1521, "Computer Systems Fundamentals", 6},
		{2521, "Data Structures and Algorithms", 6},
		{3121, "Algorithm Design and Analysis", 6},
		{3311, "Database Systems", 6},
	};

	Catalogue catalogue = CatalogueConstruct(courses, 5);
	assert(CatalogueNumCourses(catalogue) == 5);
	assert(isHeightBalanced(catalogue->tree));

	CatalogueFree(catalogue);
}

////////////////////////////////////////////////////////////////////////
