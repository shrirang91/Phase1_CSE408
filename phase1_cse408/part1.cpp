#include <iostream>
#include <cmath>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>

#include "colorinstance.h"

using namespace std;
using namespace cv;

string getColorSpace()
{
    int selection = 0;
    
    cout << "Available color spaces:" << endl;
    
    for (int i = 0; i < ColorInstance::NUM_COLOR_SPACES; i++) {
        cout << "    " << (i+1) << ". " << ColorInstance::COLOR_SPACES[i] << endl;
    }
    
    cout << "Enter the number of the desired color space: ";
    cin >> selection;
    
    if (selection <= 0 || selection > ColorInstance::NUM_COLOR_SPACES) {
        cerr << "ERROR: Didn't select a valid color space." << endl;
        exit(-1);
    }
    
    return ColorInstance::COLOR_SPACES[selection - 1];
}

int main()
{
    ColorInstance *cBottom;
    ColorInstance *cZero;
    ColorInstance *cTop;
    
    int bits;
    int instances;
    
    // Obtain the desired color space
    string space = getColorSpace();
    cout << "Chose " << space << " color space" << endl;
    
    // Obtain the three colors
    cout << endl << "Choose the bottom color instance:" << endl;
    cBottom = ColorInstance::makeInstance(space);
    
    cout << endl << "Choose the zero color instance:" << endl;
    cZero = ColorInstance::makeInstance(space);
    
    cout << endl << "Choose the top color instance:" << endl;
    cTop = ColorInstance::makeInstance(space);
    
    // Obtain the bits for the color map
    cout << endl << "Number of bits: ";
    cin >> bits;
    
    instances = int(pow(2, bits));
    cout << endl << "Color map will contain " << instances << " color instances" << endl;
    
    //create an image matrix object with the number of instances as the width, 50px tall, fill with zeros
    int width = instances;
    Mat map = Mat::zeros(50, width, CV_8UC3);
    
    //bins for color instances are created
    ColorInstance::ColorVector firstHalf = cBottom->interpolate(cZero, instances / 2);
    ColorInstance::ColorVector secondHalf = cZero->interpolate(cTop, instances / 2);
    
    //print color map partitions to file
    ofstream myfile;
    myfile.open ("colorMap" + space + ".txt");
    
    for (int i = 0; i < firstHalf.size(); i++) {
        for (int j = 0; j < 50; j++) {
            map.at<Vec3b>(j,i) = Vec3b(
                firstHalf[i]->_c1,
                firstHalf[i]->_c2,
                firstHalf[i]->_c3
            );
        }
        
        myfile << firstHalf[i]->toString() << endl;
    }
    
    for (int i = 0; i < secondHalf.size(); i++) {
        for (int j = 0; j < 50; j++) {
            map.at<Vec3b>(j, (i + firstHalf.size())) = Vec3b(
                secondHalf[i]->_c1,
                secondHalf[i]->_c2,
                secondHalf[i]->_c3
            );
        }
        
        myfile << secondHalf[i]->toString() << endl;
    }
    myfile.close();
    
    // Convert the image based on the color space
    if (space.compare(ColorInstance::COLOR_RGB) == 0) {
        cvtColor(map, map, CV_RGB2BGR);
    }
    else if (space.compare(ColorInstance::COLOR_XYZ) == 0) {
        cvtColor(map, map, CV_XYZ2BGR);
    }
    else if (space.compare(ColorInstance::COLOR_LAB) == 0) {
        cvtColor(map, map, CV_Lab2BGR);
    }
    else if (space.compare(ColorInstance::COLOR_YUV) == 0) {
        cvtColor(map, map, CV_YUV2BGR);
    }
    else if (space.compare(ColorInstance::COLOR_YCBCR) == 0) {
        Mat channels[3];
        Mat swapped;
        vector<Mat> merging;
        
        split(map, channels);
        merging.push_back(channels[0]); // Y
        merging.push_back(channels[2]); // Cr
        merging.push_back(channels[1]); // Cb
        merge(merging, swapped);
        
        cvtColor(map, map, CV_YCrCb2BGR);
    }
    else if (space.compare(ColorInstance::COLOR_YIQ) == 0) {
        for (int y = 0; y < map.rows; y++) {
            for (int x = 0; x < map.cols; x++) {
                Point pos(x, y);
                
                // Obtain the color at the x-y position.
                Vec3b &pixel = map.at<Vec3b>(pos);
                
                float y = pixel.val[0] / 255.0;
                float i = (pixel.val[1] - 128) / 255.0;
                float q = (pixel.val[2] - 128) / 255.0;
                
                // Replace the YIQ color with BGR color
                pixel.val[0] = (y - 1.106*i + 1.703*q) * 255;
                pixel.val[1] = (y - 0.272*i - 0.647*q) * 255;
                pixel.val[2] = (y + 0.956*i + 0.621*q) * 255;
            }
        }
    }
    else if (space.compare(ColorInstance::COLOR_HSL) == 0) {
        Mat channels[3];
        Mat swapped;
        vector<Mat> merging;
        
        split(map, channels);
        merging.push_back(channels[0]); // H
        merging.push_back(channels[2]); // L
        merging.push_back(channels[1]); // S
        merge(merging, swapped);
        
        cvtColor(swapped, map, CV_HLS2BGR);
    }
    
    for(;;)
    {
        
        namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
        imshow( "Display window",  map);                   // Show our image inside it.
        imwrite ("./map_out.png",map);
        
        if(waitKey (0) ==27 ) 
            break;}
    
    
    return 0;
}

