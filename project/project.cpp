//
//  project.cpp
//  opencv
//

#include "project.h"

// ===================================================================
// draw_lane() - to draw the actual lanes in between lines
// ===================================================================

// finds a middle line, and draws a polygon from the leftmost to middle and from rightmost to middle
// assumes "american" style roads (drive in right, oncoming on left)
// polgyon point format:
//  topleft, bottomleft, bottomright, topright
Mat draw_2lanes(Mat src, vector<Vec4i> lines)
{
    Mat lanes = src, dst;                     // for blending in semi-transparent lanes
    Vec4i midline = middle_line(lines);
    Vec4i left = leftmost(lines);
    Vec4i right = rightmost(lines);
    
    int npt[1] = { NUM_VERTICES };      // number of points to draw (must be int[])
    
    // create left lane polygon (oncoming traffic lane)
    // use slope to determine which points are which
    Point left_pts[1][4];
    if (slope(left) >= 0) {
        left_pts[0][0] = Point(left[0]+LANE_EDGE, left[1]);
        left_pts[0][1] = Point(left[2]+LANE_EDGE, left[3]);
    }
    else {
        left_pts[0][0] = Point(left[2]+LANE_EDGE, left[3]);
        left_pts[0][1] = Point(left[0]+LANE_EDGE, left[1]);
    }
    if (slope(midline) >= 0) {
        left_pts[0][3] = Point(midline[0]-LANE_EDGE, midline[1]);
        left_pts[0][2] = Point(midline[2]-LANE_EDGE, midline[3]);
    }
    else {
        left_pts[0][3] = Point(midline[2]-LANE_EDGE, midline[3]);
        left_pts[0][2] = Point(midline[0]-LANE_EDGE, midline[1]);
    }
    const Point * left_ppt[1] = { left_pts[0] };
    fillPoly(lanes, left_ppt, npt, NUM_POLYGONS, ONCOMING_COLOR, LINE_TYPE);
    
    // create right lane polygon (oncoming traffic lane)
    // use slope to determine which points are which
    Point right_pts[1][4];
    if (slope(midline) >= 0) {
        right_pts[0][0] = Point(midline[0]+LANE_EDGE, midline[1]);
        right_pts[0][1] = Point(midline[2]+LANE_EDGE, midline[3]);
    }
    else {
        right_pts[0][0] = Point(midline[2]+LANE_EDGE, midline[3]);
        right_pts[0][1] = Point(midline[0]+LANE_EDGE, midline[1]);
    }
    if (slope(right) >= 0) {
        right_pts[0][3] = Point(right[0]-LANE_EDGE, right[1]);
        right_pts[0][2] = Point(right[2]-LANE_EDGE, right[3]);
    }
    else {
        right_pts[0][3] = Point(right[2]-LANE_EDGE, right[3]);
        right_pts[0][2] = Point(right[0]-LANE_EDGE, right[1]);
    }
    const Point * right_ppt[1] = { right_pts[0] };
    fillPoly(lanes, right_ppt, npt, NUM_POLYGONS, THISLANE_COLOR, LINE_TYPE);
    
    // should blend the images to make semi-transparent lanes (doesn't work right)
    addWeighted(lanes, ALPHA, src, 1-ALPHA, 0.0, dst);
    return dst;
}

