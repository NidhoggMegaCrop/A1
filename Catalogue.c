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

Catalogue CatalogueNew(void) {
	// TODO
	return NULL;
}

void CatalogueFree(Catalogue catalogue) {
	// TODO
}

void CatalogueInsert(Catalogue catalogue, int code, char *name,
                     int creditPoints) {
	// TODO
}

void CatalogueDelete(Catalogue catalogue, int code) {
	// TODO
}

int CatalogueNumCourses(Catalogue catalogue) {
	// TODO
	return -1;
}

struct course CatalogueFind(Catalogue catalogue, int code) {
	// TODO
	return (struct course){COURSE_UNDEFINED, "", 0};
}

void CataloguePrint(Catalogue catalogue, FILE *out) {
	// TODO
}

////////////////////////////////////////////////////////////////////////
// Course Queries

struct course CatalogueClosest(Catalogue catalogue, int targetCode) {
	// TODO
	return (struct course){COURSE_UNDEFINED, "", 0};
}

int CatalogueLevelOrder(Catalogue catalogue, struct course courses[]) {
	// TODO
	return 0;
}

int CatalogueGetRange(Catalogue catalogue, int lowerCode, int upperCode,
                      struct course courses[]) {
	// TODO
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Index Operations

struct course CatalogueAtIndex(Catalogue catalogue, int index) {
	// TODO
	return (struct course) {COURSE_UNDEFINED, "", 0};
}

int CatalogueIndexOf(Catalogue catalogue, int code) {
	// TODO
	return -1;
}

int CatalogueCountLower(Catalogue catalogue, int code) {
	// TODO
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Efficient Construction

Catalogue CatalogueConstruct(struct course courses[], int size) {
	// TODO
	return NULL;
}

////////////////////////////////////////////////////////////////////////

