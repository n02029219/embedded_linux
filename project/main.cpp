//
//  main.cpp
//  opencv
//
//  lane-detection of an image

#include "project.h"

int main (int argc, const char * argv[])
{
    clock_t start = clock();
    
    string filestr = argc >= 2 ? "images/" + argv[1] : "images/road3.png";
    const char* filename = c_str(filestr);
    cout << "running opencv with " << filename << endl;
    
    // create image matrix
    // loading image in non-grayscale causes an error
    Mat src = imread(filename, IMREAD_GRAYSCALE);
    if (src.empty()) {
        //help();
        cout << "cannot open " << filename << endl;
        return -1;
    }
    
    // beginning time:
    clock_t canny_start = clock();
    
    // create destination matrix
    Mat dst, cdst;
    // use Canny for edge-detection
    // source, destinaton, threshold1, threshold2, aperturesize=3, L2gradient=false
    blur(dst, dst, Size(3,3));
    Canny(src, dst, CANNY_T1, CANNY_T2, CANNY_APERTURE);
    cvtColor(dst, cdst, COLOR_GRAY2RGB);
    
    // time of canny
    clock_t canny_end = clock();
    double canny_time = (double)(canny_end-canny_start)/CLOCKS_PER_SEC;
    
    // ================ PROBABILISTIC HOUGH LINE TRANSFORM ==================
    //      creates line segments
    // dst: edge-detector output (should be grayscale) 
    // lines: vector to store lines found;
    // rho: resolution of parameter r in pixels (using 1)
    // theta: resolution of parameter theta in radians (using 1 degree)
    // threshold: The minimum number of intersections to “detect” a line
    // minLinLength: The minimum number of points that can form a line. Lines with less than this number of points are disregarded.
    // maxLineGap: The maximum gap between two points to be considered in the same line.
    
    clock_t hough_start = clock();
    
    vector<Vec4i> lines;
    HoughLinesP(dst, lines, 1, CV_PI/180, HLINES_THRESH, HLINES_MINLINE, HLINES_MINGAP);
    
    // filter out horizontal lines
    remove_horizontal(&lines);
    remove_skylines(&lines, dst.rows);
    
    // time of HoughLinesP()
    clock_t hough_end = clock();
    double hough_time = (double)(hough_end-hough_start)/CLOCKS_PER_SEC;
    
    // --------------------------
    
    clock_t lines_start = clock();
    
    vector<Vec4i> lane_lines = combine_lines(lines);
    lane_lines = extend_lines(lane_lines, dst.cols, dst.rows);
    
    // time of lane_lines, extend_lines()
    clock_t lines_end = clock();
    double lines_time = (double)(lines_end-lines_start)/CLOCKS_PER_SEC;
    
    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
    //cout << "size of lines: " << lines.size() << endl;
    //cout << "size of lane_lines: " << lane_lines.size() << endl;
    //cout << "width: " << dst.cols << "  height: " << dst.rows << endl;
    //line(cdst, Point(0,0), Point(100,100), Scalar(255,255,255), 2, CV_AA);
    // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-

    clock_t draw_start = clock();
    
    // display result:
    
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        //line( cdst, Point(l[X1], l[Y1]), Point(l[X2], l[Y2]), Scalar(255,0,0), 1, CV_AA);
     
        // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
        //cout << i << " (" << l[X1] << "," << l[Y1] << ") \t(" << l[X2] << "," << l[Y2] << ")" << endl; 
        // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
    }
     
    //cout << "------" << endl;
    // display "lane lines"
    for( size_t i = 0; i < lane_lines.size(); i++ )
    {
        Vec4i l = lane_lines[i];
        line( cdst, Point(l[X1], l[Y1]), Point(l[X2], l[Y2]), Scalar(0,255,255), 2, LINE_AA);
        
        // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
        //cout << i << " (" << l[X1] << "," << l[Y1] << ") \t(" << l[X2] << "," << l[Y2] << ")" << endl; 
        // -=-=-=-=-=-=-=-=-=-=-=-=- DEBUGGING -=-=-=-=-=-=-=-=-=-=-=-=-
    }
    cout << endl;
    
    // depending on # of lines, draw either one or two lanes
    if (lane_lines.size() > 2)
        cdst = draw_2lanes(cdst, lane_lines);
    else
        cdst = draw_1lane(cdst, lane_lines);
    
    // time for drawing lines
    clock_t draw_end = clock();
    double draw_time = (double)(draw_end-draw_start)/CLOCKS_PER_SEC;
    
    
    // --------------------------
    
    clock_t image_start = clock();
    
    // create output image: .png file
    vector<int> compression_params;
    compression_params.push_back(IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);    // 0-9 for png quality
    imwrite("images/output.png", cdst, compression_params);
    
    // time for generating the image and total time
    clock_t end = clock();
    double image_time = (double)(end-image_start)/CLOCKS_PER_SEC;
    double total_time = (double)(end-start)/CLOCKS_PER_SEC;
    
    // --------------------------
    // display time results:
    cout << "canny time: " << canny_time << " s" << endl;
    cout << "hough time: " << hough_time << " s" << endl;
    cout << "lines time: " << lines_time << " s" << endl;
    cout << "draw time:  " << draw_time << " s" << endl;
    cout << "img time:   " << image_time << " s" << endl;
    cout << "TOTAL TIME: " << total_time << " s" << endl;
    
    cout << "\ndone" << endl;
    
    
    return 0;
}