// draws polgyon from leftmost to rightmost lines
Mat draw_1lane(Mat src, vector<Vec4i> lines)
{
    Mat lanes = src, dst;                     // for blending in semi-transparent lanes
    Vec4i left = leftmost(lines);
    Vec4i right = rightmost(lines);
    
    int npt[1] = { NUM_VERTICES };      // number of points to draw (must be int[])
    
    // create left lane polygon (oncoming traffic lane)
    // use slope to determine which points are which
    Point pts[1][4];
    if (slope(left) >= 0) {
        pts[0][0] = Point(left[0]+LANE_EDGE, left[1]);
        pts[0][1] = Point(left[2]+LANE_EDGE, left[3]);
    }
    else {
        pts[0][0] = Point(left[2]+LANE_EDGE, left[3]);
        pts[0][1] = Point(left[0]+LANE_EDGE, left[1]);
    }
    if (slope(right) >= 0) {
        pts[0][3] = Point(right[0]-LANE_EDGE, right[1]);
        pts[0][2] = Point(right[2]-LANE_EDGE, right[3]);
    }
    else {
        pts[0][3] = Point(right[2]-LANE_EDGE, right[3]);
        pts[0][2] = Point(right[0]-LANE_EDGE, right[1]);
    }
    const Point * ppt[1] = { pts[0] };
    fillPoly(lanes, ppt, npt, NUM_POLYGONS, THISLANE_COLOR, LINE_TYPE);
    
    // should blend the images to make semi-transparent lanes (doesn't work right)
    addWeighted(lanes, ALPHA, src, 1-ALPHA, 0.0, dst);
    return dst;
}

// ------------------------
// finding lines (middle, leftmost, rightmost)

// finds the "middle" line: always using l[0], first x-point
// middle line if x point is both less than another's, and greater than another
Vec4i middle_line(vector<Vec4i> lines)
{
    for (int i = 0; i < lines.size(); i++) {
        bool less = false;
        bool more = false;
        Vec4i l1 = lines[i];
        for (int j = 0; j < lines.size(); j++) {
            if (i == j)
                continue;   // skip same line
            Vec4i l2 = lines[j];
            
            if (l1[0] < l2[0])
                less = true;
            if (l1[2] > l2[2])
                more = true;
            
            if (less && more)
                return l1;
        }
    }
    // else: (shouldn't happen, there MUST be a middle line)
    return lines[0];
}

// finds the leftmost line (x = min)
Vec4i leftmost(vector<Vec4i> lines)
{
    Vec4i left = lines[0];
    // choose leftmost point
    int min_x1 = (lines[0])[X1];
    int min_x2 = (lines[0])[X2];
    
    for (int i = 1; i < lines.size(); i++) {
        Vec4i l = lines[i];
        // X1 may be <= because of extend_lines()
        if (l[X1] <= min_x1 && l[X2] < min_x2) {
            left = lines[i];
            min_x1 = l[X1];
            min_x2 = l[X2];
        }
    }
    
    return left;
}

// finds the rightmost line (x = max)
Vec4i rightmost(vector<Vec4i> lines)
{
    Vec4i right = lines[0];
    // choose rightmost point
    int max_x1 = (lines[0])[X1];
    int max_x2 = (lines[0])[X2];
    
    for (int i = 1; i < lines.size(); i++) {
        Vec4i l = lines[i];
        // X2 may be >= because of extend_lines()
        if (l[X1] > max_x1 && l[X2] >= max_x2) {
            right = lines[i];
            max_x1 = l[X1];
            max_x2 = l[X2];
        }
    }
    
    return right;
}


// ===================================================================
// functions - filtering lines out of image, etc
// ===================================================================


// removes lines that are too close to horizontal
void remove_horizontal(vector<Vec4i> * lines)
{
    for (size_t i = 0; i < lines->size(); i++)
        if (horizontal((*lines)[i]))
            lines->erase(lines->begin() + i--);
}

// removes two lines from a vector
void remove_lines(int i, int j, vector<Vec4i> *lines)
{
    // remove greater index first so it doesn't shift lower index
    if (i > j) {
        lines->erase(lines->begin() + i);
        lines->erase(lines->begin() + j);
    }
    else {
        lines->erase(lines->begin() + j);
        lines->erase(lines->begin() + i);    
    }
}

// determines if the slope of a line is greater than the minimum "horizontal" value
bool horizontal(Vec4i l)
{
    if (abs(slope(l)) > HORIZONTAL_TOLERANCE)    // if slope greater than allowed value
        return false;
    else
        return true;
}

// removes lines above the midway point of image; they are in the sky (can't be road lines)
void remove_skylines(vector<Vec4i> * lines, int height)
{
    for (size_t i = 0; i < lines->size(); i++)
        if (skyline((*lines)[i], height))
            lines->erase(lines->begin() + i--);
}

