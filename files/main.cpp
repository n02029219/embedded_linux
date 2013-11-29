//
//  main.cpp
//  opencv
//
//  line-detection of an image


#include "project.h"

int main (int argc, const char * argv[])
{
    
    const char* filename = argc >= 2 ? argv[1] : "road2.png";
    
    // create image matrix
    // loading image in non-grayscale causes an error
    Mat src = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    if (src.empty()) {
        //help();
        cout << "cannot open " << filename << endl;
        return -1;
    }
    
    // beginning time:
    clock_t start = clock();
    
    // create destination matrix
    Mat dst, cdst;
    // use Canny for edge-detection
    // source, destinaton, threshold1, threshold2, aperturesize=3, L2gradient=false
    blur(dst, dst, Size(3,3));
    Canny(src, dst, CANNY_T1, CANNY_T2, CANNY_APERTURE);
    cvtColor(dst, cdst, CV_GRAY2RGB);
    
    // time of canny
    clock_t canny_end = clock();
    
    // ================ PROBABILISTIC HOUGH LINE TRANSFORM ==================
    //      creates line segments
    // dst: edge-detector output (should be grayscale) 
    // lines: vector to store lines found;
    // rho: resolution of parameter r in pixels (using 1)
    // theta: resolution of parameter theta in radians (using 1 degree)
    // threshold: The minimum number of intersections to “detect” a line
    // minLinLength: The minimum number of points that can form a line. Lines with less than this number of points are disregarded.
    // maxLineGap: The maximum gap between two points to be considered in the same line.
    
    vector<Vec4i> lines;
    HoughLinesP(dst, lines, 1, CV_PI/180, HLINES_THRESH, HLINES_MINLINE, HLINES_MINGAP);
    
    // filter out horizontal lines
    remove_horizontal(&lines);
    
    //vector<Vec4i> lane_lines = combine_lines(lines);
    //lane_lines = extend_lines(lane_lines, dst.cols, dst.rows);
    
    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
    cout << "size of lines: " << lines.size() << endl;
    //cout << "size of lane_lines: " << lane_lines.size() << endl;
    cout << "width: " << dst.cols << "  height: " << dst.rows << endl;
    //line(cdst, Point(0,0), Point(100,100), Scalar(255,255,255), 2, CV_AA);
    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-

    // display result:
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        line( cdst, Point(l[X1], l[Y1]), Point(l[X2], l[Y2]), Scalar(0,255,255), 1, CV_AA);
        cout << i << " (" << l[X1] << "," << l[Y1] << ") \t(" << l[X2] << "," << l[Y2] << ")" << endl; 
    }
    cout << "------" << endl;
    // display "lane lines"
    /*
    for( size_t i = 0; i < lane_lines.size(); i++ )
    {
        Vec4i l = lane_lines[i];
        //line( cdst, Point(l[X1], l[Y1]), Point(l[X2], l[Y2]), Scalar(0,255,255), 2, CV_AA);
        cout << i << " (" << l[X1] << "," << l[Y1] << ") \t(" << l[X2] << "," << l[Y2] << ")" << endl; 
    }*/
    
    
    // total end time
    clock_t end = clock();
    cout << endl;
    cout << "Total time: " << double(end-start)/CLOCKS_PER_SEC << endl;
    cout << "Canny time: " << double(canny_end-start)/CLOCKS_PER_SEC << endl;
    cout << "Lines time: " << double(end-canny_end)/CLOCKS_PER_SEC << endl;
    
    // create output image: .png file
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);    // 0-9 for png quality
    imwrite("output_lines.png", cdst, compression_params);
    
    // display output
    namedWindow("original image");
    imshow("original image", src);
    waitKey(1000);  // waits 1 seconds
    namedWindow("lines detected");
    imshow("lines detected", cdst);
    waitKey();
    
    return 0;
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
        cout << "size: " << new_lines.size() << endl;
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
            cout << i << "," << j << "\t" << endl;
            cout << "\tl1(" << l1[X1] << "," << l1[Y1] << ") (" << l1[X2] << "," << l1[Y2] << ")   ";
            cout << slope(l1) << ",   " << x_intercept(l1) << endl;
            cout << "\tl2(" << l2[X1] << "," << l2[Y1] << ") (" << l2[X2] << "," << l2[Y2] << ")   ";
            cout << slope(l2) << ",   " << x_intercept(l2) << endl;
            // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-


            
            if(same_line(l1,l2)) {
                same = true;
                // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                cout << "\tsame line" << endl;
                // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                // swap lines so l1 is higher up than l2
                if (greater_than(l1, l2))
                    swap(&l1, &l2);
                
                // if line found, remove both previous lines and add created line
                if (adjacent(l1,l2)) {
                    cout << "\tlines adjacent" << endl;
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
                    cout << "\tFIX(" << line[X1] << "," << line[Y1] << ") (" << line[X2] << "," << line[Y2] << ")   " << endl;
                    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
                    new_lines.push_back(line);
                    break;  // because lines[i] no longer exists
                }
                else if (seperated(l1,l2)) {
                    cout << "\tlines separated" << endl;
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
                    cout << "\tFIX(" << line[X1] << "," << line[Y1] << ") (" << line[X2] << "," << line[Y2] << ")   " << endl;
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
        // if any points are near an edge, update using y = mx + b
        // x1 always less than x2 (way HoughLines stores them)
        if (l[X1] < NEAR_EDGE) {
            l[X1] = 0;
            l[Y1] = y_intercept(l);
        }
        else if (l[X2] > width-NEAR_EDGE) {
            l[X2] = width;
            l[Y2] = slope(l)*l[X2] + y_intercept(l);
        }
        // x = (y-b)/m
        else if ((l[Y1] > height - NEAR_EDGE) || (l[Y2] > height - NEAR_EDGE)) {
            if (l[Y1] > l[Y2])
                if (l[Y1] > height - NEAR_EDGE) {
                    l[Y1] = height;
                    l[X1] = (l[Y1] - y_intercept(l)) / slope(l);
                }
                else    // need this else to exit the if block
                    ;
            else 
                if (l[Y2] > height - NEAR_EDGE) {
                    l[Y2] = height;
                    l[X2] = (l[Y2] - y_intercept(l)) / slope(l);
                }
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
// determine the y-intercept of a line (using farthest point)
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


