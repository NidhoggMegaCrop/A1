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

// My own additional tests
static void myTestEmptyCatalogue(void);
static void myTestDuplicateInsert(void);
static void myTestDeleteNonExistent(void);
static void myTestDeleteAllNodeShapes(void);
static void myTestClosestEdgeCases(void);
static void myTestLevelOrderSingleNode(void);
static void myTestGetRangeEdgeCases(void);

int main(void) {
	testBasicOperations();
	myTestEmptyCatalogue();
	myTestDuplicateInsert();
	myTestDeleteNonExistent();
	myTestDeleteAllNodeShapes();

	testClosest();
	myTestClosestEdgeCases();
	testLevelOrder();
	myTestLevelOrderSingleNode();
	testGetRange();
	myTestGetRangeEdgeCases();

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

// My own test: a freshly created catalogue has no courses, CatalogueFind
// returns COURSE_UNDEFINED for any code, and CataloguePrint shows "{}".
static void myTestEmptyCatalogue(void) {
	Catalogue catalogue = CatalogueNew();

	assert(CatalogueNumCourses(catalogue) == 0);
	assert(CatalogueFind(catalogue, 1511).code == COURSE_UNDEFINED);
	checkPrint(catalogue, "{}");

	CatalogueFree(catalogue);
}

// My own test: inserting a course whose code already exists should do
// nothing - the original name/creditPoints are kept and the course count
// does not increase.
static void myTestDuplicateInsert(void) {
	Catalogue catalogue = CatalogueNew();

	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 1511, "Some Other Name", 10);

	assert(CatalogueNumCourses(catalogue) == 1);
	struct course c = CatalogueFind(catalogue, 1511);
	assert(strcmp(c.name, "Programming Fundamentals") == 0);
	assert(c.creditPoints == 6);

	CatalogueFree(catalogue);
}

// My own test: deleting a code that doesn't exist in a non-empty
// catalogue should be a no-op (course count and contents are unchanged).
static void myTestDeleteNonExistent(void) {
	Catalogue catalogue = CatalogueNew();

	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);

	CatalogueDelete(catalogue, 9999);

	assert(CatalogueNumCourses(catalogue) == 2);
	assert(CatalogueFind(catalogue, 1511).code == 1511);
	assert(CatalogueFind(catalogue, 2521).code == 2521);

	CatalogueFree(catalogue);
}

// My own test: deleting from a catalogue exercises all three deletion
// cases - a leaf node, a node with only one child, and a node with two
// children (which requires the in-order successor to be spliced in).
static void myTestDeleteAllNodeShapes(void) {
	Catalogue catalogue = CatalogueNew();

	// Build a tree shaped like:
	//            2521
	//          /      \
	//       1521       3121
	//      /    \          \
	//   1511   1531        3311
	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 1521, "Computer Systems Fundamentals", 6);
	CatalogueInsert(catalogue, 3121, "Algorithm Design and Analysis", 6);
	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 1531, "Web Front-End Programming", 6);
	CatalogueInsert(catalogue, 3311, "Database Systems", 6);

	// Delete a leaf node (1511)
	CatalogueDelete(catalogue, 1511);
	assert(CatalogueFind(catalogue, 1511).code == COURSE_UNDEFINED);
	assert(CatalogueNumCourses(catalogue) == 5);

	// Delete a node with only one child (3121, whose only child is 3311)
	CatalogueDelete(catalogue, 3121);
	assert(CatalogueFind(catalogue, 3121).code == COURSE_UNDEFINED);
	assert(CatalogueFind(catalogue, 3311).code == 3311);
	assert(CatalogueNumCourses(catalogue) == 4);

	// Delete a node with two children (1521, whose children are now
	// just 1531 on the right)
	CatalogueDelete(catalogue, 1521);
	assert(CatalogueFind(catalogue, 1521).code == COURSE_UNDEFINED);
	assert(CatalogueFind(catalogue, 1531).code == 1531);
	assert(CatalogueNumCourses(catalogue) == 3);

	// Delete the root (2521), which has two children (1531 and 3311)
	CatalogueDelete(catalogue, 2521);
	assert(CatalogueFind(catalogue, 2521).code == COURSE_UNDEFINED);
	assert(CatalogueNumCourses(catalogue) == 2);
	checkPrint(
		catalogue,
		"{(1531, Web Front-End Programming, 6), (3311, Database Systems, 6)}"
	);

	CatalogueFree(catalogue);
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

// My own test: CatalogueClosest on edge cases - an exact match, a target
// below the smallest code, a target above the largest code, and a target
// that is exactly equidistant from two codes (should return the smaller).
static void myTestClosestEdgeCases(void) {
	Catalogue catalogue = CatalogueNew();
	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 1521, "Computer Systems Fundamentals", 6);
	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 3121, "Algorithm Design and Analysis", 6);

	// Exact match
	assert(CatalogueClosest(catalogue, 2521).code == 2521);
	// Below the smallest code
	assert(CatalogueClosest(catalogue, 0).code == 1511);
	// Above the largest code
	assert(CatalogueClosest(catalogue, 9999).code == 3121);
	// Tie: 1511 and 1521 are both 5 away from 1516; expect the smaller
	assert(CatalogueClosest(catalogue, 1516).code == 1511);

	// Single-course catalogue
	Catalogue single = CatalogueNew();
	CatalogueInsert(single, 2521, "Data Structures and Algorithms", 6);
	assert(CatalogueClosest(single, 1000).code == 2521);
	assert(CatalogueClosest(single, 5000).code == 2521);

	CatalogueFree(catalogue);
	CatalogueFree(single);
}

// My own test: CatalogueLevelOrder on a single-node catalogue should
// return 1 and store just that course.
static void myTestLevelOrderSingleNode(void) {
	Catalogue catalogue = CatalogueNew();
	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);

	struct course courses[1];
	int numCourses = CatalogueLevelOrder(catalogue, courses);
	assert(numCourses == 1);
	assert(courses[0].code == 2521);

	CatalogueFree(catalogue);
}

// My own test: CatalogueGetRange edge cases - range boundaries exactly
// matching existing codes, a range containing no courses, and a range
// covering the whole catalogue.
static void myTestGetRangeEdgeCases(void) {
	Catalogue catalogue = CatalogueNew();
	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 1521, "Computer Systems Fundamentals", 6);
	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);
	CatalogueInsert(catalogue, 3121, "Algorithm Design and Analysis", 6);

	struct course result[4];

	// Range boundaries exactly on existing codes (inclusive)
	int numCourses = CatalogueGetRange(catalogue, 1511, 2521, result);
	assert(numCourses == 3);
	assert(result[0].code == 1511);
	assert(result[1].code == 1521);
	assert(result[2].code == 2521);

	// Range containing no courses
	numCourses = CatalogueGetRange(catalogue, 1600, 1700, result);
	assert(numCourses == 0);

	// Range covering the whole catalogue
	numCourses = CatalogueGetRange(catalogue, 0, 9999, result);
	assert(numCourses == 4);
	assert(result[3].code == 3121);

	// lowerCode == upperCode, matching an existing code
	numCourses = CatalogueGetRange(catalogue, 2521, 2521, result);
	assert(numCourses == 1);
	assert(result[0].code == 2521);

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