// determines if a line is "in the sky" (both y-points are above midway point of image
bool skyline(Vec4i l, int height)
{
    return (l[Y1] < height / 2 && l[Y2] < height / 2)   ?   true : false;
}

// ------------------------

// concatenate lines that are close together (same slope, similar x,y position)
// or seperated (same slope, different x,y position)
// must pass adjacent/seperated functions lines like so: l1(x,y) > l2(x,y)
//  returns a new line vector
vector<Vec4i> combine_lines(vector<Vec4i> lines)
{
    vector<Vec4i> new_lines = lines;
    Vec4i l1, l2;
    bool same = false;
    
    for (size_t i = 0; i < new_lines.size(); ) {
        
        // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
        //cout << "size: " << new_lines.size() << endl;
        // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
        
        l1 = new_lines[i];
        
        for (size_t j = 0; j < new_lines.size(); j++) {
            // skip i==j, unless at the last index
            if (i == j) {
                if (j == new_lines.size()-1)
                    continue;
                j++;
            }
            same = false;
            l2 = new_lines[j];
            
            // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
            //cout << i << "," << j << "\t" << endl;
            //cout << "\tl1(" << l1[X1] << "," << l1[Y1] << ") (" << l1[X2] << "," << l1[Y2] << ")   ";
            //cout << slope(l1) << ",   " << x_intercept(l1) << endl;
            //cout << "\tl2(" << l2[X1] << "," << l2[Y1] << ") (" << l2[X2] << "," << l2[Y2] << ")   ";
            //cout << slope(l2) << ",   " << x_intercept(l2) << endl;
            // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
            
            
            
            if(same_line(l1,l2)) {
                same = true;
                
                // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                //cout << "\tsame line" << endl;
                // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                
                // swap lines so l1 is higher up than l2
                if (greater_than(l1, l2))
                    swap(&l1, &l2);
                
                // if line found, remove both previous lines and add created line
                if (adjacent(l1,l2)) {
                    
                    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                    //cout << "\tlines adjacent" << endl;
                    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                    
                    // fix by making a new line:
                    //  x[1,2] = avg(l1_x,l2_x), y[1,2] = avg(l1_y,l2_y)   
                    // first find min and max y-values (in case slopes aren't same sign)
                    // reverse min/max since 0,0 in TOP LEFT
                    double s1 = slope(l1);
                    double s2 = slope(l2);
                    int min_y1 = l1[Y1], max_y1 = l1[Y2], min_y2 = l2[Y1], max_y2 = l2[Y2];
                    if ((s1 >= 0 && s2 < 0) || (s1 < 0 && s2 >= 0)) {
                        min_y1 = slope(l1) >= 0 ?   l1[Y1] : l1[Y2];
                        max_y1 = slope(l1) >= 0 ?   l1[Y2] : l1[Y1];
                        min_y2 = slope(l2) >= 0 ?   l2[Y1] : l2[Y2];
                        max_y2 = slope(l2) >= 0 ?   l2[Y2] : l2[Y1];
                    }
                    int x1 = (int)mean(l1[X1], l2[X1]);
                    int x2 = (int)mean(l1[X2], l2[X2]);
                    int y1 = (int)mean(min_y1, min_y2);
                    int y2 = (int)mean(max_y1, max_y2);        
                    // remove lines if concatenated
                    remove_lines((int)i, (int)j, &new_lines);
                    // add to new and old line vectors
                    Vec4i line (x1,y1,x2,y2);
                    
                    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                    //cout << "\tFIX(" << line[X1] << "," << line[Y1] << ") (" << line[X2] << "," << line[Y2] << ")   " << endl;
                    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                    
                    new_lines.push_back(line);
                    break;  // because lines[i] no longer exists
                }
                else if (seperated(l1,l2)) {
                    
                    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                    //cout << "\tlines separated" << endl;
                    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                    
                    // fix by making a new line:
                    //  min(x,y) : min(l1_lo, l2_lo)
                    //  max(x,y) : max(l1_hi, l2_hi)
                    // first find min and max y-values (in case slopes aren't same sign)
                    // reverse min/max since 0,0 in TOP LEFT
                    double s1 = slope(l1);
                    double s2 = slope(l2);
                    int min_y1 = l1[Y1], max_y1 = l1[Y2], min_y2 = l2[Y1], max_y2 = l2[Y2];
                    if ((s1 >= 0 && s2 < 0) || (s1 < 0 && s2 >= 0)) {
                        min_y1 = slope(l1) >= 0 ?   l1[Y1] : l1[Y2];
                        max_y1 = slope(l1) >= 0 ?   l1[Y2] : l1[Y1];
                        min_y2 = slope(l2) >= 0 ?   l2[Y1] : l2[Y2];
                        max_y2 = slope(l2) >= 0 ?   l2[Y2] : l2[Y1];
                    }
                    int x1 = min(l1[X1], l2[X1]);
                    int x2 = max(l1[X2], l2[X2]);
                    int y1 = max(min_y1, min_y2);
                    int y2 = min(max_y1, max_y2); 
                    // remove lines if concatenated
                    remove_lines((int)i, (int)j, &new_lines);
                    // add to new and old lines vectors
                    Vec4i line (x1,y1,x2,y2);
                    
                    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                    //cout << "\tFIX(" << line[X1] << "," << line[Y1] << ") (" << line[X2] << "," << line[Y2] << ")   " << endl;
                    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                    
                    new_lines.push_back(line);
                    break;  // because lines[i] no longer exists
                }
            }
            // restore, in case value was swapped (for future i,j pairs)
            // only happens if lines were the same but not adjacent or separated (shouldn't happen)
            l1 = new_lines[i];
        }
        if (!same)
            i++;
    }
    return new_lines;
}


