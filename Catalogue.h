// COMP2521 - Assignment 1
// Interface to the Course Catalogue ADT

// !!! DO NOT MODIFY THIS FILE !!!

#ifndef CATALOGUE_H
#define CATALOGUE_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#include "course.h"

#define COURSE_UNDEFINED INT_MIN

typedef struct catalogue *Catalogue;

////////////////////////////////////////////////////////////////////////
// Basic Operations

Catalogue CatalogueNew(void);
void CatalogueFree(Catalogue catalogue);
void CatalogueInsert(Catalogue catalogue, int code, char *name,
                     int creditPoints);
void CatalogueDelete(Catalogue catalogue, int code);
int CatalogueNumCourses(Catalogue catalogue);
struct course CatalogueFind(Catalogue catalogue, int code);
void CataloguePrint(Catalogue catalogue, FILE *out);

////////////////////////////////////////////////////////////////////////
// Course Queries

struct course CatalogueClosest(Catalogue catalogue, int targetCode);
int CatalogueLevelOrder(Catalogue catalogue, struct course courses[]);
int CatalogueGetRange(Catalogue catalogue, int lowerCode, int upperCode,
                      struct course courses[]);

////////////////////////////////////////////////////////////////////////
// Index Operations

struct course CatalogueAtIndex(Catalogue catalogue, int index);
int CatalogueIndexOf(Catalogue catalogue, int code);
int CatalogueCountLower(Catalogue catalogue, int code);

////////////////////////////////////////////////////////////////////////
// Efficient Construction

Catalogue CatalogueConstruct(struct course courses[], int size);

////////////////////////////////////////////////////////////////////////

#endif

