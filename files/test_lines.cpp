//
//  main.cpp
//  opencv
//
//  line-detection of an image


//#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;

const double TOLERANCE = .3;  // how far from slope=0 is considered horizontal

bool isHorizontal(Vec4i);

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
    
    // ================ STANDARD HOUGH LINE TRANSFORM ==================
    //      creates lines that go all the way across the image
    /*
    // dst: edge-detector output (should be grayscale) 
    // lines: vector to store lines found;
    // rho: resolution of parameter r in pixels (using 1)
    // theta: resolution of parameter theta in radians (using 1 degree)
    // threshold: min number of intersections to "detect" a line
    // srn and stn: default parameters to zero, check OpenCV reference for more info
    vector<Vec2f> lines;
    HoughLines(dst, lines, 1, CV_PI/180, 200, 0, 0);
     
    // display result:
    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*a);
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*a);
        line(cdst, pt1, pt2, Scalar(0,255,255), 3, CV_AA);
    }
     */
    
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
    
    // display result:
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        if (!isHorizontal(l)) {
            line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,255,255), 1, CV_AA);
            cout << "(" << l[0] << ", " << l[1] << ") \t(" << l[2] << ", " << l[3] << ")" << endl;
        }
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

// determines if the slope of a line is greater than the minimum "horizontal" value
bool isHorizontal(Vec4i l)
{
    double dx = l[2] - l[0];
    double dy = l[3] - l[1];
    if (abs(dy / dx) > TOLERANCE)    // if slope greater than allowed value
        return false;
    else
        return true;
}