// extends lines so that they reach the bottom/edge of the screen
//  if smallest x (will always be X1) is near 0, make it 0
//  if largest x (will always be X2) is near the width, make it width (at correct y)
// must determine largest y first
//  if largest y is near height, make it height (at correct x)
//  don't care about if y is near 0 because that would be the sky in the image
// NEAR_EDGE value assumes lines will probably be close to one edge, so = 150px
vector<Vec4i> extend_lines(vector<Vec4i> lines, int width, int height)
{
    vector<Vec4i> new_lines;
    new_lines.clear();  // just to be sure it's empty
    for (size_t i = 0; i < lines.size(); i++) {
        Vec4i l = lines[i];
        // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
        //cout << i << " (" << l[X1] << "," << l[Y1] << ") \t(" << l[X2] << "," << l[Y2] << ")" << endl; 
        // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
        int y_int = y_intercept(l);
        double s = slope(l);
        // if any points are near an edge, update using y = mx + b
        // y near bottom: x = (y-b)/m
        if ((l[Y1] > height - NEAR_EDGE) || (l[Y2] > height - NEAR_EDGE)) {
            if (l[Y1] > l[Y2])
                if (l[Y1] > height - NEAR_EDGE) {
                    l[Y1] = height;
                    l[X1] = (l[Y1] - y_int) / s;
                }
                else    // need this else to exit the if block
                    ;
            else if (l[Y2] > height - NEAR_EDGE) {
                    l[Y2] = height;
                    l[X2] = (l[Y2] - y_int) / s;
            }
        }
        // x1 always less than x2 (way HoughLines stores them)
        else if (l[X1] < NEAR_EDGE || s < 0) {
            l[X1] = 0;
            l[Y1] = y_int;
        }
        else if (l[X2] > width-NEAR_EDGE || s >= 0) {
            l[X2] = width;
            l[Y2] = s*l[X2] + y_int;
        }
                
        new_lines.push_back(Vec4i(l[X1],l[Y1],l[X2],l[Y2]));
    }
    
    return new_lines;
}

// ------------------------

