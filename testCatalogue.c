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
static void myTestBalanceInsertRR(void);
static void myTestBalanceInsertLL(void);
static void myTestBalanceInsertLRandRL(void);
static void myTestBalanceDeleteMany(void);
static void myTestIndexingEmptyAndOutOfRange(void);
static void myTestIndexingLargerCatalogue(void);
static void myTestConstructEmpty(void);
static void myTestConstructSingle(void);
static void myTestConstructLargerAndFullyFunctional(void);

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
	myTestBalanceInsertRR();
	myTestBalanceInsertLL();
	myTestBalanceInsertLRandRL();
	myTestBalanceDeleteMany();

	testIndexingOperations();
	myTestIndexingEmptyAndOutOfRange();
	myTestIndexingLargerCatalogue();

	testConstruct();
	myTestConstructEmpty();
	myTestConstructSingle();
	myTestConstructLargerAndFullyFunctional();

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

// My own test: inserting in strictly increasing order forces a
// left rotation (RR case) at every step, and the codes must still be
// retrievable in the correct order afterwards.
static void myTestBalanceInsertRR(void) {
	Catalogue catalogue = CatalogueNew();
	for (int code = 1000; code < 1000 + 20; code++) {
		char name[MAX_COURSE_NAME];
		snprintf(name, sizeof(name), "Course %d", code);
		CatalogueInsert(catalogue, code, name, 6);
		assert(isHeightBalanced(catalogue->tree));
	}
	assert(CatalogueNumCourses(catalogue) == 20);
	for (int code = 1000; code < 1000 + 20; code++) {
		assert(CatalogueFind(catalogue, code).code == code);
	}
	CatalogueFree(catalogue);
}

// My own test: inserting in strictly decreasing order forces a
// right rotation (LL case) at every step.
static void myTestBalanceInsertLL(void) {
	Catalogue catalogue = CatalogueNew();
	for (int code = 1000 + 20; code > 1000; code--) {
		char name[MAX_COURSE_NAME];
		snprintf(name, sizeof(name), "Course %d", code);
		CatalogueInsert(catalogue, code, name, 6);
		assert(isHeightBalanced(catalogue->tree));
	}
	assert(CatalogueNumCourses(catalogue) == 20);
	CatalogueFree(catalogue);
}

// My own test: inserting 30, 10, 20 forces a left-right (LR) double
// rotation, and inserting 10, 30, 20 forces a right-left (RL) double
// rotation. Check the tree stays balanced and every code is findable.
static void myTestBalanceInsertLRandRL(void) {
	Catalogue lr = CatalogueNew();
	CatalogueInsert(lr, 30, "C30", 6);
	CatalogueInsert(lr, 10, "C10", 6);
	CatalogueInsert(lr, 20, "C20", 6);
	assert(isHeightBalanced(lr->tree));
	assert(CatalogueFind(lr, 10).code == 10);
	assert(CatalogueFind(lr, 20).code == 20);
	assert(CatalogueFind(lr, 30).code == 30);
	CatalogueFree(lr);

	Catalogue rl = CatalogueNew();
	CatalogueInsert(rl, 10, "C10", 6);
	CatalogueInsert(rl, 30, "C30", 6);
	CatalogueInsert(rl, 20, "C20", 6);
	assert(isHeightBalanced(rl->tree));
	assert(CatalogueFind(rl, 10).code == 10);
	assert(CatalogueFind(rl, 20).code == 20);
	assert(CatalogueFind(rl, 30).code == 30);
	CatalogueFree(rl);
}

