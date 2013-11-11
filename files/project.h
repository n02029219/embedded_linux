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

const double HORIZONTAL_TOLERANCE = 0.3;    // how far from slope=0 is considered horizontal
const double POINT_TOLERANCE = 10;          // how close (in px) for two lines to be "equal"
const double SLOPE_TOLERANCE = 0.1;         // how close (in %) two lines slopes must be to be "equal"

// functions to reduce number of lines in image
// ---
void remove_horizontal(vector<Vec4i> *);    // removes horizontal lines from the vector<Vec4i>
bool horizontal(Vec4i);                     // determines if a line is horizontal (below tolerance)
// ---
vector<Vec4i> combine_lines(vector<Vec4i>); // combines adjacent/seperated lines
// ---
bool greater_than(Vec4i, Vec4i);            // returns true if l1(x,y) > l2(x,y) (hypotenuse)
void swap(Vec4i*, Vec4i*);                  // swaps two lines to pass to adjacent/seperated() correctly
// ---
bool adjacent(Vec4i, Vec4i);                // returns true if l1,l2 are adjacent
bool seperated(Vec4i, Vec4i);               // returns true if l1,l2 are seperated but the "same" line
// ---
double slope(Vec4i);                        // returns slope of the line passed
double y_intercept(Vec4i);                  // determine y-intercept of line passed
double hypotenuse(double,double);           // returns the hypotenuse of two points
int mean(int,int);                          // returns mean between two points

#endif
