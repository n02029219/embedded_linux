//
//  project.h
//  opencv
//
//  header file for Embedded Linux project

#ifndef opencv_project_h
#define opencv_project_h

//#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;

// constants for opencv functions (canny, houghlinesp)
#define CANNY_T1        100
#define CANNY_T2        100
#define CANNY_APERTURE  3
#define HLINES_THRESH   25
#define HLINES_MINLINE  100
#define HLINES_MINGAP   20

// constants for referncing points (x1,y1)(x2,y2) = l1[0,1,2,3]
#define X1  0
#define Y1  1
#define X2  2
#define Y2  3

// tolerances for determining if two lines are "the same"
const double HORIZONTAL_TOLERANCE = 0.33;    // how far from slope=0 is considered horizontal
const double POINT_TOLERANCE = 100;          // how close (in px) for two lines to be "equal" (x-intercept)
const double SLOPE_TOLERANCE = 0.70;         // how close (in %) two lines slopes must be to be "equal"
// tolerance for how close to edge to extend to a side of image (in px):
const int NEAR_EDGE = 100;

// functions to reduce number of lines in image
// ---
void remove_horizontal(vector<Vec4i> *);    // removes horizontal lines from the vector<Vec4i>
void remove_lines(int,int,vector<Vec4i>*);  // removes two lines from a vector of lines
bool horizontal(Vec4i);                     // determines if a line is horizontal (below tolerance)
// ---
vector<Vec4i> combine_lines(vector<Vec4i>); // combines adjacent/seperated lines
vector<Vec4i> extend_lines(vector<Vec4i>,int,int);  // extends lines to reach end (bottom, edges) of screen
// ---
bool greater_than(Vec4i, Vec4i);            // returns true if first line is higher up than second
void swap(Vec4i*, Vec4i*);                  // swaps two lines to pass to adjacent/seperated() correctly
// ---
bool same_line(Vec4i, Vec4i);               // returns true if l1,l2 are the "same" line
bool adjacent(Vec4i, Vec4i);                // returns true if l1,l2 are adjacent
bool seperated(Vec4i, Vec4i);               // returns true if l1,l2 are seperated but the "same" line
// ---
double slope(Vec4i);                        // returns slope of the line passed
double y_intercept(Vec4i);                  // determine y-intercept of line passed
double x_intercept(Vec4i);                  // determine x-intercept of line passed
int mean(int,int);                          // returns mean between two points

#endif
