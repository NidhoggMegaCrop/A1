// COMP2521 - Assignment 1
// Implementation of the Course Catalogue ADT

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Catalogue.h"
#include "CatalogueStructs.h"
#include "course.h"

////////////////////////////////////////////////////////////////////////
// Basic Operations

static struct node *newNode(int code, char *name, int creditPoints) {
	struct node *n = malloc(sizeof(*n));
	if (n == NULL) {
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}
	n->course.code = code;
	strcpy(n->course.name, name);
	n->course.creditPoints = creditPoints;
	n->left = NULL;
	n->right = NULL;
	n->height = 0;
	n->size = 1;
	return n;
}

static void freeTree(struct node *n) {
	if (n == NULL) return;
	freeTree(n->left);
	freeTree(n->right);
	free(n);
}

////////////////////////////////////////////////////////////////////////
// AVL Balancing Helpers

static int height(struct node *n) {
	return n == NULL ? -1 : n->height;
}

static int size(struct node *n) {
	return n == NULL ? 0 : n->size;
}

// Recomputes n's height and size from its (already up-to-date) children.
static void updateNode(struct node *n) {
	int lh = height(n->left);
	int rh = height(n->right);
	n->height = 1 + (lh > rh ? lh : rh);
	n->size = 1 + size(n->left) + size(n->right);
}

static int balanceFactor(struct node *n) {
	return height(n->left) - height(n->right);
}

static struct node *rotateRight(struct node *y) {
	struct node *x = y->left;
	y->left = x->right;
	x->right = y;
	updateNode(y);
	updateNode(x);
	return x;
}

static struct node *rotateLeft(struct node *x) {
	struct node *y = x->right;
	x->right = y->left;
	y->left = x;
	updateNode(x);
	updateNode(y);
	return y;
}

// Restores the height-balance property of n assuming both of its
// subtrees are already height-balanced, and updates n's height and size.
static struct node *rebalance(struct node *n) {
	updateNode(n);
	int bf = balanceFactor(n);
	if (bf > 1) {
		if (balanceFactor(n->left) < 0) {
			n->left = rotateLeft(n->left);
		}
		return rotateRight(n);
	} else if (bf < -1) {
		if (balanceFactor(n->right) > 0) {
			n->right = rotateRight(n->right);
		}
		return rotateLeft(n);
	}
	return n;
}

////////////////////////////////////////////////////////////////////////

static struct node *insertNode(struct node *n, int code, char *name,
                                int creditPoints, bool *inserted) {
	if (n == NULL) {
		*inserted = true;
		return newNode(code, name, creditPoints);
	}
	if (code < n->course.code) {
		n->left = insertNode(n->left, code, name, creditPoints, inserted);
	} else if (code > n->course.code) {
		n->right = insertNode(n->right, code, name, creditPoints, inserted);
	} else {
		return n;
	}
	return rebalance(n);
}

static struct node *minNode(struct node *n) {
	while (n->left != NULL) n = n->left;
	return n;
}

static struct node *deleteNode(struct node *n, int code, bool *deleted) {
	if (n == NULL) return NULL;
	if (code < n->course.code) {
		n->left = deleteNode(n->left, code, deleted);
	} else if (code > n->course.code) {
		n->right = deleteNode(n->right, code, deleted);
	} else {
		*deleted = true;
		if (n->left == NULL) {
			struct node *right = n->right;
			free(n);
			return right;
		} else if (n->right == NULL) {
			struct node *left = n->left;
			free(n);
			return left;
		} else {
			struct node *succ = minNode(n->right);
			n->course = succ->course;
			bool discard = false;
			n->right = deleteNode(n->right, succ->course.code, &discard);
		}
	}
	return rebalance(n);
}

static struct node *findNode(struct node *n, int code) {
	if (n == NULL) return NULL;
	if (code < n->course.code) return findNode(n->left, code);
	if (code > n->course.code) return findNode(n->right, code);
	return n;
}

static void printTree(struct node *n, FILE *out, bool *first) {
	if (n == NULL) return;
	printTree(n->left, out, first);
	if (*first) {
		*first = false;
	} else {
		fprintf(out, ", ");
	}
	fprintf(out, "(%d, %s, %d)", n->course.code, n->course.name,
	        n->course.creditPoints);
	printTree(n->right, out, first);
}

Catalogue CatalogueNew(void) {
	Catalogue catalogue = malloc(sizeof(*catalogue));
	if (catalogue == NULL) {
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}
	catalogue->tree = NULL;
	catalogue->numCourses = 0;
	return catalogue;
}

void CatalogueFree(Catalogue catalogue) {
	freeTree(catalogue->tree);
	free(catalogue);
}

