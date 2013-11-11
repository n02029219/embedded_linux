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
    Canny(src, dst, 100, 100, 3);
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
    HoughLinesP(dst, lines, 1, CV_PI/180, 25, 50, 20 );
    
    // filter out horizontal lines
    remove_horizontal(&lines);
    
    // display result:
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,255,255), 1, CV_AA);
        cout << "(" << l[0] << ", " << l[1] << ") \t(" << l[2] << ", " << l[3] << ")" << endl;
    }
    
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
    waitKey(1000);  // waits 3 seconds
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

// ------------------------

// determines if the slope of a line is greater than the minimum "horizontal" value
bool horizontal(Vec4i l)
{
    if (slope(l) > HORIZONTAL_TOLERANCE)    // if slope greater than allowed value
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
    vector<Vec4i> new_lines;
    Vec4i l1, l2;
    for (size_t i = 0; i < lines.size(); i++) {
        l1 = lines[i];
        for (size_t j = 0; j < lines.size(); j++) {
            l2 = lines[j];
            if (!greater_than(l1,l2))    // if l1 < l2
                swap(l1,l2);
            if (adjacent(l1,l2)) {
                // fix by making a new line:
                //  x[1,2] = avg(l1_x,l2_x), y[1,2] = avg(l1_y,l2_y)                
                int x1 = mean(l1[0], l2[0]);
                int y1 = mean(l1[1], l2[1]);
                int x2 = mean(l1[2], l2[2]);
                int y2 = mean(l1[3], l2[3]);
                new_lines.push_back(Vec4i (x1,y1,x2,y2));
            }
            else if (seperated(l1,l2)) {
                // fix by making a new line:
                //  min(x,y) : min(l1_lo, l2_lo)
                //  max(x,y) : max(l1_hi, l2_hi)
                int x1 = (int)min(l1[0], l2[0]);
                int y1 = (int)min(l1[1], l2[1]);
                int x2 = (int)max(l1[2], l2[2]);
                int y2 = (int)max(l1[3], l2[3]);
                new_lines.push_back(Vec4i (x1,y1,x2,y2));
            }
            
            
            l1 = lines[i];  // restore, in case value was swapped
        }
    }
    return new_lines;
}

// ------------------------

// determines if line l1 > l2
//  uses hypotenuse to the farthest point to do so
//  (this may not be the best way to go about doing this)
bool greater_than(Vec4i l1, Vec4i l2)
{
    return hypotenuse(l1[2],l1[3]) > hypotenuse(l2[2],l2[3]) ?    true : false;
}

// swaps two lines (to pass to functions in the form: greater,lesser)
void swap(Vec4i *l1, Vec4i *l2)
{
    Vec4i temp = *l2;
    *l2 = *l1;
    *l1 = temp;
}

// ------------------------

// assumes l1 > l2
// determines if two lines are adjacent (similar slope and position)
//  if y-intercepts are "equal" (within tolerance)
//  if slopes are "equal", and smallest l1(x,y) < greatest l2(x,y)
// fix by making a new line: (in calling function)
//  x[1,2] = avg(l1_x,l2_x), y[1,2] = avg(l1_y,l2_y)
bool adjacent(Vec4i l1, Vec4i l2)
{
    // if slopes aren't within tolerance, return false
    double s1 = slope(l1);
    double s2 = slope(l2);
    if (!(s2 > s1-(s1*SLOPE_TOLERANCE) && s2 < s1+(s1*SLOPE_TOLERANCE)))
        return false;
    
    // if y-ints aren't within tolerance, return false
    double yint1 = y_intercept(l1);
    double yint2 = y_intercept(l2);
    if (!(yint2 > yint1-POINT_TOLERANCE && yint2 < yint1+POINT_TOLERANCE))
        return false;
    
    // use hypotenuse to determine difference in location between the two lines
    if (hypotenuse(l1[0],l1[1] > hypotenuse(l2[2],l2[3])))
        return false;   // seperated, not adjacent, if smallest(l1) > greatest(l2)
    
    // else:
    return true;
}

// assumes l1 > l2
// determines if two lines are seperated (similar slope, position lines up)
//  if y-intercepts are "equal" (within tolerance)
//  if slopes are "equal" and smallest l1(x,y) > greatest l2(x,y)
// fix by making a new line: (in calling function)
//  min(x,y) : min(l1_lo, l2_lo)
//  max(x,y) : max(l1_hi, l2_hi)
bool seperated(Vec4i l1, Vec4i l2)
{
    // if slopes aren't within tolerance, return false
    double s1 = slope(l1);
    double s2 = slope(l2);
    if (!(s2 > s1-(s1*SLOPE_TOLERANCE) && s2 < s1+(s1*SLOPE_TOLERANCE)))
        return false;
    
    // if y-ints aren't within tolerance, return false
    double yint1 = y_intercept(l1);
    double yint2 = y_intercept(l2);
    if (!(yint2 > yint1-POINT_TOLERANCE && yint2 < yint1+POINT_TOLERANCE))
        return false;
    
    // use hypotenuse to determine difference in location between the two lines
    if (hypotenuse(l1[0],l1[1] < hypotenuse(l2[2],l2[3])))
        return false;   // adjacent, not seperated, if smallest(l1) < greatest(l2)
    
    // else:
    return true;
}

// ------------------------

// returns the slope of the line passed
double slope(Vec4i l)
{
    return abs((l[3]-l[1]) / (l[2]-l[0]));
}
// determine the y-intercept of a line (using farthest point)
// y0 = y - mx
double y_intercept(Vec4i l)
{
    return l[3] - slope(l)*l[2];
}
// returns the hypotenuse
double hypotenuse(double x, double y)
{
    return sqrt(x*x + y*y);
}
// gets the mean between two points (to be used with Vec4i points)
int mean(int a, int b)
{
    return round((a+b) / 2);
}