// My own test: repeatedly deleting from a larger balanced catalogue
// (in an order that forces multiple rebalances) must keep the tree
// balanced at every step and never lose an undeleted course.
static void myTestBalanceDeleteMany(void) {
	Catalogue catalogue = CatalogueNew();
	int codes[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45,
	               55, 65, 75, 85};
	int n = sizeof(codes) / sizeof(codes[0]);
	for (int i = 0; i < n; i++) {
		char name[MAX_COURSE_NAME];
		snprintf(name, sizeof(name), "Course %d", codes[i]);
		CatalogueInsert(catalogue, codes[i], name, 6);
	}
	assert(isHeightBalanced(catalogue->tree));

	// Delete every other course, checking balance after each deletion
	for (int i = 0; i < n; i += 2) {
		CatalogueDelete(catalogue, codes[i]);
		assert(isHeightBalanced(catalogue->tree));
		assert(CatalogueFind(catalogue, codes[i]).code == COURSE_UNDEFINED);
	}
	// The untouched courses must still all be present
	for (int i = 1; i < n; i += 2) {
		assert(CatalogueFind(catalogue, codes[i]).code == codes[i]);
	}

	CatalogueFree(catalogue);
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

// My own test: indexing operations on an empty catalogue and on
// out-of-range indices should not find anything.
static void myTestIndexingEmptyAndOutOfRange(void) {
	Catalogue catalogue = CatalogueNew();

	assert(CatalogueAtIndex(catalogue, 0).code == COURSE_UNDEFINED);
	assert(CatalogueIndexOf(catalogue, 1511) == -1);
	assert(CatalogueCountLower(catalogue, 1511) == 0);

	CatalogueInsert(catalogue, 1511, "Programming Fundamentals", 6);
	CatalogueInsert(catalogue, 2521, "Data Structures and Algorithms", 6);

	// Negative index and index >= numCourses are both out of range
	assert(CatalogueAtIndex(catalogue, -1).code == COURSE_UNDEFINED);
	assert(CatalogueAtIndex(catalogue, 2).code == COURSE_UNDEFINED);
	// A code that doesn't exist has no index
	assert(CatalogueIndexOf(catalogue, 9999) == -1);

	CatalogueFree(catalogue);
}

// My own test: indexing operations on a larger catalogue - every index
// must map back to the correct code (CatalogueAtIndex and
// CatalogueIndexOf must be inverses of each other), and
// CatalogueCountLower must agree with the index for existing codes and
// work correctly for codes that fall in between or outside the range.
static void myTestIndexingLargerCatalogue(void) {
	Catalogue catalogue = CatalogueNew();
	int codes[] = {1511, 1521, 1531, 2521, 3121, 3311, 3331, 3821, 4141};
	int n = sizeof(codes) / sizeof(codes[0]);
	for (int i = 0; i < n; i++) {
		char name[MAX_COURSE_NAME];
		snprintf(name, sizeof(name), "Course %d", codes[i]);
		CatalogueInsert(catalogue, codes[i], name, 6);
	}

	for (int i = 0; i < n; i++) {
		assert(CatalogueAtIndex(catalogue, i).code == codes[i]);
		assert(CatalogueIndexOf(catalogue, codes[i]) == i);
		assert(CatalogueCountLower(catalogue, codes[i]) == i);
	}

	// A code strictly between two existing codes
	assert(CatalogueCountLower(catalogue, 3200) == 5);
	// Below the smallest code
	assert(CatalogueCountLower(catalogue, 0) == 0);
	// Above the largest code
	assert(CatalogueCountLower(catalogue, 9999) == n);

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

// My own test: CatalogueConstruct on an empty array should produce a
// valid, empty catalogue.
static void myTestConstructEmpty(void) {
	Catalogue catalogue = CatalogueConstruct(NULL, 0);
	assert(CatalogueNumCourses(catalogue) == 0);
	checkPrint(catalogue, "{}");
	CatalogueFree(catalogue);
}

// My own test: CatalogueConstruct on a single-course array.
static void myTestConstructSingle(void) {
	struct course courses[] = {
		{2521, "Data Structures and Algorithms", 6},
	};
	Catalogue catalogue = CatalogueConstruct(courses, 1);
	assert(CatalogueNumCourses(catalogue) == 1);
	assert(isHeightBalanced(catalogue->tree));
	assert(CatalogueFind(catalogue, 2521).code == 2521);
	CatalogueFree(catalogue);
}

// My own test: CatalogueConstruct on a larger array must not modify the
// input array, must produce a height-balanced tree, and every
// operation from earlier parts must work correctly on the result
// (find, closest, level order, get range, indexing, insert, delete).
static void myTestConstructLargerAndFullyFunctional(void) {
	struct course courses[] = {
		{1511, "Programming Fundamentals", 6},
		{1521, "Computer Systems Fundamentals", 6},
		{1531, "Web Front-End Programming", 6},
		{2521, "Data Structures and Algorithms", 6},
		{3121, "Algorithm Design and Analysis", 6},
		{3311, "Database Systems", 6},
		{3331, "Applied Cryptography", 6},
		{3821, "Cloud Computing and Big Data Systems", 6},
	};
	int n = sizeof(courses) / sizeof(courses[0]);
	struct course coursesCopy[8];
	memcpy(coursesCopy, courses, sizeof(courses));

	Catalogue catalogue = CatalogueConstruct(courses, n);

	// Input array must not be modified
	for (int i = 0; i < n; i++) {
		assert(courses[i].code == coursesCopy[i].code);
		assert(strcmp(courses[i].name, coursesCopy[i].name) == 0);
		assert(courses[i].creditPoints == coursesCopy[i].creditPoints);
	}

	assert(CatalogueNumCourses(catalogue) == n);
	assert(isHeightBalanced(catalogue->tree));

	// CatalogueFind / CatalogueAtIndex / CatalogueIndexOf agree with
	// the original sorted order
	for (int i = 0; i < n; i++) {
		assert(CatalogueFind(catalogue, courses[i].code).code == courses[i].code);
		assert(CatalogueAtIndex(catalogue, i).code == courses[i].code);
		assert(CatalogueIndexOf(catalogue, courses[i].code) == i);
	}

	// CatalogueClosest
	assert(CatalogueClosest(catalogue, 3800).code == 3821);

	// CatalogueGetRange
	struct course result[8];
	int numCourses = CatalogueGetRange(catalogue, 1500, 2600, result);
	assert(numCourses == 4);
	assert(result[3].code == 2521);

	// Insertion and deletion must keep the tree balanced afterwards
	CatalogueInsert(catalogue, 4141, "Ethical Hacking", 6);
	assert(isHeightBalanced(catalogue->tree));
	assert(CatalogueNumCourses(catalogue) == n + 1);

	CatalogueDelete(catalogue, 1511);
	assert(isHeightBalanced(catalogue->tree));
	assert(CatalogueFind(catalogue, 1511).code == COURSE_UNDEFINED);
	assert(CatalogueNumCourses(catalogue) == n);

	CatalogueFree(catalogue);
}

////////////////////////////////////////////////////////////////////////