// assumes l1,l2 are "same" lines
// determines if l1 is higher up than l2, using y-coordinate of furthest point
// uses highest y-coordinate for each line
bool greater_than(Vec4i l1, Vec4i l2)
{
    // use min, not max because 0,0 is TOP LEFT
    return min(l1[Y1],l1[Y2]) > min(l2[Y1],l2[Y2]) ? true : false;
}

// swaps two lines (to pass to functions in the form: greater,lesser)
void swap(Vec4i *l1, Vec4i *l2)
{
    Vec4i temp = *l2;
    *l2 = *l1;
    *l1 = temp;
}

// ------------------------

// if lines are the "same"
// same slope, same x-intercept (within tolerance)
bool same_line(Vec4i l1, Vec4i l2)
{
    // if x-ints aren't within tolerance, return false
    double xint1 = abs(x_intercept(l1));
    double xint2 = abs(x_intercept(l2));
    if (!(xint2 > xint1-POINT_TOLERANCE && xint2 < xint1+POINT_TOLERANCE))
        return false;
    
    // if slopes aren't within tolerance, return false
    double s1 = slope(l1);
    double s2 = slope(l2);
    if (s1 >= 0 && s2 >= 0) {       // both positive
        if (!(s2 > s1-(s1*SLOPE_TOLERANCE) && s2 < s1+(s1*SLOPE_TOLERANCE)))
            return false;
    }
    else if (s1 < 0 && s2 < 0) {    // both negative
        if (!(s2 > s1+(s1*SLOPE_TOLERANCE) && s2 < s1-(s1*SLOPE_TOLERANCE)))
            return false;
    }
    else {  // slopes only equal if absolute values are close (because of different signs)
        s1 = abs(s1);
        s2 = abs(s2);
        if (!(s2 > s1-(s1*SLOPE_TOLERANCE) && s2 < s1+(s1*SLOPE_TOLERANCE)))
            return false;
    }
    
    return true;
}

// assumes l1,l2 are "same" lines and l1 > l2
// determines if two lines are adjacent (similar slope and position)
//  if y-intercepts are "equal" (within tolerance)
//  if slopes are "equal", and smallest l1(x,y) < greatest l2(x,y)
// fix by making a new line: (in calling function)
//  x[1,2] = avg(l1_x,l2_x), y[1,2] = avg(l1_y,l2_y)
// reverse signs since 0,0 in TOP LEFT
bool adjacent(Vec4i l1, Vec4i l2)
{
    // flip sign since 0,0 is TOP LEFT
    int min1 = max(l1[Y1], l1[Y2]);
    int max2 = min(l2[Y1], l2[Y2]);
    if (min1 < max2)
        return false;   // separated, not adjacent
    // else
    return true;
}

// assumes l1,l2 are "same" lines and l1 > l2
// determines if two lines are seperated (similar slope, position lines up)
//  if y-intercepts are "equal" (within tolerance)
//  if slopes are "equal" and smallest l1(x,y) > greatest l2(x,y)
// reverse signs since 0,0 in TOP LEFT
bool seperated(Vec4i l1, Vec4i l2)
{
    // flip sign since 0,0 is TOP LEFT
    int min1 = max(l1[Y1], l1[Y2]);
    int max2 = min(l2[Y1], l2[Y2]);
    if (min1 > max2)
        return false;   // adjacent, not separated
    // else
    return true;
}

// ------------------------

// returns the slope of the line passed
double slope(Vec4i l)
{
    return (double)(l[Y2]-l[Y1]) / (double)(l[X2]-l[X1]);
}

// determine the y-intercept of a line (using farthest point)
// y0 = y - mx
double y_intercept(Vec4i l)
{
    return l[Y2] - slope(l)*l[X2];
}

// determine the x-intercept of a line (using farthest point)
// x0 = x - y/m
double x_intercept(Vec4i l)
{
    return l[X2] - (double)l[Y2]/slope(l);
}
// gets the mean between two points (to be used with Vec4i points)
int mean(int a, int b)
{
    return round((a+b) / 2);
}