void CatalogueInsert(Catalogue catalogue, int code, char *name,
                     int creditPoints) {
	bool inserted = false;
	catalogue->tree = insertNode(catalogue->tree, code, name, creditPoints,
	                              &inserted);
	if (inserted) catalogue->numCourses++;
}

void CatalogueDelete(Catalogue catalogue, int code) {
	bool deleted = false;
	catalogue->tree = deleteNode(catalogue->tree, code, &deleted);
	if (deleted) catalogue->numCourses--;
}

int CatalogueNumCourses(Catalogue catalogue) {
	return catalogue->numCourses;
}

struct course CatalogueFind(Catalogue catalogue, int code) {
	struct node *n = findNode(catalogue->tree, code);
	if (n == NULL) return (struct course){COURSE_UNDEFINED, "", 0};
	return n->course;
}

void CataloguePrint(Catalogue catalogue, FILE *out) {
	bool first = true;
	fprintf(out, "{");
	printTree(catalogue->tree, out, &first);
	fprintf(out, "}");
}

////////////////////////////////////////////////////////////////////////
// Course Queries

struct course CatalogueClosest(Catalogue catalogue, int targetCode) {
	struct node *n = catalogue->tree;
	struct node *best = n;
	while (n != NULL) {
		int diff = abs(n->course.code - targetCode);
		int bestDiff = abs(best->course.code - targetCode);
		if (diff < bestDiff ||
		    (diff == bestDiff && n->course.code < best->course.code)) {
			best = n;
		}
		if (targetCode < n->course.code) {
			n = n->left;
		} else if (targetCode > n->course.code) {
			n = n->right;
		} else {
			break;
		}
	}
	return best->course;
}

int CatalogueLevelOrder(Catalogue catalogue, struct course courses[]) {
	int n = catalogue->numCourses;
	if (n == 0) return 0;

	struct node **queue = malloc(n * sizeof(struct node *));
	if (queue == NULL) {
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}

	int front = 0, back = 0, count = 0;
	queue[back++] = catalogue->tree;
	while (front < back) {
		struct node *curr = queue[front++];
		courses[count++] = curr->course;
		if (curr->left != NULL) queue[back++] = curr->left;
		if (curr->right != NULL) queue[back++] = curr->right;
	}

	free(queue);
	return count;
}

static int getRange(struct node *n, int lowerCode, int upperCode,
                     struct course courses[], int count) {
	if (n == NULL) return count;
	if (n->course.code > lowerCode) {
		count = getRange(n->left, lowerCode, upperCode, courses, count);
	}
	if (n->course.code >= lowerCode && n->course.code <= upperCode) {
		courses[count++] = n->course;
	}
	if (n->course.code < upperCode) {
		count = getRange(n->right, lowerCode, upperCode, courses, count);
	}
	return count;
}

int CatalogueGetRange(Catalogue catalogue, int lowerCode, int upperCode,
                      struct course courses[]) {
	return getRange(catalogue->tree, lowerCode, upperCode, courses, 0);
}

////////////////////////////////////////////////////////////////////////
// Index Operations

struct course CatalogueAtIndex(Catalogue catalogue, int index) {
	if (index < 0 || index >= catalogue->numCourses) {
		return (struct course){COURSE_UNDEFINED, "", 0};
	}

	struct node *n = catalogue->tree;
	while (n != NULL) {
		int leftSize = size(n->left);
		if (index < leftSize) {
			n = n->left;
		} else if (index == leftSize) {
			return n->course;
		} else {
			index -= leftSize + 1;
			n = n->right;
		}
	}
	// unreachable since index is within [0, numCourses - 1]
	return (struct course){COURSE_UNDEFINED, "", 0};
}

static int indexOf(struct node *n, int code, int offset) {
	if (n == NULL) return -1;
	if (code < n->course.code) return indexOf(n->left, code, offset);
	if (code > n->course.code) {
		return indexOf(n->right, code, offset + size(n->left) + 1);
	}
	return offset + size(n->left);
}

int CatalogueIndexOf(Catalogue catalogue, int code) {
	return indexOf(catalogue->tree, code, 0);
}

static int countLower(struct node *n, int code) {
	if (n == NULL) return 0;
	if (code <= n->course.code) return countLower(n->left, code);
	return size(n->left) + 1 + countLower(n->right, code);
}

int CatalogueCountLower(Catalogue catalogue, int code) {
	return countLower(catalogue->tree, code);
}

////////////////////////////////////////////////////////////////////////
// Efficient Construction

Catalogue CatalogueConstruct(struct course courses[], int size) {
	// TODO
	return NULL;
}

////////////////////////////////////////////////////////////////////////

